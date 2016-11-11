#include "method.h"

#ifndef TEST
#define TEST 0
#if !TEST
#define QUERY 1
#if !QUERY
int main () {
//	ifstream readfile ("beta_data");
//	ofstream writefile ("test.bin", ios::binary); // | ios::app
	string path=PATH;
	ifstream readfile (path+"50_postings");
	ofstream writefile (path+"inverted-index.bin", ios::binary); // | ios::app
	
	string line, word="";
	
	int ID=0, pos_offset=0, last_pos=0, docID=0, last_one=-1, freq=0, count=0, offset=0;
	vector<int> curr_chunk, chunk_doc, chunk_freq, last_docID, size_of_blocks;
	queue<int> block_doc, block_freq;
	
	int num_chunk=0, num_block=0, num_doc=0, chunk_size=0, buffer_size=0;
	
	vector<vector<int>> lexicon;
	vector<string> termid;
	unordered_map<string, int> curr_block;
	
	if (readfile.is_open())
  	{
  		time_t start=time(0);
  		cout<<"file opened "<<ctime(&start)<<endl;
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
				if(!lexicon.empty())
					lexicon.back()[3]=count;
				word=temp[0];
				vector<int> pos={num_chunk, num_block, num_doc, count};		// start pos of each word
				lexicon.push_back(pos);
				termid.push_back(word);
				curr_block[word]=termid.size()-1;
				num_doc++;					//-------------------------111	
				count=1;
				block_doc.push(docID);					
				block_freq.push(freq);	
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
				if(block_doc.empty())
				{
//					chunk_doc.push_back(docID);
					block_doc.push(docID);
//					cout<<docID<<" "<<chunk_doc.size()<<endl;
				}
				else
				{
//					chunk_doc.push_back(docID-last_one);
					block_doc.push(docID-last_one);
				}
//				chunk_freq.push_back(freq);			
				block_freq.push(freq);
				last_one=docID;			// record doc num in current block
			}
			if(block_doc.size()==NUMOFDOCID)	// one more block full
			{	
				while(!block_doc.empty())
				{
					int topdoc=block_doc.front();
					curr_chunk.push_back(topdoc);
					block_doc.pop();
				}
				while(!block_freq.empty())
				{
					int topfreq=block_freq.front();
					curr_chunk.push_back(topfreq);
					block_freq.pop();
				}
				// format [docid][freq]
				last_docID.push_back(docID); // first docID of each block 
				size_of_blocks.push_back(4*NUMOFDOCID*2);	// size of each block

//				int mdsize=((num_block+1)*2+1+1)*4;	// num_block count from 0
				int mdsize=NUMOFMETADATA*4;
				int curr_size=curr_chunk.size()*4+mdsize;
				if(curr_size>CHUNKSIZE)
				{
					
//					cout<<" mdsize "<<mdsize<<" "<<curr_size<<" "<<num_block<<endl;
					/* -----------------add this information------------------------------
					MetaDataSize TotalBlocks LastDocId1 .. LastDocIdn SizeOfBlock1 .. SizeOfBlock2 DocId1 DocId2 .. DocIdn Freq1 .. Freqn
	 
					Taking example from your last answer
					Actual docIDs:      [ 2  4  7  9 ]  [ 13  15  21  23 ]  [ 28  31  36  43 ] 
					
					Then we store like below
					3 9 23 43 4 4 4 2 2 3 9 4 2 6 23 5 3 5 43 ...(frequencies afterwards)
					---------------------------------------------------------------------*/
					
					// add metadata into chunk or write metadata into file
					vector<int> meta_data(mdsize/4);	// one block occupies 2
					meta_data[0]= meta_data.size()-1;		// the first one is the size of MetaData
					meta_data[1]=num_block;
					for(int i=0;i<num_block;i++)
					{
						meta_data[i+2]=last_docID[i];		// may use offset here at expanse of keeping all the first docs in memory
						meta_data[i+2+num_block]=size_of_blocks[i];		
//						cout<<" !!! "<<last_docID[i]<<" "<<size_of_blocks[i]<<endl;				 
					}

					// pad chunk to CHUNKSIZE
												
					//-----------------------
					
//					cout<<CHUNKSIZE-mdsize<<"   sdfaaaaaaa        "<<chunk_doc.size()<<" "<<chunk_freq.size()<<chunk_doc.back()<<" "<<chunk_freq.back()<<endl;
					writefile.write((char*)&meta_data[0], meta_data.size()*4);		// write metadata into file
					writefile.write((char*)&curr_chunk[0], CHUNKSIZE-mdsize); // write chunk into file// fill up to 64kb?
					num_chunk++;
					
					if(num_chunk>5)
						break;	
//					chunk_doc.erase(chunk_doc.begin(), chunk_doc.end()-NUMOFDOCID);					
//					chunk_freq.erase(chunk_freq.begin(), chunk_freq.end()-NUMOFDOCID);					
					curr_chunk.erase(curr_chunk.begin(), curr_chunk.end()-2*NUMOFDOCID);
					last_docID.clear();		// better using queues
					size_of_blocks.clear();
					num_block=0;
						
					last_docID.push_back(docID); // last docID of each block
					size_of_blocks.push_back(4*curr_chunk.size());	// size of each block
					
					// update lexicon
					for(auto c:curr_block)
					{
						lexicon[c.second][0]=num_chunk;
						lexicon[c.second][1]=num_block;
					}
					if(num_chunk%1==0)
						cout<<num_chunk<<" chunks "<<difftime(time(0), start)<<"s "<<endl;
				}
				curr_block.clear();
				num_doc=0;
				num_block++;
			}
		}
		readfile.close();
		cout<<"generating "<<num_chunk<<" chunks "<<difftime(time(0), start)<<"s\n "<<endl;
	}	
  	else
  		cout<<"file not opened"<<endl;	
	writefile.close();
	
	output_lexicon(termid, lexicon);
