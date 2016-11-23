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
	ifstream file_t(path+"lexicon");
	if(readfile.is_open() && file_t.is_open())
	{
		cout<<"\nlexicon is loading..."<<endl;
		string word, line;
		int num=0;
		while(getline(readfile, word))
		{
			getline(file_t, line);
			stringstream ss(line);
			string item;
			int i=0;
			while(getline(ss, item, '\ '))
			{
				lexicon[num*4+(i++)]=stoi(item);
			}
			termid[word]=num++;
		}
	}
	else
	{
		cout<<"fail to open file"<<endl;
	}
	readfile.close();
	file_t.close();
	cout<<"lexicon size is "<<lexicon.size()<<endl;
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
	return bm25;
}

void open_list(string word, unordered_map<string, int> &termid, vector<int> &lxcon, vector<int*> &vmdata, int* block, vector<int> &curr_pos, ifstream &datafile)
{	
	int offset=termid[word];
	int nth_chunk=lxcon[0+offset*4], nth_block=lxcon[1+offset*4], nth_doc=lxcon[2+offset*4], count=lxcon[3+offset*4];
	for(int i=0;i<4;i++)
		curr_pos.push_back(lxcon[i+offset*4]);
	long long start_pos=(long long)nth_chunk*CHUNKSIZE;			// BE CAREFUL OF OVERFLOW!!!
	datafile.seekg (start_pos, ios::beg);
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

void do_query(vector<int> &lxcon, vector<string> &input, vector<string> &url_table, vector<int> &url_len, unordered_map<string, int> &termid, priority_queue<pair<float, string>> &q, priority_queue<pair<float, int*>> &qf, unordered_map<string, vector<pair<float, string>>> &caches)
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
	vector<vector<int>> block(input.size(), vector<int>(256, 0));
	vector<ifstream*> file_pointer(input.size());
	string path=PATH;
	for(int i=0;i<input.size();i++)
	{
		file_pointer[i]=new ifstream;
		file_pointer[i]->open(path+"inverted-index.bin", ios::binary|ios::ate);
		open_list(input[i], termid, lxcon, vmdata, &block[i][0], curr_pos[i], *file_pointer[i]);
	}
#if CONJUNCTIVE
	// find first query word in lexicon
	int nth_chunk=curr_pos[min_pos][0], nth_doc=curr_pos[min_pos][2], count=curr_pos[min_pos][3];
	PForDelta p1;
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
			for(int i=0;i<input.size();i++)
			{
				if(i==min_pos)
				{
					continue;
				}
				if(!match_id(file_pointer[i], vmdata[i], curr_pos[i], &block[i][0], freq, curr_pos[i][3], r))
				{
					is_match=false;
					break;
				}				
				freqa[i]=freq;
				score+=compute_bm25(freq, curr_pos[i][3], url_len[r]);
			}
			if(is_match)
			{
				qf.push(make_pair(score, freqa));
				q.push(make_pair(score, url_table[r]));
			}
		}
		ini_pos=0;
		if(remain_num<=0)		// check if cross block
			break;
		if(curr_pos[min_pos][1]>=NUMOFBLOCK){		// new chunk
			int new_msize=0;
			file_pointer[min_pos]->read((char*)&new_msize, 4);
			if(new_msize!=4001){
					cerr<<new_msize<<" error "<<file_pointer[min_pos]->tellg()<<" "<<curr_pos[min_pos][0]<<" "<<curr_pos[min_pos][1]<<endl;
					exit(0);
			}
			file_pointer[min_pos]->read((char*)vmdata[min_pos], new_msize*4);
			int num_blocks=vmdata[min_pos][0];
			file_pointer[min_pos]->read((char*)&block[min_pos][0], vmdata[min_pos][1+num_blocks]);	
			curr_pos[min_pos][0]++;
			curr_pos[min_pos][1]=1;
			curr_pos[min_pos][2]=0;
		} 
		else {			// new block
			int num_blocks=vmdata[min_pos][0];
			file_pointer[min_pos]->read((char*)&block[min_pos][0],vmdata[min_pos][1+num_blocks+(curr_pos[min_pos][1]++)]);
			curr_pos[min_pos][2]=0;
		}
	}

