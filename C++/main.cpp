#include "method.h"


using namespace std;

#define NUMOFDOCID 4
//#define CHUNKSIZE 65536 
//#define METADATASIZE 128
#define CHUNKSIZE 92 
#define METADATASIZE 12
#define BUFFERSIZE NUMOFDOCID*4

#ifndef TEST
#define TEST 1


#if !TEST
int main () {
//	cout<<"this is test mode!"<<endl;
	ifstream readfile ("beta_data");
	ofstream writefile ("test.bin", ios::binary); // | ios::app
	
	string line, word="";
	
	int ID=0, pos_offset=0, last_pos=0, docID=0, last_one=0, freq=0, count=0, offset=0;
	vector<int> chunk_doc, chunk_freq, first_docID, size_of_blocks;
	
	int num_chunk=0, num_block=0, num_doc=0, chunk_size=0, buffer_size=0;
	vector<vector<int>> lexicon;
	vector<string> termid;
	
	if (readfile.is_open())
  	{
  		cout<<"file opened"<<endl;
		while(getline(readfile,line))
		{
//			cout<<line<<endl;
			vector<string> temp;	// WORD, DOCID, DOCID, DOCID...
			stringstream ss(line);
			string item;
			while (getline(ss, item, '\ ')) 
			{
				temp.push_back(item);  //[term, docID, freq]
			}
			docID = stoi(temp[1]);
			freq = stoi(temp[2]);
			if(temp[0]!=word)		// encounter a new word
			{				
				if(word!="")
				{
					vector<int> pos={num_chunk, num_block, num_doc-1, count};	// -1 previous pos
					lexicon.push_back(pos);
					count=0;
				}
				word=temp[0];
				termid.push_back(word);	
				
				last_one=docID;			// record doc num in current block
				num_doc++;
				count++;
				chunk_doc.push_back(docID);		
				chunk_freq.push_back(freq);						
			} 				
			else if(docID==last_one)	// should remove duplicates by linux sort
			{
				continue;					
			}	
			else
			{
				num_doc++;
				count++;
				if(chunk_doc.size()%NUMOFDOCID==0)
				{
					chunk_doc.push_back(docID);
					cout<<docID<<" "<<chunk_doc.size()<<endl;
				}
				else
					chunk_doc.push_back(docID-last_one);
				chunk_freq.push_back(freq);			
				last_one=docID;			// record doc num in current block
			}
			
			if(!chunk_doc.empty() && chunk_doc.size()%NUMOFDOCID==0)	// one more block full
			{	
//				for(auto c:chunk_doc)
//				cout<<c<<" ";
//				cout<<endl;
				first_docID.push_back(*(chunk_doc.begin()+num_block*4)); // first docID of each block
				size_of_blocks.push_back(16);	// size of each block
				num_block++;
				num_doc=0;

				int mdsize=(num_block*2+1+1)*4;
				int curr_size=(chunk_doc.size()+chunk_freq.size())*4+mdsize;
				if(curr_size>CHUNKSIZE)
				{
					/* -----------------add this information------------------------------
					MetaDataSize TotalBlocks LastDocId1 .. LastDocIdn SizeOfBlock1 .. SizeOfBlock2 DocId1 DocId2 .. DocIdn Freq1 .. Freqn
	 
					Taking example from your last answer
					Actual docIDs:      [ 2  4  7  9 ]  [ 13  15  21  23 ]  [ 28  31  36  43 ] 
					
					Then we store like below
					3 9 23 43 4 4 4 2 2 3 9 4 2 6 23 5 3 5 43 ...(frequencies afterwards)
					---------------------------------------------------------------------*/
					
					// add metadata into chunk or write metadata into file
					vector<int> meta_data((mdsize-2*4)/4);	// one block occupies 2
					meta_data[0]= meta_data.size()-1;		// the first one is the size of MetaData
					meta_data[1]=num_block-1;
					for(int i=0;i<num_block-1;i++)
					{
						meta_data[i+2]=first_docID[i];		// may use offset here at expanse of keeping all the first docs in memory
						meta_data[i+2+num_block-1]=size_of_blocks[i];						 
					}


					// pad something to chunk
										
					writefile.write((char*)&meta_data[0],meta_data.size()*4);		// write metadata into file
					writefile.write((char*)&chunk_doc[0], (chunk_doc.size()-4)*4); 	// write chunk into file// fill up to 64kb?
					writefile.write((char*)&chunk_freq[0], (chunk_freq.size()-4)*4);
					num_chunk++;
					
					chunk_doc.erase(chunk_doc.begin(), chunk_doc.end()-NUMOFDOCID);
					chunk_freq.erase(chunk_freq.begin(), chunk_freq.end()-NUMOFDOCID);
					first_docID.clear();
					size_of_blocks.clear();
					num_block=0;
					
					first_docID.push_back(chunk_doc.front()); // last docID of each block
					size_of_blocks.push_back(16);	// size of each block
					num_block++;
				}  
			}	
		}
		readfile.close();
	}	
  	else
  		cout<<"file not opened"<<endl;
  		
  	vector<int> pos={num_chunk, num_block, num_doc-1, count};	// -1 previous pos
	lexicon.push_back(pos);
	
	int mdsize=(num_block*2+2)*4;
  	cout<<num_block<<" "<<mdsize<<endl;
	vector<int> meta_data(mdsize/4);
	meta_data[0]= meta_data.size()-1; // the first one is the size of MetaData
	meta_data[1]= num_block;
	for(int i=0;i<num_block;i++)
	{
		meta_data[i+2]=first_docID[i];
		meta_data[i+2+num_block]=size_of_blocks[i];		
		cout<<first_docID[i]<<" "<<size_of_blocks[i]<<endl;				 
	}

	for(auto m:meta_data)
		cout<<m<<" ";
	// pad something to chunk
										
	writefile.write((char*)&meta_data[0],meta_data.size()*4);		// write metadata into file
	writefile.write((char*)&chunk_doc[0], chunk_doc.size()*4); 	// write chunk into file// fill up to 64kb?
	writefile.write((char*)&chunk_freq[0], chunk_freq.size()*4);  	
	writefile.close();
	
//	cout<<termid.size()<<" "<<lexicon.size()<<endl;
//	output_lexicon(termid, lexicon);
//	read_chunk("test.bin");
	
	return 0;
}

