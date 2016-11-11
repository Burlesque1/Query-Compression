#include "method.h"

void output_lexicon(vector<string> &termid, vector<vector<int>> &lexicon){
	time_t start=time(0);
	cout<<"generating lexicon... "<<endl;
	ofstream termid_f ("F:\\data\\termid");
	ofstream lexicon_ft("F:\\data\\lexicon");
	for(int i=0;i<termid.size();i++)
	{
		termid_f<<termid[i]<<endl;
	}
	for(auto l:lexicon)
	{
		for(auto ll:l)
			lexicon_ft<<ll<<" ";
		lexicon_ft<<endl;
	}
	lexicon_ft.close();
	termid_f.close();
	cout<<"lexicon costs "<<difftime(time(0), start)<<"s "<<endl;
}

void load_lexicon(unordered_map<string, int> &termid, vector<int> &lexicon)
{
	ifstream readfile ("termid");
	// unordered_map<string, int> termid;
	if(readfile.is_open())
	{
		cout<<"\nlexicon is loading..."<<endl;
//		cout<<"termid file opened"<<endl;
		string line;
		int num=0;
		while(getline(readfile, line))
		{
			termid[line]=num++;
			// cout<<line<<" "<<termid[line]+1<<endl;
		}			
	}
	else
	{
		cout<<"fail to open file"<<endl;
	}
	readfile.close();
	
	ifstream file_t("lexicon");
	string line;
	while(getline(file_t, line))
	{
		stringstream ss(line);
		string item;
		while(getline(ss, item, '\ '))
		{
			lexicon.push_back(stoi(item));
		}
	}
}


void load_url(vector<string> &url_table, vector<int> &url_length)
{
	ifstream readfile ("URL-TABLE");
	if(readfile.is_open())
	{
		cout<<"URL-TABLE is loading..."<<endl;
		string line;
		while(getline(readfile, line))
		{
			stringstream ss(line);
			vector<string> tmp;
			string l;
			while(getline(ss, l, '\ '))
			{
				tmp.push_back(l);
			}
			url_table.push_back(tmp[1]);
			url_length.push_back(stoi(tmp[2]));
		}			
	}
	else
	{
		cout<<"fail to open url table"<<endl;
	}
	readfile.close();
	cout<<"URL-TABLE loading finish!"<<endl;
	cout<<"URL-TABLE size is "<<url_table.size()<<endl;
}


float compute_bm25(float freq, int count, int length)
{
	int N=5151893;
	float K=1.2*(0.25+0.75*length/AVDL);
	float bm25=log((N-count+0.5)/(count+0.5))*(1.2+1)*freq/(K+freq);
	// cout<<"bm25 "<<bm25<<endl;
	return bm25;
}

void open_list(string word, unordered_map<string, int> &termid, vector<int> &lxcon, vector<int*> &vmdata, ifstream &datafile)
{
	/* int offset=termid[min_term];
	int nth_chunk=lxcon[0+offset*4], nth_block=lxcon[1+offset*4], nth_doc=lxcon[2+offset*4], count=lxcon[3+offset*4];
	streampos start_cpos=nth_chunk*CHUNKSIZE;
	cout<<offset<<" a  "<<" "<<nth_chunk<<" "<<nth_block<<" "<<nth_doc<<" "<<count<<endl;
	// cout<<min_term<<" "<<termid[min_term]<<" "<<offset<<endl;	
	
	datafile.seekg (start_cpos, ios::beg);
	
	//---------calculate meta data-----------
	int *mdsize=new int;						// mdata size
	datafile.read((char*)mdsize, 4);
	int *mdata=new int[*mdsize];				// mdata
	datafile.read((char*)mdata, *mdsize*4);
	int num_blocks=mdata[0];	
	int size_sum=0;
	int nth=1+num_blocks+nth_block;
	// int *nth_size=&mdata[1+num_blocks+nth_block];
	for(int i=num_blocks+1;i<=num_blocks+nth_block;i++)
		size_sum+=mdata[i];
	datafile.seekg (size_sum, ios::cur);
	//----------------------------------------
	// for(int i=0;i<*mdsize;i++){
		// cout<<mdata[i]<<" "<<i<<endl;
	// } */
	
	int offset=termid[word];
//	cout<<word<<" "<<offset<<endl;
	int nth_chunk=lxcon[0+offset*4], nth_block=lxcon[1+offset*4], nth_doc=lxcon[2+offset*4], count=lxcon[3+offset*4];
	streampos start_cpos=nth_chunk*CHUNKSIZE;
//	cout<<nth_chunk<<" "<<nth_block<<" "<<nth_doc<<endl;
	datafile.seekg (start_cpos, ios::beg);
	
	//---------calculate meta data-----------
	int *mdsize=new int;						// mdata size
	datafile.read((char*)mdsize, 4);
	// cout<<*mdsize<<endl;
	int *mdata=new int[*mdsize];				// mdata
	datafile.read((char*)mdata, *mdsize*4);
//	 for(int i=0;i<*mdsize;i++)
//		 cout<<mdata[i]<<" "<<i<<endl;
	vmdata.push_back(mdata);
	
	int num_blocks=mdata[0];	
	int size_sum=0;
	// int nth=1+num_blocks+nth_block;
	// int *nth_size=&mdata[1+num_blocks+nth_block];
	for(int i=num_blocks+1;i<=num_blocks+nth_block;i++)
	{
//		 cout<<mdata[i]<<" "<<i<<endl;
		size_sum+=mdata[i];
	}
//	cout<<size_sum<<" "<<954*1024<<endl;
	datafile.seekg (size_sum, ios::cur);
//	int *block=new int[2*NUMOFDOCID];
//	datafile.read((char*)block, 2*NUMOFDOCID*4+8);
//	for(int i=0;i<2*NUMOFDOCID+2;i++)
//		cout<<block[i]<<" ";
}