//	ifstream w ("inverted-index.bin", ios::binary|ios::ate); // | ios::app
//	streampos size=w.tellg();
//	cout<<"size="<<size<<endl;
//	cout<<endl;
//	int *mem=new int[size/4];
//	w.seekg(0, ios::beg);
//	w.read((char*)mem, size);
//	for(int i=0;i<5163;i++)
//	{
//		cout<<mem[i]<<" ";
//		if((i+1)%5162==0)
//			cout<<"\n\n\n"<<endl;
//	}
//		delete[] mem;
	return 0;
}

#else
int main()
{
	//-----load URL table------------
	vector<string> url_table;
	vector<int> url_len;
	time_t start=time(0);
	load_url(url_table, url_len);
	cout<<"url table costs "<<difftime(time(0), start)<<"s "<<endl;
	
	//-----load lexicon--------------
	unordered_map<string, int> termid;
	priority_queue<pair<float, string>> result;
	priority_queue<pair<float, int*>> qf;
	vector<int> lexicon;
	load_lexicon(termid, lexicon);
	string path=PATH;
	ifstream datafile (path+"inverted-index.bin", ios::binary|ios::ate);
	
	streampos size2=datafile.tellg();
//	cout<<"\ndata size="<<size2<<endl;	
	cout<<"\nlexicon costs "<<difftime(time(0), start)<<"s "<<endl;
	unordered_map<string, vector<pair<float, string>>> caches;
	// input vector
//	vector<string> input;
	vector<string> input={"0"};
//	while(1)
//	{
//		input_query(input);
		clock_t qt=clock();
//		for(auto in:input)
//		{
//			if(caches.find(in)!=caches.end())
//			{
//				for(auto v:caches[in])
//				{
//					result.push(v);
//				}
//			}
//		}
		do_query(datafile, lexicon, input, url_table, url_len, termid, result, qf, caches);
		int i=0;
		while(!result.empty() && i<10)
		{
			cout<<++i<<". "<<result.top().second<<" "<<result.top().first<<" ";
			for(int i=0;i<input.size();i++)
				cout<<qf.top().second[i]<<" ";
			cout<<endl;
			qf.pop();
			result.pop();
		}
		input.clear();
		result = priority_queue <pair<float, string>>(); 
		cout<<"This query using "<<float(clock()-qt)/1000<<"s "<<endl;
//	}

	datafile.close();
	return 0;
}
#endif
#else
int main(){
	
	clock_t qt=clock();
	
	for(long i=0;i<2000000000;i++)
		;
	float a=float(clock()-qt)/1000;
	cout<<"20000000 using "<<float(clock()-qt)/1000<<"s "<<a<<endl;
	string path=PATH;
	ifstream f(path+"50_postings");
	if(f.is_open()){
		cout<<"opened!"<<endl;
	}
}
#endif
#endif




