#include "lab3.hpp"

void init() {
  /*initialization the arithin*/
  arithin["add"] = "+";
  arithin["sub"] = "-";
  arithin["mul"] = "*";
  arithin["div"] = "/";
  arithin["mod"] = "%";
  arithin["cmple"] = "<=";
  arithin["cmpeq"] = "==";
  arithin["cmplt"] = "<";
  ariins.push_back("cmple");
  ariins.push_back("cmpeq");
  ariins.push_back("cmplt");
  ariins.push_back("add");
  ariins.push_back("sub");
  ariins.push_back("mul");
  ariins.push_back("div");
  ariins.push_back("mod");
}

bool equalvec(vector<int>::iterator itb, vector<int>::iterator ite) {
  int num = *itb;
  vector<int>::iterator it = itb;
  while (it != ite) {
    if ((*it) != num)
      return false;
    it++;
  }
  return true;
}

bool iscons(string value) {
  for (unsigned i = 0; i != value.size(); i++) {
    if ((value[i] >= 'a' && value[i] <= 'z') ||
        (value[i] >= 'A' && value[i] <= 'Z') || value[i] == '(' ||
        value[i] == ')')
      return false;
  }
  return true;
}

int str2int(string str) {
  int i;
  char *pp = new char[str.size() + 1];
  strcpy(pp, str.c_str());
  sscanf(pp, "%d", &i);
  delete[] pp;
  return i;
}

string int2str(int num) {
  char buffer[20];
  sprintf(buffer, "%d", num);
  string s(buffer);
  return s;
}

bool strisint(string code) {
  for (unsigned i = 0; i < code.size(); i++) {
    if (code[i] < '0' || code[i] > '9')
      return false;
  }
  return true;
}

bool strhasnochar(string code) {
  if (code.find("[") == string::npos)
    return true;
  int index = code.find("[");
  for (unsigned i = index; i != code.size(); i++) {
    if ((code[i] > 'a' && code[i] < 'z') || (code[i] > 'A' && code[i] < 'Z'))
      return false;
  }
  return true;
}

/*vector if two block have same int*/
bool exist_block(vector<int> a, vector<int> b) {
  vector<int>::iterator it_a = a.begin();
  vector<int>::iterator it_b = b.begin();
  while (it_a != a.end() && it_b != b.end()) {
    if (*(it_a) == *(it_b))
      return true;
    else if (*(it_a) > *(it_b))
      it_b++;
    else
      it_a++;
  }
  return false;
}

/*merge two vector*/
vector<int> merge_region(vector<int> a, vector<int> b) {
  for (vector<int>::iterator it = b.begin(); it != b.end(); it++) {
    a.push_back(*it);
  }
  a.resize(unique(a.begin(), a.end()) - a.begin());
  return a;
}

/*test if an int exist in a vector*/
bool exist(int t, const vector<int> a) {
  for (unsigned i = 0; i < a.size(); i++) {
    if (t == a[i])
      return true;
  }
  return false;
}

/*sort the scr by size*/
bool scr_sort(const vector<int> a, const vector<int> b) {
  if (a.size() > b.size())
    return false;
  else
    return true;
}

string& replace_all(string& str,const string& old_value,const string& new_value)
{
    while(true)
    {
        int pos=0;
        if((pos=str.find(old_value,0))!=string::npos)
            str.replace(pos,old_value.length(),new_value);
        else break;
    }
    return str;
}