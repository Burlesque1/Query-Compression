#include "method.h"


#ifndef TEST
#define TEST 0


#if !TEST
int main () {
//	cout<<"this is test mode!"<<endl;
	ifstream readfile ("beta_data");
	ofstream writefile ("test.bin", ios::binary); // | ios::app
	
	string line, word="";
	
	int ID=0, pos_offset=0, last_pos=0, docID=0, last_one=0, freq=0, count=0, offset=0;
	vector<int> chunk_doc, chunk_freq, first_docID, size_of_blocks;
	
	int num_chunk=0, num_block=0, num_doc=0, chunk_size=0, buffer_size=0;
	
	int pos_chunk=0, pos_block=0, pos_doc=0;
//	queue<string> curr_block;
//	queue<int> doc_count;
//	queue<int> curr_doc_num;
//	unordered_map<string, int> doc_count;
	vector<vector<int>> lexicon;
	vector<string> termid;
	unordered_map<string, int> curr_block;
	
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
					;		
				}					
				if(!lexicon.empty())
					lexicon.back()[3]=count;
				word=temp[0];
				vector<int> pos={num_chunk, num_block, num_doc, count};		// start pos of each word
				lexicon.push_back(pos);
				termid.push_back(word);
				curr_block[word]=termid.size()-1;
//				cout<<word<<" "<<num_chunk<<" "<<num_block
				num_doc++;					//-------------------------111	
				count=1;
				pos_doc=num_doc;
				pos_chunk=num_chunk;
				pos_block=num_block;
				chunk_doc.push_back(docID);		
				chunk_freq.push_back(freq);			
				last_one=docID;			
			} 				
			else if(docID==last_one)	// should remove duplicates by linux sort
			{
				continue;					
			}	
			else
			{
				num_doc++;					//----------------------------111
				count++;
				if(chunk_doc.size()%NUMOFDOCID==0)
				{
					chunk_doc.push_back(docID);
//					cout<<docID<<" "<<chunk_doc.size()<<endl;
				}
				else
					chunk_doc.push_back(docID-last_one);
				chunk_freq.push_back(freq);			
				last_one=docID;			// record doc num in current block
			}
			
			if(!chunk_doc.empty() && chunk_doc.size()%NUMOFDOCID==0)	// one more block full
			{	
//				cout<<" !!! "<<*(chunk_doc.begin()+num_block*4)<<endl;
				first_docID.push_back(*(chunk_doc.begin()+num_block*4)); // first docID of each block
				size_of_blocks.push_back(16);	// size of each block

				int mdsize=((num_block+1)*2+1+1)*4;	// num_block count from 0
				int curr_size=(chunk_doc.size()+chunk_freq.size())*4+mdsize;
				if(curr_size>CHUNKSIZE)
				{
					
				cout<<" mdsize "<<mdsize<<" "<<curr_size<<" "<<num_block<<endl;
					/* -----------------add this information------------------------------
					MetaDataSize TotalBlocks LastDocId1 .. LastDocIdn SizeOfBlock1 .. SizeOfBlock2 DocId1 DocId2 .. DocIdn Freq1 .. Freqn
	 
					Taking example from your last answer
					Actual docIDs:      [ 2  4  7  9 ]  [ 13  15  21  23 ]  [ 28  31  36  43 ] 
					
					Then we store like below
					3 9 23 43 4 4 4 2 2 3 9 4 2 6 23 5 3 5 43 ...(frequencies afterwards)
					---------------------------------------------------------------------*/
					
					// add metadata into chunk or write metadata into file
					vector<int> meta_data(mdsize/4-2);	// one block occupies 2
					meta_data[0]= meta_data.size()-1;		// the first one is the size of MetaData
					meta_data[1]=num_block;
					for(int i=0;i<num_block;i++)
					{
						meta_data[i+2]=first_docID[i];		// may use offset here at expanse of keeping all the first docs in memory
						meta_data[i+2+num_block]=size_of_blocks[i];		
//						cout<<" !!! "<<first_docID[i]<<" "<<size_of_blocks[i]<<endl;				 
					}
					
//					for(auto m:meta_data)
//						cout<<m<<" ";
//						cout<<endl;
//					for(int i=0;i<first_docID.size();i++)
//						cout<<first_docID[i]<<" "<<size_of_blocks[i]<<endl;
//						cout<<endl;

					// pad something to chunk
										
					//-----------------------
					
					writefile.write((char*)&meta_data[0], meta_data.size()*4);		// write metadata into file
					writefile.write((char*)&chunk_doc[0], (chunk_doc.size()-4)*4); 	// write chunk into file// fill up to 64kb?
					writefile.write((char*)&chunk_freq[0], (chunk_freq.size()-4)*4);
					num_chunk++;
					
//					cout<<"\n test "<<endl;
//					for(auto c:chunk_doc){
//						cout<<c<<" ";
//					}
//					cout<<endl;
//					
					chunk_doc.erase(chunk_doc.begin(), chunk_doc.end()-NUMOFDOCID);
					
					chunk_freq.erase(chunk_freq.begin(), chunk_freq.end()-NUMOFDOCID);
					first_docID.clear();		// better using queue
					size_of_blocks.clear();
					num_block=0;
					
					first_docID.push_back(chunk_doc.front()); // last docID of each block
					size_of_blocks.push_back(16);	// size of each block
					
					// update lexicon
					for(auto c:curr_block)
					{
						cout<<" updated "<<c.first<<" "<<c.second<<" "<<num_chunk<<" "<<num_block<<endl;
						cout<<" old "<<lexicon[c.second][0]<<" "<<lexicon[c.second][1]<<endl;
						lexicon[c.second][0]=num_chunk;
						lexicon[c.second][1]=num_block;
					}
					cout<<endl;
				}
//				cout<<num_chunk<<" "<<num_block<<" "<<curr_block.empty()<<endl;
//				while(!curr_block.empty())		// store the end pos
//				{
//					termid.push_back(curr_block.front());	
//					vector<int> pos={num_chunk, num_block, 	curr_doc_num.front(), doc_count.front()};		// start pos of each word
//					lexicon.push_back(pos);
//					curr_block.pop();
//					curr_doc_num.pop();
//					doc_count.pop();
////					termid.push_back(word);	
//				}  
//			for(auto c:curr_block)
//			{
//				cout<<c.first<<" ";
//			}
//			cout<<"\n fdfs "<<num_chunk<<endl;
				curr_block.clear();
				num_doc=0;
				num_block++;
			}
		}
		readfile.close();
	}	
  	else
  		cout<<"file not opened"<<endl;	
	writefile.close();