void do_query(ifstream &datafile, vector<int> &lxcon, vector<string> &input, vector<string> &url_table, vector<int> &url_len, unordered_map<string, int> &termid, priority_queue<pair<float, string>> &q, priority_queue<pair<float, int*>> &qf, unordered_map<string, vector<pair<float, string>>> &caches)
{
	int min_count=INT_MAX, min_pos=0;
	string min_term="";
	for(auto i:input)
	{
		if(termid.find(i)==termid.end())
		{
			cout<<" no such word "<<i<<" found!"<<endl;
			return;
		}
		if(lxcon[3+termid[i]*4]<min_count)
		{
			min_term=i;
			min_count=lxcon[3+termid[i]*4];
			min_pos=find(input.begin(),input.end(),i)-input.begin();
		}
	}
	if(min_count==INT_MAX)
	{
		cout<<"no result!"<<endl;
		return;
	}
	
	vector<int*> vmdata;
	ifstream *file_pointer=new ifstream[input.size()];
	streampos a;
	for(int i=0;i<input.size();i++)
	{
		file_pointer[i].open("inverted-index.bin", ios::binary|ios::ate);
		open_list(input[i], termid, lxcon, vmdata, file_pointer[i]);
	}
	int *block=new int[2*NUMOFDOCID];
	file_pointer[min_pos].read((char*)block, 2*NUMOFDOCID*4);
//	for(int i=0;i<2*NUMOFDOCID;i++)
//		cout<<block[i]<<" ";
		
	
	int offset=termid[min_term];
	int nth_chunk=lxcon[0+offset*4], nth_block=lxcon[1+offset*4], nth_doc=lxcon[2+offset*4], count=lxcon[3+offset*4];
	vector<int> fpos={nth_chunk, nth_block, nth_doc, count};
//	cout<<block[nth_doc]<<endl;
	queue<int> rec_id;		// change to queue
	int ini_pos=nth_doc+1;
	for(int curr_num=0;curr_num<NUMOFDOCID;)
	{	
//		cout<<" fdsa "<<endl;
		int prev=0;
		for(int i = ini_pos;i<NUMOFDOCID && curr_num<count;i++, curr_num++)
		{
			rec_id.push(block[i]+prev);
			prev=rec_id.back();
		}
		while(!rec_id.empty())
		{
			int *freqa=new int[input.size()];
			auto r=rec_id.front();
			rec_id.pop();
			bool is_match=true;
			int freq=block[(ini_pos++)+NUMOFDOCID], curr_count=count;
			freqa[min_pos]=freq;
//			float score=compute_bm25(freq, curr_count, url_len[r]);
			float score=compute_bm25(freq, curr_count, 7000);
			for(int i=0;i<input.size();i++)
			{
				if(input[i]==min_term)
				{
					continue;
				}
//				 if(!match_id(&file_pointer[i], vmdata[i], fpos, freq, r))
//				 {
//				 	curr_count=lxcon[termid[input[i]]];
//					is_match=false;
//					break;
//				 }				
				freqa[i]=freq;
//				score+=compute_bm25(freq, curr_count, url_len[r]);
				score+=compute_bm25(freq, curr_count, 7000);
			}
			if(is_match)
			{
				qf.push(make_pair(score, freqa));
//				q.push(make_pair(score, url_table[r]));
				q.push(make_pair(score, "dfsdsfsdfsdfds"));
//				cout<<"score "<<score<<" "<<freq<<" "<<curr_count<<" "<<r<<endl;
			}
			
		}
		if(nth_block<NUMOFBLOCK)
		{
			int nth=1+nth_block+NUMOFBLOCK;
//			cout<<nth<<endl;
//			cout<<vmdata.size()<<endl;
			int nth_size=vmdata[min_pos][nth];
			// block=new int[nth_size/2];				
			file_pointer[min_pos].read((char*)block, nth_size);	// load new block size includes freq
			nth_block++;
//			cout<<"old chunk "<<nth_size<<endl;
//			for(int i=0;i<nth_size/4;i++)
//				cout<<block[i]<<" "<<i<<endl;
//				cout<<" dfsfa afds "<<endl;
		}
		else
		{
			// new chunk
			int *mdsize=new int;
			file_pointer[min_pos].read((char*)mdsize, 4);
			int *mdata=new int[*mdsize];
			file_pointer[min_pos].read((char*)mdata, *mdsize*4);
			for(int i=0;i<*mdsize;i++)
			{
				vmdata[min_pos][i]=mdata[i];
			}
			file_pointer[min_pos].read((char*)block, 2*NUMOFDOCID*4);
			cout<<"new chunk"<<endl;
			nth_block=0;
		}
		ini_pos=0; 
	} 
}

