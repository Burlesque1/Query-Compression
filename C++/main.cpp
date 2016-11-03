#include "method.h"


using namespace std;

#define NUMOFDOCID 4
//#define CHUNKSIZE 65536 
//#define METADATASIZE 128
#define CHUNKSIZE 56 
#define METADATASIZE 12
#define BUFFERSIZE NUMOFDOCID*4

#ifndef TEST
#define TEST 0


#if TEST!=1
int main () {
	cout<<"this is test mode!"<<endl;
	ifstream readfile ("beta_data",ios::binary);
	ofstream writefile ("test.bin", ios::binary); // | ios::app
	
	int ID=0, pos_offset=0, last_pos=0, docID=0, last_one=0, freq=0, count_docID=0;
	int chunk_size=0, num_block=0, buffer_size=0;
	string line, word="";
//	int chunk_size=65536;
	vector<int> chunk_id, chunk_freq, last_docID, size_of_blocks;
	char *chunk=nullptr;
	if (readfile.is_open())
  	{
  		cout<<"opened"<<endl;
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
				word=temp[0];
				count_docID=0;
//				// add into lexicon
//				term_ID[word]=ID++;	
//				ID_term.push_back(word);
//				lexicon.push_back(pos_offset);  
//				last_pos=writefile.tellp(); 
			} 				
			else if(docID==last_one)	// should remove duplicates by linux sort
			{
				continue;					
			}	
			last_one=docID;
//			int last=block.empty() ? 0:block.back(); // it's ok to use back() like that
			chunk_id.push_back(docID);
			chunk_freq.push_back(freq);
			count_docID++;
			if(!chunk_id.empty() && chunk_id.size()%NUMOFDOCID==0)
			{	
				char *buffer=(char*)&chunk_id[0];
				buffer_size=BUFFERSIZE; 
				last_docID.push_back(chunk_id.back()); // last docID of each block
				size_of_blocks.push_back(buffer_size);	// size of each block
				num_block++;
//				writefile.write(buffer, buffer_size);	

				// write chunk into file
				int mdsize=(num_block*2+2)*4;
				int curr_size=chunk_id.size()*4+mdsize;
				if(curr_size>CHUNKSIZE)
				{
					/* -----------------add this information------------------------------
					TotalBlocks LastDocId1 .. LastDocIdn SizeOfBlock1 .. SizeOfBlock2 DocId1 DocId2 .. DocIdn Freq1 .. Freqn
	 
					Taking example from your last answer
					Actual docIDs:      [ 2  4  7  9 ]  [ 13  15  21  23 ]  [ 28  31  36  43 ] 
					
					Then we store like below
					3 9 23 43 4 4 4 2 2 3 9 4 2 6 23 5 3 5 43 ...(frequencies afterwards)
					---------------------------------------------------------------------*/
					
					// add metadata into chunk or write metadata into file
					vector<int> meta_data((mdsize-8)/4);
					meta_data[0]= meta_data.size()-1;
					meta_data[1]=num_block-1;
					for(int i=0;i<num_block-1;i++)
					{
						meta_data[i+2]=last_docID[i];
						meta_data[i+1+num_block]=size_of_blocks[i];						 
					}
//					for(auto c:meta_data)
//						cout<<c<<" df "<<mdsize<<" "<<num_block<<endl;
//						
//					for(auto c:last_docID)
//						cout<<c<<" ";	cout<<endl;
//					for(auto c:size_of_blocks)
//						cout<<c<<" ";	cout<<endl;
					// write metadata into file
					writefile.write((char*)&meta_data[0],meta_data.size()*4);
					// write chunk into file
					writefile.write((char*)&chunk_id[0], (chunk_id.size()-4)*4); // fill up to 64kb?
					

										
//					writefile.write((char*)&meta_data[0], mdsize);					
					chunk_id.erase(chunk_id.begin(), chunk_id.end()-NUMOFDOCID);
					last_docID.clear();
					size_of_blocks.clear();
					num_block=0;
					
					last_docID.push_back(chunk_id.back()); // last docID of each block
					size_of_blocks.push_back(buffer_size);	// size of each block
					num_block++;
					
				}  
			}				
			// accumulate pos
			pos_offset=writefile.tellp() - last_pos; // store difference of pos to compress. In binary mode each int occupies 4 bytes	 
//			
		}
		readfile.close();
	}	
  	else
  		cout<<"file not opened"<<endl;
	writefile.close();
	
	read_b_file("test.bin");
	
	return 0;
}

#else
int main()
{
	read_b_file("test.bin");
	
	return 0;	
}

#endif
#endif



