#include <iostream>
#include <fstream>
#include <unordered_map> 
#include <string>
#include <sstream>
#include <vector>
#include <queue>
#include <list>
#include <cmath>
#include <string.h>

using namespace std;

#define NUMOFDOCID 4
//#define CHUNKSIZE 65536 
//#define METADATASIZE 128
#define CHUNKSIZE 88
#define METADATASIZE 12
#define BUFFERSIZE NUMOFDOCID*4

void input_query(vector<string> &words);
int *load_lexicon(unordered_map<string, int> &termid);
void do_query(ifstream &datafile, int *lxcon, int offset);
int read_chunk(string file_name);
int read_lexicon(string termid, string lexicon);
int setup_lexicon(vector<int> &lexicon, vector<string> &termID);
int output_lexicon(vector<string> &termid, vector<vector<int>> &lexicon);

int foo(int a);

