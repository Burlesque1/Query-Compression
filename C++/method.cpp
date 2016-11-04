#include "method.h"

int read_chunk(string file_name)	// used for checking final answer
{
	ifstream file (file_name, ios::in|ios::binary|ios::ate);   // ios::ate	set the initial position at the end of the file.
  																// otherwise the initial position is the beginning of the file.
	if (file.is_open())
	{
		cout << "the entire file content is in memory\n";
	  	streampos size;
  		int * memblock;

		// read metadata size
		int *msize=new int;
		size = file.tellg();
		file.seekg (0, ios::beg);	// remember that we opened the file with this pointer at the end	
		file.read ((char*)msize, 4);
		cout<<"meta data size is "<<*msize<<endl;	
		// read meta data
		int *metadata=new int[*msize];	
		file.read ((char*)metadata, *msize*4);	
		// read data
		cout<<"size="<<size<<endl;
		streampos datasize=size-*msize*4-4;
		memblock = new int [datasize];
		file.read ((char*)memblock, datasize);
		file.close();
		
		for(int i=0;i<*msize;i++)
			cout <<metadata[i] << " "<<i<<endl;
		for(int i=0;i*4<datasize;i++)
			cout <<memblock[i] << " "<<i<<endl;
		delete[] memblock;
   
	    return 1;	// succeed return 1
	}
	else 
	{	
		cout << "Unable to open file";
		return -1;	// fail return -1
	}
}

int read_lexicon(string filename1, string filename2)
{
	ifstream readfile ("termid");
	unordered_map<string, int> termid;
	if(readfile.is_open())
	{
		cout<<"termid file opened"<<endl;
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
	
	ifstream file("lexicon.bin", ios::binary);
	int offset=termid["Equilife"]*16;
	int *mem=new int[4];
	file.seekg (offset, ios::beg);
	file.read ((char*)mem, 16);
	int num_chunk=mem[0], num_block=mem[1], num_doc=mem[2], count=mem[3];
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
	for(int i=0;i<4;i++)
		cout<<mem[i]<<" ";
	delete[] mem;
	file.close();
	textfile.close();
	return 0;	
}

int output_lexicon(vector<string> &termid, vector<vector<int>> &lexicon){
	ofstream termid_f ("termid");
	ofstream  lexicon_f("lexicon.bin", ios::binary);
	ofstream  lexicon_ft("lexicon");
	for(int i=0;i<termid.size();i++)
	{
		termid_f<<termid[i]<<endl; 
		cout<<"  "<<termid[i]<<endl;
	}
	for(auto l:lexicon){
		for(auto ll:l)
			lexicon_ft<<ll<<" ";
		lexicon_ft<<endl;
	}
	for(auto l:lexicon)
		lexicon_f.write((char*)&l[0],l.size()*4);
	termid_f.close();
	lexicon_f.close();
	lexicon_ft.close();
	return 0;
}
int foo(int a)
{
	cout<<"dfdf "<<a<<endl;
}
