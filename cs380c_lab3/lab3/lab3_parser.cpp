#include "lab3.hpp"
using namespace std;


int parse_ins_num(string code) {
  char *p = new char[code.size() + 1];
  strcpy(p, code.c_str());
  int num;
  sscanf(p, "%*s %d", &num);
  delete[] p;
  return num;
}

string parse_first_param(string code) {
  char *p = new char[code.size() + 1];
  strcpy(p, code.c_str());
  char pparm[20];
  sscanf(p, "%*s %*s %*s %s", pparm);
  string parm = pparm;
  delete[] p;
  return parm;
}

string parse_second_param(string code) {
  char *p = new char[code.size() + 1];
  strcpy(p, code.c_str());
  char pparm[20];
  sscanf(p, "%*s %*s %*s %*s %s", pparm);
  string parm = pparm;
  delete[] p;
  return parm;
}

string parse_ins_param(string parm) {

  if (parm.find("(") != string::npos) {
    parm = parm.substr(parm.find("(") + 1, parm.find(")") - parm.find("(") - 1);
    unsigned num = str2int(parm);
    if (num > interins.size())
      return " ";
    return interins[num - 1];
  } else {
    if (parm == "GP" || parm == "FP") {
      return "--";
    }
  }
  if (parm.find("#") != string::npos) {
    parm = parm.substr(0, parm.find("#"));
    if (parm.find("_base") != string::npos) {
      parm = parm.substr(0, parm.find("_base"));
    }
    return parm;
  }
  return parm;
}

string parse_move(string code) {
  string param1 = parse_ins_param(parse_first_param(code));
  string param2 = parse_ins_param(parse_second_param(code));
  return param2 + "=" + param1;
}

string parse_ari(string code, string op) {
  op = arithin[op];
  string param1 = parse_ins_param(parse_first_param(code));
  string param2 = parse_ins_param(parse_second_param(code));
  if (param1.find("--") != string::npos) {
    param1.replace(param1.find("--"), 2, "0");
  }
  if (param2.find("--") != string::npos) {
    param2.replace(param2.find("--"), 2, "0");
  }
  return "(" + param1 + op + param2 + ")";
}

string parse_load(string code) {
  string param1 = parse_ins_param(parse_first_param(code));
  int p_beg = 0, p_end;
  while (param1[p_beg] == '(')
    p_beg++;
  p_end = p_beg;
  while (param1[p_end] != '+')
    p_end++;
  string name = param1.substr(p_beg, p_end - p_beg);
  if (param1.find("--") != string::npos) {
    return name;
  }
  param1.replace(p_beg, p_end - p_beg, "0");
  return name + "[" + param1 + "/8" + "]";
}

string parse_store(string code) {
  string param1 = parse_ins_param(parse_first_param(code));
  string param2 = parse_ins_param(parse_second_param(code));
  int p_beg = 1, p_end;
  while (param2[p_beg] == '(')
    p_beg++;
  p_end = p_beg;
  while (param2[p_end] != '+')
    p_end++;
  string name = param2.substr(p_beg, p_end - p_beg);
  if (param2.find("--") != string::npos) {
    return name + "=" + param1;
  }
  param2.replace(p_beg, p_end - p_beg, "0");
  return name + "[" + param2 + "/8" + "]=" + param1;
}

string parse_ins_op(string code) {
  char *p = new char[code.size() + 1];
  strcpy(p, code.c_str());
  char op[20];
  sscanf(p, "%*s %*s %s", op);
  string oop = op;
  delete[] p;
  return oop;
}

/*return num of bl jump */
int parse_bl(string code) {
  char *p = new char[code.size() + 1];
  strcpy(p, code.c_str());
  char jump[20];
  sscanf(p, "%*s %*s %*s %*s %s", jump);
  string jjump = jump;
  jjump = jjump.substr(1, jjump.size() - 2);
  delete[] p;
  return str2int(jjump);
}

int parse_br(string code) {
  char *p = new char[code.size() + 1];
  strcpy(p, code.c_str());
  char jump[20];
  sscanf(p, "%*s %*s %*s %s", jump);
  string jjump = jump;
  jjump = jjump.substr(1, jjump.size() - 2);
  delete[] p;
  return str2int(jjump);
}