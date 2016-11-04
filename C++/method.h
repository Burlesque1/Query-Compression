#include <iostream>
#include <fstream>
#include <unordered_map> 
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <cmath>
#include <string.h>
using namespace std;


int read_chunk(string file_name);
int read_lexicon(string termid, string lexicon);
int setup_lexicon(vector<int> &lexicon, vector<string> &termID);
int output_lexicon(vector<string> &termid, vector<vector<int>> &lexicon);

int foo(int a);

