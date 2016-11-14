#include "method.h"

void output_lexicon(vector<string> &termid, vector<vector<int>> &lexicon){
	time_t start=time(0);
	cout<<"generating lexicon... "<<endl;
	string path=PATH;
	ofstream termid_f (path+"termid");
	ofstream lexicon_ft(path+"lexicon");
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
	}
	lexicon_ft.close();
	termid_f.close();
	cout<<"lexicon costs "<<difftime(time(0), start)<<"s "<<endl;
}

void load_lexicon(unordered_map<string, int> &termid, vector<int> &lexicon)
{
	string path=PATH;
	ifstream readfile (path+"termid");
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
//			 cout<<line<<" "<<termid[line]+1<<endl;
		}			
	}
	else
	{
		cout<<"fail to open file"<<endl;
	}
	readfile.close();
	
	ifstream file_t(path+"lexicon");
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
	string path=PATH;
	ifstream readfile (path+"URL-TABLE");
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

void open_list(string word, unordered_map<string, int> &termid, vector<int> &lxcon, vector<int*> &vmdata, int* block, vector<int> &curr_pos, ifstream &datafile)
{	
	int offset=termid[word];
	int nth_chunk=lxcon[0+offset*4], nth_block=lxcon[1+offset*4], nth_doc=lxcon[2+offset*4], count=lxcon[3+offset*4];
	for(int i=0;i<4;i++)
		curr_pos.push_back(lxcon[i+offset*4]);
	
	streampos start_cpos=nth_chunk*CHUNKSIZE;
	datafile.seekg (start_cpos, ios::beg);
	
	//---------calculate meta data-----------
	int *mdsize=new int;						// mdata size
	datafile.read((char*)mdsize, 4);
	int *mdata=new int[*mdsize];				// mdata
	datafile.read((char*)mdata, *mdsize*4);
	vmdata.push_back(mdata);
	
	int num_blocks=mdata[0];	
	int size_sum=0;
	for(int i=num_blocks+1;i<=num_blocks+nth_block;i++)
	{
		size_sum+=mdata[i];
	}
	datafile.seekg (size_sum, ios::cur);
	datafile.read((char*)block, mdata[1+num_blocks+(nth_block++)]);
	curr_pos[1]=nth_block;
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
	vector<vector<int>> curr_pos(input.size());
	vector<int*> block(input.size(), new int[256]);
	ifstream *file_pointer=new ifstream[input.size()];
	string path=PATH;
	for(int i=0;i<input.size();i++)
	{
		file_pointer[i].open(path+"inverted-index.bin", ios::binary|ios::ate);
		open_list(input[i], termid, lxcon, vmdata, block[i], curr_pos[i], file_pointer[i]);
	}
	
#if CONJUNCTIVE
	// find first query word in lexicon
	int nth_chunk=curr_pos[min_pos][0], nth_block=curr_pos[min_pos][1], nth_doc=curr_pos[min_pos][2], count=curr_pos[min_pos][3];
	
	// decompress
	//-----------

	cout<<"\n CONJUNCTIVE QUERY \n"<<endl;
	queue<int> rec_id;		
	int ini_pos=nth_doc, remain_num=count;
	while(remain_num>0){
		int prev=0;
		for(int i=ini_pos;i<NUMOFDOCID && remain_num>0;i++, remain_num--){
			rec_id.push(block[min_pos][i]+prev);
			prev=rec_id.back();
		}		
		while(!rec_id.empty()){
			int *freqa=new int[input.size()];
			auto r=rec_id.front();
			rec_id.pop();
			bool is_match=true;
			int freq=block[min_pos][(ini_pos++)+NUMOFDOCID];
			freqa[min_pos]=freq;
			float score=compute_bm25(freq, count, url_len[r]);
			// float score=compute_bm25(freq, count, 7000);
			for(int i=0;i<input.size();i++)
			{
				if(input[i]==min_term)
				{
					continue;
				}
	//			if(!match_id(&file_pointer[i], vmdata[i], block[i], freq, curr_pos[i][3], r))
	//			{
	//				is_match=false;
	//				break;
	//			}				
				freqa[i]=freq;
				score+=compute_bm25(freq, count, url_len[r]);
				// score+=compute_bm25(freq, count, 7000);
			}
			if(is_match)
			{
				qf.push(make_pair(score, freqa));
				q.push(make_pair(score, url_table[r]));
				// q.push(make_pair(score, "dfsdsfsdfsdfds"));
//				cout<<"score "<<score<<" "<<freq<<" "<<remain_num<<" "<<r<<endl;
			} 
		}
		ini_pos=0;
		if(remain_num<=0)		// check if cross block
			break;
		if(nth_block>=NUMOFBLOCK){		// new chunk
			// renew metadata
			int new_msize=0;
			file_pointer[min_pos].read((char*)&new_msize, 4);
			cout<<" ??? "<<new_msize<<endl;
			nth_block=0;
			file_pointer[min_pos].read((char*)vmdata[min_pos], new_msize*4);
			int num_blocks=vmdata[min_pos][0];
			file_pointer[min_pos].read((char*)block[min_pos], vmdata[min_pos][1+num_blocks+(nth_block++)]);			
			curr_pos[min_pos][1]=nth_block;
			curr_pos[min_pos][0]++;
//			for(int i=0;i<new_msize;i++)
//				cout<<vmdata[min_pos][i]<<" ";
//			
//			cout<<nth_block<<" dfs "<<endl;
//			for(int i=0;i<256;i++)
//				cout<<block[min_pos][i]<<" ";
//			cout<<endl;
//			break;char f;
//			cin>>f;
		} 
		else {			// new block
			int num_blocks=vmdata[min_pos][0];
			file_pointer[min_pos].read((char*)block[min_pos], vmdata[min_pos][1+num_blocks+(nth_block++)]);
			curr_pos[min_pos][1]=nth_block;
		
		}
//		cout<<" finish "<<endl;
	}
#else
	cout<<"\n DISCONJUNCTIVE QUERY \n"<<endl;

	for(int i=0;i<input.size();i++){
		min_pos=i;
		// find first query word in lexicon
		int nth_chunk=curr_pos[min_pos][0], nth_block=curr_pos[min_pos][1], nth_doc=curr_pos[min_pos][2], count=curr_pos[min_pos][3];
		
		// decompress
		//-----------
		
		queue<int> rec_id;		
		int ini_pos=nth_doc, remain_num=count;
		while(remain_num>0){
			int prev=0;
			for(int i=ini_pos;i<NUMOFDOCID && remain_num>0;i++, remain_num--){
				rec_id.push(block[min_pos][i]+prev);
				prev=rec_id.back();
			}		
			while(!rec_id.empty()){
				int *freqa=new int[input.size()];
				auto r=rec_id.front();
				rec_id.pop();
				bool is_match=true;
				int freq=block[min_pos][(ini_pos++)+NUMOFDOCID];
				freqa[min_pos]=freq;
				// float score=compute_bm25(freq, count, url_len[r]);
				float score=compute_bm25(freq, count, 7000);
				for(int i=0;i<input.size();i++)
				{
					if(input[i]==min_term)
					{
						continue;
					}
					freqa[i]=freq;
					// score+=compute_bm25(freq, count, url_len[r]);
					score+=compute_bm25(freq, count, 7000);
				}
				if(is_match)
				{
					qf.push(make_pair(score, freqa));
					// q.push(make_pair(score, url_table[r]));
					q.push(make_pair(score, "dfsdsfsdfsdfds"));
				} 
			}
			ini_pos=0;
			if(remain_num<=0)		// check if cross block
				break;
			if(nth_block>=NUMOFBLOCK){		// new chunk
				int new_msize=0;		// renew metadata
				file_pointer[min_pos].read((char*)&new_msize, 4);
				cout<<" ??? "<<new_msize<<endl;
				nth_block=0;
				file_pointer[min_pos].read((char*)vmdata[min_pos], new_msize*4);
				int num_blocks=vmdata[min_pos][0];
				file_pointer[min_pos].read((char*)block[min_pos], vmdata[min_pos][1+num_blocks+(nth_block++)]);			
				curr_pos[min_pos][1]=nth_block;
				curr_pos[min_pos][0]++;
			} 
			else {			
				int num_blocks=vmdata[min_pos][0];
				file_pointer[min_pos].read((char*)block[min_pos], vmdata[min_pos][1+num_blocks+(nth_block++)]);
				curr_pos[min_pos][1]=nth_block;
			
			}
		}
	}
#endif
}

