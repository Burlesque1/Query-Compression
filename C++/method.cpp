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
	ofstream lexicon_f("lexicon.bin", ios::binary);
	ofstream lexicon_ft("lexicon");
	for(int i=0;i<termid.size();i++)
	{
		termid_f<<termid[i]<<endl; 
		// cout<<"  "<<termid[i]<<endl;
	}
	for(auto l:lexicon)
	{
		for(auto ll:l)
			lexicon_ft<<ll<<" ";
		lexicon_ft<<endl;
		lexicon_f.write((char*)&l[0],l.size()*4);
	}
	termid_f.close();
	lexicon_f.close();
	lexicon_ft.close();
	return 0;
}

int *load_lexicon(unordered_map<string, int> &termid)
{
	ifstream readfile ("termid");
	// unordered_map<string, int> termid;
	if(readfile.is_open())
	{
		cout<<"lexicon loading..."<<endl;
//		cout<<"termid file opened"<<endl;
		string line;
		int num=0;
		while(getline(readfile, line))
		{
			termid[line]=num++;
			cout<<line<<" "<<termid[line]+1<<endl;
		}			
	}
	else
	{
		cout<<"fail to open file"<<endl;
	}
	readfile.close();
	
	ifstream file("lexicon.bin", ios::binary|ios::ate);
	streampos size=file.tellg();
	
	int *lxcon=new int[size/4];
	file.seekg (0, ios::beg);
	file.read ((char*)lxcon, size);
	file.close();
	cout<<"lexicon loading finish!"<<endl;
	cout<<"lexicon size="<<size<<endl;
	for(int i=0;i<28;i++)
		cout<<lxcon[i]<<" ";
	cout<<endl;
	return lxcon;
}

void do_query(ifstream &datafile, int *lxcon, int offset)
{//-------------------------------------------------------------------------------------------------------------
	int nth_chunk=lxcon[0+offset*4], nth_block=lxcon[1+offset*4], nth_doc=lxcon[2+offset*4], count=lxcon[3+offset*4];
	streampos start_cpos=nth_chunk*CHUNKSIZE;
	cout<<offset<<" a  "<<" "<<nth_chunk<<" "<<nth_block<<" "<<nth_doc<<" "<<count<<endl;
		
	datafile.seekg (start_cpos, ios::beg);
	
	//---------calculate meta data-----------
	int *mdsize=new int;						// mdata size
	datafile.read((char*)mdsize, 4);
	int *mdata=new int[*mdsize];				// mdata
	datafile.read((char*)mdata, *mdsize*4);
	int num_blocks=mdata[0];	
	int size_sum=0, nth_size=mdata[num_blocks+nth_block+1];
	// for(int i=num_blocks+1;i<=num_blocks+nth_block;i++)
		// size_sum+=mdata[i];
	//----------------------------------------
	// for(int i=0;i<*mdsize;i++)
		// cout<<mdata[i]<<" ";
	// cout<<endl;
	// cout<<nth_size<<endl;
	// int *block=new int[nth_size/4];		// what if compressed??
	// datafile.seekg (size_sum, ios::cur);
	// datafile.read((char*)block, nth_size);
	// cout<<nth_size<<" "<<nth_chunk<<" "<<nth_block<<" "<<nth_doc<<" "<<" should be   "<<"  "<<block[nth_doc]<<endl;
	
	// for(int i=0;i<nth_size/4;i++)
		// cout<<block[i]<<" ";
	// cout<<"  "<<endl;
}


void input_quert(vector<string> &words)
{
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
}


int foo(int a)
{
	cout<<"dfdf "<<a<<endl;
}
