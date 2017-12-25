#include <cstring>
#include <iostream>
#include <string>
#include <stack>
#include <stdio.h>
#include <utility>
#include <algorithm>
#include <vector>
#include <map>
#include <set>

using namespace std;

vector<string> interins;
vector<string> ariins;
map<string, string> arithin;

/*Definition in func*/
typedef struct Definition {
  int num;
  string name;
  string value;
  bool isconst;
  vector<int> kill;
} Def;

/*parse and get information from instr*/
int parse_ins_num(string code);
string parse_first_param(string code);
string parse_second_param(string code);
string parse_ins_param(string parm);
string parse_move(string code);
string parse_ari(string code, string op);
string parse_load(string code);
string parse_store(string code);
string parse_ins_op(string code);
int parse_bl(string code);
int parse_br(string code);

/*util funcs*/
void init();
string& replace_all(string& str,const string& old_value,const string& new_value);
bool equalvec(vector<int>::iterator itb, vector<int>::iterator ite);
bool iscons(string value);
int str2int(string str);
string int2str(int num);
bool strisint(string code);
bool strhasnochar(string code);
bool exist_block(vector<int> a, vector<int> b);
vector<int> merge_region(vector<int> a, vector<int> b);
bool exist(int t, const vector<int> a);
bool scr_sort(const vector<int> a, const vector<int> b);