#else
	cout<<"\n DISJUNCTIVE QUERY \n"<<endl;
	unordered_set<int> searched;
	vector<vector<int>> backup(curr_pos);
	for(int i=0;i<input.size();i++){
		min_pos=i;
		
		int nth_chunk=curr_pos[min_pos][0], nth_doc=curr_pos[min_pos][2], count=curr_pos[min_pos][3];
		
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
				
				if(searched.find(i)!=searched.end())
					continue;
				
				int freq=block[min_pos][(ini_pos++)+NUMOFDOCID];
				freqa[min_pos]=freq;
				float score=compute_bm25(freq, count, url_len[r]);		
				for(int i=0;i<input.size();i++)
				{
					if(i==min_pos || searched.find(i)!=searched.end())
					{
						continue;
					}
					if(match_id(file_pointer[i], vmdata[i], curr_pos[i], &block[i][0], freq, curr_pos[i][3], r))
					{							
						searched.insert(r);
						freqa[i]=freq;
						score+=compute_bm25(freq, curr_pos[i][3], url_len[r]);
					}
					else
						freqa[i]=0;
				}
				q.push(make_pair(score, url_table[r]));
				qf.push(make_pair(score, freqa));
			}
			
			ini_pos=0;
			if(remain_num<=0)		// check if cross block
				continue;
			if(curr_pos[min_pos][1]>=NUMOFBLOCK){		// new chunk
				int msize=0;		// renew metadata
				file_pointer[min_pos]->read((char*)&msize, 4);
				if(msize!=4001){
					cerr<<" error "<<msize<<" "<<file_pointer[min_pos]->tellg()<<" "<<curr_pos[min_pos][0]<<" "<<curr_pos[min_pos][1]<<endl;
					exit(0);
				}
				file_pointer[min_pos]->read((char*)vmdata[min_pos], msize*4);
				int num_blocks=vmdata[min_pos][0];
				file_pointer[min_pos]->read((char*)&block[min_pos][0], vmdata[min_pos][1+num_blocks]);			
				curr_pos[min_pos][1]=1;
				curr_pos[min_pos][0]++;
			} 
			else {			
				int num_blocks=vmdata[min_pos][0];
				file_pointer[min_pos]->read((char*)&block[min_pos][0], vmdata[min_pos][1+num_blocks+(curr_pos[min_pos][1]++)]);
				curr_pos[min_pos][2]=0;
			}
		}
		int k=0;
		for(auto b:backup){
			open_list(input[k], termid, lxcon, vmdata, &block[k][0], curr_pos[k], *file_pointer[k]);	
			curr_pos[k++]=b;
		}
	}
#endif
}

bool match_id(ifstream* fp, int *mdata, vector<int> &fpos, int *block, int &freq, int count, int targetid)
{ 
	if((long long)fpos[0]*CHUNKSIZE>=8441792720){
		cout<<(long long)fpos[0]*CHUNKSIZE<<" "<<(long long)fpos[0]<<" "<<CHUNKSIZE<<" "<<endl;
		return false;
	}
	int nth_doc=fpos[2];	// update count?
	int last_docid=mdata[fpos[1]-1];
	if(nth_doc+count<=NUMOFDOCID || targetid<=last_docid){			// check if cross block
		return search_in_block(block, fpos[2], freq, count, targetid);
	}
	else{
		int curr_block=fpos[1];
		int size_sum=0;
		int block_num=mdata[0];
		while(curr_block<=mdata[0]){
			int block_size=mdata[curr_block+block_num];
			// cout<<block_size<<endl;
			if(targetid<=last_docid){
				fp->seekg(size_sum, ios::cur);	// always keep fp pointing to begin of blocks
				fp->read((char*)block, block_size);
				fpos[1]=curr_block+1;
				fpos[2]=0;
				return search_in_block(block, fpos[2], freq, count, targetid);
			}
			size_sum+=block_size;
			curr_block++;
			last_docid=mdata[curr_block];
		}
		fpos[0]++;		// go to next chunk
		fpos[1]=1;
		fpos[2]=0;
		long long f=fpos[0];
		fp->seekg(f*CHUNKSIZE, ios::beg);	
		int msize=0;
		fp->read((char*)&msize, 4);
		if(msize!=4001){			
			cerr<<msize<<" error "<<fp->tellg()<<endl;
			return false;
			exit(0);
		}
		// return true;
		fp->read((char*)mdata, msize*4);
		fp->read((char*)block, mdata[block_num+1]);
		return match_id(fp, mdata, fpos, block, freq, count, targetid);
	}
}


bool search_in_block(int *block, int &start_pos, int &freq, int &count, int targetid)
{
	for(;start_pos<NUMOFDOCID && count>0;start_pos++, count--){
		if(block[start_pos]==targetid){
			freq=block[start_pos+NUMOFDOCID];
			return true;
		}
	}
	freq=block[start_pos+NUMOFDOCID-1];
	return true;
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