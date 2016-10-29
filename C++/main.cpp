#include <iostream>
#include <fstream>
#include <unordered_map> 
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include "method.h"


using namespace std;

#ifndef TEST
#define TEST 1


#if TEST!=1
int main () {
	  
	  // ascii mode lexicon
	  vector<int> lexicon;					// <termID, file_offset>
	  
	  // used for binary mode lexicon
	  unordered_map<string, int> term_ID;	// <term, termID>
	  vector<string> ID_term;  				// <termID, term>
	  
	  int ID=0, pos_offset=0, last_pos=0, docID=0, last_docID=0;
	  string line, word="";
	  ifstream readfile ("G:\\A2\\posting_b");
	  ofstream writefile ("test.bin", ios::binary);
	  if (readfile.is_open())
	  {	
		cout<<"opened"<<endl;
		int i=0;
		while ( getline (readfile,line) )
		{	
			if(i++>5000000)
				break;
			vector<string> temp;	// WORD, DOCID, DOCID, DOCID...
			stringstream ss(line);
			string item;
			while (getline(ss, item, '\ ')) 
			{
				temp.push_back(item);
			}			
			docID = stoi(temp[1]);
			if(temp[0]==word && docID==last_docID)
				;
			else
			{
				if(temp[0]!=word)		// encounter a new word
				{	
					word=temp[0];
					// add into lexicon
					term_ID[word]=ID++;
					ID_term.push_back(word);
					lexicon.push_back(pos_offset);  
					last_pos=writefile.tellp(); 
				} 	
				last_docID=docID;
				// write into file
				writefile.seekp(0, ios::end);
				writefile.write((char*)(&docID), sizeof(docID));
				// accumulate pos
				pos_offset=writefile.tellp() - last_pos; // store difference of pos to compress. In binary mode each int occupies 4 bytes	   	
			}
			if(i>=100000 && i==(i/100000)*100000)
				cout<<i<<" lines finishes "<<endl;		
		}
		writefile.close();
		readfile.close();
	  }
	  else 
			cout << "Unable to open file"; 
	 
		
	//  read_b_file(""); // for checking purpose
	  setup_lexicon(lexicon, ID_term);
	  return 0;
}

#else
int main()
{
	cout<<"this is test mode!"<<endl;
	ifstream readfile ("beta_data",ios::binary);
	ofstream writefile ("test.bin", ios::binary);
	
	int ID=0, pos_offset=0, last_pos=0, docID=0, last_docID=0, freq=0, count_docID=0;
	string line, word="";
	int block_size=65536;
	vector<int> chunk, chunk_freq;
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
			else if(docID==last_docID)
			{
				continue;					
			}	
			last_docID=docID;
			chunk.push_back(docID);
			chunk_freq.push_back(freq);
			count_docID++;
			if(chunk.size()==20)
			{
				// write into file
				// writefile.seekp(0, ios::end);	// make pointer pointing to the end of file
				int last=chunk[0];
				int skip_len=sqrt(count_docID);
				writefile.write((char*)(&last), sizeof(last));
				for(int i=1;i<chunk.size();i++)
				{			
					int ID=chunk[i];	
					int gap=ID;	
					writefile.write((char*)(&gap), sizeof(gap));
					last=ID;
				}
				chunk.clear();
			}				
			
			// accumulate pos
			pos_offset=writefile.tellp() - last_pos; // store difference of pos to compress. In binary mode each int occupies 4 bytes	   		
		}
		readfile.close();
	}	
  	else
  		cout<<"not"<<endl;
	writefile.close();
//	read_b_file("test.bin");
//	foo(3);
	streampos size;
  	int * memblock;
//
  ifstream file ("test.bin", ios::in|ios::binary|ios::ate); // ios::ate	set the initial position at the end of the file.
  															// otherwise the initial position is the beginning of the file.
  if (file.is_open())
  {
    size = file.tellg();		// size of the whole file
    memblock = new int [size];
    file.seekg (0, ios::beg);	// remember that we opened the file with this pointer at the end
    file.read ((char*)memblock, size);
    file.close();

    cout << "the entire file content is in memory\n";
	for(int i=0;i*4<size;i++)
		cout << memblock[i] << " "<<i<<endl;
    delete[] memblock;
    return 1;	// succeed return 1
  }
  else 
  {
	cout << "Unable to open file";
//	return -1;	// fail return -1
  }
	return 0;
}

#endif
#endif



