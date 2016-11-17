#include <iostream>
#include <fstream>
#include <unordered_map> 
#include <unordered_set>
#include <string>
#include <sstream>
#include <vector>
#include <queue>
#include <list>
#include <cmath>
#include <ctime>
#include <algorithm>


using namespace std;

#define CONJUNCTIVE 0
//#define PATH "C:\\Users\\Student\\Desktop\\New folder\\data\\"
#define PATH "h:\\data\\"
#define AVDL 7255
#define NUMOFDOCID 128
#define NUMOFBLOCK 2000
#define NUMOFMETADATA (2+NUMOFBLOCK*2)
#define CHUNKSIZE (NUMOFMETADATA*4+NUMOFBLOCK*NUMOFDOCID*2*4)

void open_list(string word, unordered_map<string, int> &termid, vector<int> &lxcon, vector<int*> &vmdata, int* block, vector<int> &curr_pos, ifstream &datafile);
void output_lexicon(vector<string> &termid, vector<vector<int>> &lexicon);
void input_query(vector<string> &words);
void load_url(vector<string> &url_table, vector<int> &url_length);
void load_lexicon(unordered_map<string, int> &termid, vector<int> &lexicon);
void do_query(vector<int> &lxcon, vector<string> &input, vector<string> &url_table, vector<int> &url_len, unordered_map<string, int> &termid, priority_queue<pair<float, string>> &q, priority_queue<pair<float, int*>> &qf, unordered_map<string, vector<pair<float, string>>> &caches);
bool match_id(ifstream* fp, int *mdata, vector<int> &fpos, int *block, int &freq, int count, int targetid);
bool search_in_block(int *block, int &start_pos, int &freq, int &count, int targetid);
int nextGEQ(int *block, int targetid);
float compute_bm25(float freq, int count, int length);

int foo(int a);
int read_lexicon(string termid, string lexicon);
int read_chunk(string file_name);