//	for(int i=0;i<termid.size();i++)
//	{
//		cout<<termid[i]<<" "<<lexicon[i][0]<<" "<<lexicon[i][1]<<" "<<lexicon[i][2]<<endl;
//	}
	output_lexicon(termid, lexicon);
	ifstream w ("test.bin", ios::binary|ios::ate); // | ios::app
	streampos size=w.tellg();
	int *mem=new int[size/4];
	w.seekg(0, ios::beg);
	w.read((char*)mem, size);
	for(int i=0;i<size/4;i++)
	{
		cout<<mem[i]<<" ";
		if(i%22==21)
			cout<<endl;
		}
		delete[] mem;
	return 0;
}

#else
int main()
{
	unordered_map<string, int> termid;
	
	int *lxcon=load_lexicon(termid);
	ifstream datafile ("test.bin", ios::binary|ios::ate);
	streampos size2=datafile.tellg();
	cout<<"\ndata size="<<size2<<endl;
	
//	int offset=termid["Equilife"];
//	do_query(datafile, lxcon, offset);
	for (auto t:termid)
	{
		if(t.first=="equimentq")
			continue;
		cout<<t.first<<" ";
		do_query(datafile, lxcon, t.second);
	}

	delete[] lxcon;
	return 0;
}

#endif
#endif