bool match_id(ifstream* fp, int *vmdata, vector<int> &fpos, int &freq, int targetid)
{
	int i=fpos[2]+1;
	for(int size_sum=0;i<=NUMOFBLOCK;i++)
	{
		if(targetid<=vmdata[i])
		{
			fp->seekg(size_sum, ios::cur);
			int *block=new int[NUMOFDOCID*2]; // [doc][freq]
			fp->read((char*)block, NUMOFDOCID*2*4);
			int nextid=nextGEQ(block, targetid);
			if(nextid==targetid)
			{
				freq=block[i+NUMOFDOCID];
				return true;
			}
			break;
		}
		size_sum+=vmdata[i+NUMOFBLOCK];
	}
	if(i<=NUMOFBLOCK)
		return false;
	else
	{
		fp->seekg(CHUNKSIZE, ios::cur);	// go next chunk
		return match_id(fp, vmdata, fpos, freq, targetid);
	}	
}

int nextGEQ(int *block, int targetid)
{
	return 0;
}

bool match_id(ifstream &datafile, int *lxcon, int &freq, int &count, int targetid)
{
	// check if it has been cached already
	
	//------------------------------------
	
	//----------------if not read from file----------------
	int nth_chunk=lxcon[0], nth_block=lxcon[1], nth_doc=lxcon[2];
	count=lxcon[3];
	streampos start_cpos=nth_chunk*CHUNKSIZE;
	datafile.seekg (start_cpos, ios::beg);
	
	//---------calculate meta data-----------
	int *mdsize=new int;						// mdata size
	datafile.read((char*)mdsize, 4);
	int *mdata=new int[*mdsize];				// mdata
	datafile.read((char*)mdata, *mdsize*4);
	//----------------------------------------
	
	//----------find target docID-------------	
	bool cross_block=nth_doc+count>NUMOFDOCID;
	int num_blocks=mdata[0], size_sum=0, skip_num=0;
	
	//	skip blocks
	//	nextGEQ(lp, k) 
	for(;cross_block && skip_num<count/NUMOFDOCID && targetid>mdata[1+nth_block+skip_num];skip_num++);

	// read block data
	int nth_size=mdata[1+num_blocks+nth_block+skip_num];
	char *cmpblock=new char[nth_size];		// what if compressed??	
	for(int i=num_blocks+1;i<=num_blocks+nth_block+skip_num;i++)
		size_sum+=mdata[i];
	datafile.seekg (size_sum, ios::cur);
	datafile.read(cmpblock, nth_size);
	
	// decompress
	
	//-----------
	int *block=new int[NUMOFDOCID*2];
	block=(int*)cmpblock;
	
	if(cross_block)
	{
		// start from 1
		for(int i=0;i<count-skip_num*NUMOFDOCID;i++)	// could use binary search
			if(block[i]==targetid)
			{
				freq=block[i+NUMOFDOCID];		//	getFreq(lp)
				return true;
			}
	}
	else
	{
		// start from nth_doc
		for(int i=nth_doc;i<NUMOFDOCID;i++)
			if(block[i]==targetid)
			{
				freq=block[i+NUMOFDOCID];
				return true;
			}
	}
	return false;
}

void input_query(vector<string> &words)
{
	cout<<"\n\nplease enter your query words:\n"<<endl;
	string query;
	getline(cin, query, '\n');
	// cout<<query<<endl;
	stringstream ss(query);
	string keyword;
	while(getline(ss, keyword, ' '))
	{
		if(keyword=="")
		{
			cout<<"please re-enter!"<<endl;
			break;
		}
		words.push_back((keyword));
	}
	// cout<<"\nThe query words are:\n";
	// for(auto w:words)
		// cout<<w<<" ";
	// cout<<endl;
}

//--------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------

int foo(int a)
{
	cout<<"dfdf "<<a<<endl;
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


int *load_lexicon(unordered_map<string, int> &termid)
{
	ifstream readfile ("termid");
	// unordered_map<string, int> termid;
	if(readfile.is_open())
	{
		cout<<"\nlexicon is loading..."<<endl;
//		cout<<"termid file opened"<<endl;
		string line;
		int num=0;
		while(getline(readfile, line))
		{
			termid[line]=num++;
			// cout<<line<<" "<<termid[line]+1<<endl;
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
	return lxcon;
}


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