#else
int main()
{
	ifstream readfile ("termid");
	unordered_map<string, int> termid;
	if(readfile.is_open())
	{
		cout<<"lexicon loading..."<<endl;
//		cout<<"termid file opened"<<endl;
		string line;
		int num=0;
		while(getline(readfile, line))
		{
			termid[line]=num++;
//			cout<<line<<endl;
		}			
	}
	else
	{
		cout<<"fail to open file"<<endl;
		return -1;
	}
	readfile.close();
	
	ifstream textfile("lexicon");
	string line;
//	while(getline(textfile, line))
//	cout<<line<<endl;
	
	ifstream file("lexicon.bin", ios::binary|ios::ate);
	streampos size=file.tellg();
	cout<<"lexicon size="<<size<<endl;
	
	int *mem=new int[size/4];
	file.seekg (0, ios::beg);
	file.read ((char*)mem, size);
	textfile.close();
	file.close();
	cout<<"\nlexicon loading finish!"<<endl;
	
	while(0)
	{
		cout<<"\n\nplease enter your query words:\n"<<endl;
		string query;
		getline(cin, query, '\n');
		cout<<query<<endl;
		stringstream ss(query);
		string keyword;
		vector<string> words;
		while(getline(ss, keyword, ' '))
			words.push_back((keyword));
		cout<<"\nThe query words are:\n";
		for(auto w:words)
			cout<<w<<" ";
		cout<<endl;
	}
	
	for(int i=0;i<32;i++)
		cout<<mem[i]<<" ";
	cout<<endl;
	int offset=termid["Equilife"];
	for(int i=0;i<4;i++)
		cout<<mem[i+offset*4]<<" ";
	int num_chunk=mem[0+offset*4], num_block=mem[1+offset*4], num_doc=mem[2+offset*4], count=mem[3+offset*4];
	if(num_doc==-1)
	{
		num_doc=3;
		if(num_block!=0)
			num_block--;
		else
		{
			num_block=1;
			num_chunk--;
		}
	
	}
	delete[] mem;
	ifstream datafile ("test.bin", ios::binary|ios::ate);
	streampos size2=datafile.tellg();
	cout<<"\ndata size="<<size2<<endl;
	
	int *mem2=new int[size2/4];
	datafile.seekg (0, ios::beg);
	datafile.read ((char*)mem2, size2);
	for(int i=0;i<size2/4;i++)
		cout<<mem2[i]<<" ";
	return 0;
}

#endif
#endif