bool search_in_block(int *block, int start_pos, int &freq, int &count, int targetid)
{
	for(int i=start_pos;i<NUMOFDOCID;i++, count--){
		if(block[i]==targetid){
			freq=block[i+NUMOFDOCID];
			return true;
		}
	}
	return false;
}

bool match_id(ifstream* fp, int *mdata, vector<int> &fpos, int *block, int &freq, int count, int targetid)
{
	int nth_block=fpos[1], nth_doc=fpos[2];	// update count?
	int last_docid=mdata[nth_block+1];
	int size_sum=0;
	if(nth_doc+count<=NUMOFDOCID || targetid<=last_docid){			// check if cross block
		// fp->read((char*)block, vmdata[1+nth_block]);
		return search_in_block(block, nth_doc, freq, count, targetid);
	}
	else{
		for(;nth_block<mdata[0];last_docid=mdata[++nth_block]){
			int block_num=mdata[0];
			int block_size=mdata[nth_block+block_num+1];
			if(targetid<=last_docid){
				fp->seekg(size_sum, ios::cur);	// always keep fp pointing to begin of blocks
				fp->read((char*)block, block_size);
				
				// decompress
				;
				fpos[1]=nth_block;
				fpos[2]=0;
				return search_in_block(block, 0, freq, count, targetid);
			}
			size_sum+=block_size;
		}		
		fpos[0]++;		// go to next chunk
		fpos[1]=0;
		fpos[2]=0;
		fp->seekg(size_sum, ios::cur);	
		int msize=0;
		fp->read((char*)&msize, 4);
		fp->read((char*)mdata, msize);
		int block_num=mdata[0];
		fp->read((char*)block, mdata[block_num+0]);
		return match_id(fp, mdata, fpos, block, freq, count, targetid);
	}
}

void input_query(vector<string> &words)
{
	cout<<"\n\nplease enter your query words:\n"<<endl;
	string query;
	getline(cin, query, '\n');
	stringstream ss(query);
	string keyword;
	while(getline(ss, keyword, ' '))
	{
		if(keyword=="")
		{
			cout<<"please re-enter!"<<endl;
			break;
		}
		if(find(words.begin(), words.end(), keyword)==words.end())
			words.push_back((keyword));
	}
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


