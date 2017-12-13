//
//  main.cpp
//  lab2
//
//  Created by Leonardo on 2017/10/22.
//  Copyright © 2017年 Leonardo. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <sstream>
#include <stack>
#include <set>
using namespace std;
// undefined = 0;if = 1;if-else 2;while 3;function 4;line 5;
#define UNDEFINED 0
#define METHOD_IF 1
#define METHOD_ELSE 2
#define METHOD_WHILE 3
#define FUNC_BEGIN 4
#define TMP_LINE 5
#define OUT_LINE 6
#define ADDRESS 7
#define NUMBER 8

#define MAX_ADDR 32768

#define OPT_NULL 0
#define OPT_SCP 1
#define OPT_DSE 2

#define BACKEND_C 0
#define BACKEND_3ADDR 1
#define BACKEND_CFG 2
#define BACKEND_REP 3

string tostring(int n)
{
    stringstream ss;
    ss << n;
    string result;
    ss >> result;
    return result;
}
class PARAM
{
public:
    int type; // 0 base; 1 offset
    string name;
    string parent_name;
    int size;
    int address;
};

class FUNC
{
public:
    string name;
    int size;
    int param_num;
    int start_number;
    int end_number;
    map<int, PARAM> local_stack;
    
    map<int, set<int > > BLOCK, PRED;
    set<int> scrblock, visited;// SCR block
    set<string> varname;
    vector<int> assignment;
    map<int, set<int > > gen,kill,in,out;
    map<int, set<string> > genvar;
    map<int, set<string> > def,use,invar,outvar;
    int constantspropagated, statementseliminatedinscr, statementseliminatedoutscr;
    void buildPRED()
    {
        map<int, set<int> >::iterator iter;
        set<int>::iterator jter;
        for(iter = BLOCK.begin();iter!=BLOCK.end();iter++)
            for(jter = iter->second.begin();jter!=iter->second.end();jter++)
                PRED[*jter].insert(iter->first);
    }
    void dfs(int n)
    {
        scrblock.insert(n);
        set<int>::iterator iter;
        for(iter = PRED[n].begin();iter != PRED[n].end();iter++)
        {
            if(visited.find(*iter) == visited.end())
            {
                visited.insert(*iter);
                dfs(*iter);
                visited.erase(*iter);
            }
        }
    }
    int getblock(int instr)
    {
        map<int, set<int> >::iterator iter;
        for(iter = BLOCK.begin();iter!=BLOCK.end();iter++)
        {
            if(iter->first > instr)
                break;
        }
        iter--;
        return iter->first;
    }
};

class LINE
{
public:
    string instruction; //first
    string raw_instr;
    string instr[2];    //second and third
    int instr_type[2];  //second and third type
    int instr_number[2];//                 number
    int number;// No
    int output_type;
    int func_number;
    string output_string;
    int address_number;
    int address_type;
    string index = "";
    string arrayname = "";
    //int parent_line;
    bool unlive;
    // FP 1 GP 0
    // undefined = 0;if = 1;if-else 2;while 3;function 4;line 5;
    // address = 6
    int first_begin;
    int first_end;
    int second_begin; // if-else
    int second_end;
    void check_instr(int i)
    {
        if(instr[i] == "")
        {
            instr_type[i] = -1;
            return;
        }
        int tmp_number;
        char tmp_buffer[50];
        if(instr[i] == "FP")
        {
            instr_type[i] = 3;
            return;
        }
        else if(instr[i] == "GP")
        {
            instr_type[i] = 4;
            return;
        }
        else if(instr[i][0] == '(')
        {
            sscanf(instr[i].c_str(), "(%d)", &tmp_number);
            instr_number[i] = tmp_number;
            instr_type[i] = 1;
            return;
        }
        else if(instr[i][0] == '[')
        {
            sscanf(instr[i].c_str(), "[%d]", &tmp_number);
            instr_number[i] = tmp_number;
            instr_type[i] = 2;
            return;
        }
        else if(isNum(instr[i]))
        {
            sscanf(instr[i].c_str(), "%d", &tmp_number);
            instr_number[i] = tmp_number;
            instr_type[i] = 0;
            return;
        }
        else if(instr[i].find("_base#") <= instr[i].length())
        {
            sscanf(instr[i].c_str(), "%[^#]#%d", tmp_buffer,&tmp_number);
            instr_number[i] = tmp_number;
            //instr[i] = tmp_buffer;
            instr[i] = instr[i].substr(0,instr[i].find("_base#"));
            instr_type[i] = 6;
            return;
        }
        else if(instr[i].find("_offset#") <= instr[i].length())
        {
            sscanf(instr[i].c_str(), "%[^#]#%d", tmp_buffer,&tmp_number);
            instr_number[i] = tmp_number;
            //instr[i] = tmp_buffer;
            instr[i] = instr[i].substr(0,instr[i].find("_offset#"));
            instr_type[i] = 7;
            return;
        }
        else if(instr[i].find("#") <= instr[i].length() )
        {
            sscanf(instr[i].c_str(), "%[^#]#%d", tmp_buffer,&tmp_number);
            instr_number[i] = tmp_number;
            instr[i] = tmp_buffer;
            instr_type[i] = 5;
            return;
        }
        instr_type[i] = -1;
    };
    bool isNum(string str)
    {
        for(int i=0;i<str.length();i++)
        {
            if(str[i]<'0'||str[i]>'9')
                return false;
        }
        return true;
    };
};
// instr    type
// num      0
// (num)    1
// [num]    2
// FP       3
// GP       4
// x#num    5
// x_base#  6
// x_offset#7

vector<LINE> LINE_VECTOR;
vector<FUNC> FUNC_VECTOR;
map<int, PARAM> global;
vector<PARAM> global_param;
vector<int> tmp_vector;
set<int> tmp_set;
map<int,vector<int> >::iterator tmp_iter;
map<int,set<int> >::iterator tmp_jter;
bool warningdiv0 = false;

string inverse(string tmp_string)
{
    string result = tmp_string;
    for(int i=0;i<tmp_string.length();i++)
    {
        if(tmp_string.substr(i,2) == "==")
        {
            tmp_string[i] = '!';
            result = tmp_string;
            break;
        }
        else if(tmp_string.substr(i,2) == "<=")
        {
            result = tmp_string.substr(0,i)+'>'+tmp_string.substr(i+2,tmp_string.length()-i-2);
            break;
        }
        else if(tmp_string.substr(i,2) == "<")
        {
            result = tmp_string.substr(0,i)+">="+tmp_string.substr(i+1,tmp_string.length()-i-1);
            break;
        }
    }
    return result;
}
string tooperator(string str)
{
    if (str == "add") return "+";
    if (str == "sub") return "-";
    if (str == "mul") return "*";
    if (str == "div") return "/";
    if (str == "mod") return "%";
    if (str == "cmpeq") return " == ";
    if (str == "cmple") return " <= ";
    if (str == "cmplt") return " < ";
    return "+";
}

bool defarray(string str)
{
    bool result = false;
    for(int i=0;i<str.length();i++)
    {
        if(str[i] == '[')
        {
            result = true;break;
        }
        else if(str[i] == '=')
        {
            break;
        }
    }
    return result;
}
string getdef(string str) // 返回赋值语句的被赋值变量
{
    //cout<<"getdef input "<<str<<endl;
    int begin, end;
    for (begin = 0; begin < str.length(); begin++)
    {
        if (str[begin] == '_' || (str[begin] >= 'a' && str[begin] <= 'z') || (str[begin] >= 'A' && str[begin] <= 'Z'))
        {
            break;
        }
    }
    for (end = begin+1; end < str.length(); end++)
    {
        if (!(str[end] == '_' || (str[end] >= 'a' && str[end] <= 'z') || (str[end] >= 'A' && str[end] <= 'Z') || (str[end] >= '0' && str[end] <= '9')))
        {
            break;
        }
    }
    //cout<<"getdef result "<<str.substr(begin, end)<<endl;
    return str.substr(begin, end);
}
set<string> getuse(string str) // 返回赋值语句中使用到的变量集
{
    int i;
    string curuse = "";
    set<string> result;
    for (i = 0; i < str.length(); i++)
    {
        if (str[i] == '(' || str[i] == '[' || str[i] == '=')
        {
            i++;
            break;
        }
    }
    for ( ; i < str.length(); i++)
    {
        if (str[i] == '_' || (str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z'))
        {
            curuse += str[i];
        }
        else if (curuse != "")
        {
            result.insert(curuse);
            curuse = "";
        }
    }
    if(curuse != "")
        result.insert(curuse);
    return result;
}
string calculate(string str) // 返回str表达式的运算结果。如果有除零则warningdiv0为true
{
    string result = str;
    bool brace = true;
    int lb=0, rb=0;
    while (brace) // 有括号则一直计算
    {
        brace = false;
        int i;
        for (i = 0; i < result.length(); i++) // 找匹配的括号对
        {
            if (result[i] == '(')
            {
                brace = true;
                lb = i;
            }
            else if (result[i] == ')')
            {
                if (brace)
                {
                    rb = i;
                    break;
                }
            }
            else if ((result[i] >= 'A' && result[i] <= 'Z') || (result[i] >= 'a' && result[i] <= 'z') || result[i] == '_')
            {
                brace = false;
            }
        }
        if (brace)
        {
            string num1 = "", op = "", num2 = "";
            for (i = lb+1; i < rb; i++)
            {
                if (result[i] >= '0' && result[i] <= '9')
                {
                    if (op == "")
                    {
                        num1 += result[i];
                    }
                    else
                    {
                        num2 += result[i];
                    }
                }
                else if (result[i] == '+' || result[i] == '-' || result[i] == '*' || result[i] == '/' || result[i] == '%')
                {
                    op += result[i];
                }
                else if (result.substr(i, 3) == " < " || result.substr(i, 3) == " > ")
                {
                    op += result[i+1];
                    i += 2;
                }
                else if (result.substr(i, 4) == " <= " || result.substr(i, 4) == " >= " || result.substr(i, 4) == " == " || result.substr(i, 4) == " != ")
                {
                    op += result.substr(i+1, 2);
                    i += 3;
                }
            }
            string tempresult;
            if (op == "+") tempresult = tostring(atoi(num1.c_str())+atoi(num2.c_str()));
            else if (op == "-") tempresult = tostring(atoi(num1.c_str())-atoi(num2.c_str()));
            else if (op == "*") tempresult = tostring(atoi(num1.c_str())*atoi(num2.c_str()));
            else if (op == "/")
            {
                if (num2 == "0") // 除零退出，下同
                {
                    warningdiv0 = true;
                    break;
                }
                tempresult = tostring(atoi(num1.c_str())/atoi(num2.c_str()));
            }
            else if (op == "%")
            {
                if (num2 == "0")
                {
                    warningdiv0 = true;
                    break;
                }
                tempresult = tostring(atoi(num1.c_str())%atoi(num2.c_str()));
            }
            else if (op == "<") tempresult = tostring(atoi(num1.c_str())<atoi(num2.c_str()));
            else if (op == ">") tempresult = tostring(atoi(num1.c_str())>atoi(num2.c_str()));
            else if (op == "==") tempresult = tostring(atoi(num1.c_str())==atoi(num2.c_str()));
            else if (op == "!=") tempresult = tostring(atoi(num1.c_str())!=atoi(num2.c_str()));
            else if (op == "<=") tempresult = tostring(atoi(num1.c_str())<=atoi(num2.c_str()));
            else if (op == ">=") tempresult = tostring(atoi(num1.c_str())>=atoi(num2.c_str()));
            else
            {
                break;
            }
            result = result.substr(0, lb)+tempresult+result.substr(rb+1, result.length()-rb-1); // 用新的计算结果取代旧的表达式
        }
    }
    return result;
}

void getconstant(string output, bool &init, bool &sameconstant, int &value) // 调用calculate计算output的值。如果value未初始化（init为false），则定义value，如果value已经初始化，则sameconstant取决于该计算结果是否等于value
{
    //cout<<"getconstant in "<<output<<endl;
    int begin=0, end=(int)output.length()-1;
    int i;
    for (i = 0; i < output.length(); i++) // 获取要计算的部分
    {
        if (output[i] == '=')
        {
            begin = i;
        }
        else if (output[i] == ';')
        {
            end = i;
        }
    }
    for (i = begin+1; i <= end; i++) // 如果要计算的部分有变量（即非定值），则不可能得到定值，sameconstant为false
    {
        if ((output[i] >= 'a' && output[i] <= 'z') || (output[i] >= 'A' && output[i] <= 'Z') || output[i] == '_')
        {
            sameconstant = false;
            return;
        }
    }
    string calculateresult = calculate(output.substr(begin+1, end-begin+1).c_str()).c_str(); // 计算定值表达式运算的结果
    int result = atoi(calculateresult.c_str());
    //cout<<"getconstant "<<output.substr(begin+1, end-begin+1).c_str()<<' '<<result<<endl;
    if (!init)
    {
        init = true;
        if (calculateresult[0] == '(') // 出现除零（例如calculateresult == "(2/0)"）
        {
            sameconstant =false;
        }
        else
        {
            value = result;
        }
    }
    else if (calculateresult[0] == '(' || result != value) // 定值一样且没有发生除零
    {
        sameconstant = false;
    }
}
string varreplace(string str, string var, string value) // 常量传播：用value替换var
{
    int i;
    //cout<<"varreplace "<<var<<' '<<value<<endl;
    string curuse = "";
    string result = str;
    for (i = 0; i < str.length(); i++) // 从赋值号或第一个"("（Write语句或函数调用）或第一个"["（对数组元素的赋值）开始替换
    {
        if (str[i] == '(' || str[i] == '[' || str[i] == '=')
        {
            i++;
            break;
        }
    }
    for ( ; i < str.length(); i++) // 找所有可替换的变量
    {
        if (str[i] == '_' || (str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z'))
        {
            curuse += str[i];
        }
        else if (curuse!= "")
        {
            if (curuse == var)
            {
                result = result.substr(0, i-curuse.length())+value+result.substr(i, result.length()-i);
                i -= curuse.length();
            }
            curuse = "";
        }
    }
    return result;
}


void readFile(string filename)
{
    stack<LINE> IF_stack;
    vector<string> FUNC_param_vector;
    
    //ifstream f;
    //f.open(filename);
    char buffer[50];
    char instr[50];
    char instr1[50];
    char instr2[50];
    int number;
    
    while(cin.getline(buffer,50) )
    {
        
        
        //cout<<"before read  "<<buffer<<endl;
        strcpy(instr,"");
        strcpy(instr1,"");
        strcpy(instr2,"");
        number = 0;
        sscanf(buffer,"    instr %d: %s %s %s",&number,instr,instr1,instr2);
        //cout<<"read in  "<<number<<' '<<instr<<' '<<instr1<<' '<<instr2<<endl;
        //cout<<"after read   "<<buffer<<endl;
        if(!number) continue;
        LINE* tmp_line = new LINE();
        tmp_line->raw_instr = buffer;
        tmp_line->number = number;
        tmp_line->instruction = instr;
        tmp_line->instr[0] = instr1;
        tmp_line->instr[1] = instr2;
        tmp_line->check_instr(0);
        tmp_line->check_instr(1);
        tmp_line->unlive = false;
        tmp_line->func_number = (int)FUNC_VECTOR.size()-1;
        //tmp_line->check_type();
        //cout<<tmp_line->raw_instr<<endl;
        //cout<<tmp_line->instr[0]<<' '<<tmp_line->instr_type[0]<<endl;
        //cout<<tmp_line->instr[1]<<' '<<tmp_line->instr_type[1]<<endl;
        
        if(tmp_line->instruction == "nop" || tmp_line->instruction == "entrypc")
        {
            tmp_line->func_number = -1;
            tmp_line->output_type = UNDEFINED;
        }
        else if(tmp_line->instruction == "enter")
        {
            FUNC* tmp_func = new FUNC();
            if(number > 2 && LINE_VECTOR[number - 2].instruction == "entrypc")
            {
                tmp_func->name = "main";
            }
            else
            {
                tmp_func->name = "func_" + to_string(FUNC_VECTOR.size());
            }
            tmp_func->start_number = number;
            
            tmp_vector.clear();
            //add block
            tmp_func->BLOCK.insert(map<int,set<int> >::value_type(number,set<int>()));
            
            FUNC_VECTOR.push_back(*tmp_func);
            tmp_line->output_type = FUNC_BEGIN;
            tmp_line->first_begin = number;
            tmp_line->func_number = (int)FUNC_VECTOR.size()-1;
        }
        else if(tmp_line->instruction == "ret")
        {
            tmp_line->func_number = (int)FUNC_VECTOR.size()-1;
            tmp_line->output_type = UNDEFINED;
            FUNC_VECTOR.back().param_num = tmp_line->instr_number[0]/8;
            LINE_VECTOR[FUNC_VECTOR.back().start_number-1].first_end = number;
            
            /*
            tmp_set.clear();
            FUNC_VECTOR.back().BLOCK.insert(map<int,set<int>>::value_type(number,tmp_set));
            for(tmp_jter = FUNC_VECTOR.back().BLOCK.begin(); tmp_jter!=FUNC_VECTOR.back().BLOCK.end(); tmp_jter++)
            {
                if(tmp_jter->first >= number)
                {
                    tmp_jter--;
                    tmp_jter->second.insert(number);
                    break;
                }
            }*/
            
            
        }
        else if(tmp_line->instruction == "blbc" || tmp_line->instruction == "blbs")
        {
            tmp_line->func_number = (int)FUNC_VECTOR.size()-1;
            tmp_line->output_type = METHOD_IF;
            if(tmp_line->instruction == "blbc")
            {
                tmp_line->output_string = LINE_VECTOR[tmp_line->instr_number[0]-1].output_string;
            }
            else
            {
                tmp_line->output_string = inverse(LINE_VECTOR[tmp_line->instr_number[0]-1].output_string);
            }
            tmp_line->first_begin = number;
            tmp_line->first_end = tmp_line->instr_number[1]-1;
            IF_stack.push(*tmp_line);
            
            int tmp_reg = tmp_line->instr_number[1];
            tmp_set.clear();
            FUNC_VECTOR.back().BLOCK.insert(map<int,set<int> >::value_type(number+1,tmp_set));
            FUNC_VECTOR.back().BLOCK.insert(map<int,set<int> >::value_type(tmp_reg,tmp_set));
            
            for(tmp_jter = FUNC_VECTOR.back().BLOCK.begin();tmp_jter!=FUNC_VECTOR.back().BLOCK.end();tmp_jter++)
            {
                if(tmp_jter->first >= number+1)
                    break;
            }
            tmp_jter--;
            tmp_jter->second.insert(number+1);
            tmp_jter->second.insert(tmp_reg);
            
            for(tmp_jter = FUNC_VECTOR.back().BLOCK.begin();tmp_jter!=FUNC_VECTOR.back().BLOCK.end();tmp_jter++)
            {
                if(tmp_jter->first >= tmp_reg)
                    break;
            }
            tmp_jter--;
            tmp_jter->second.insert(tmp_reg);
            //cout<<"insert"<<endl;
        }
        else if(tmp_line->instruction == "br")
        {
            //cout<<"br"<<endl;
            tmp_line->func_number = (int)FUNC_VECTOR.size()-1;
            tmp_line->output_type = UNDEFINED;
            int tmp_reg = tmp_line->instr_number[0];
            
            tmp_vector.clear();
            tmp_set.clear();

            if(tmp_reg < tmp_line->number)
            {
                for(int i=tmp_reg;i<tmp_line->number;i++)
                {
                    if(LINE_VECTOR[i-1].output_type == METHOD_IF)
                    {
                        LINE_VECTOR[i-1].output_type = METHOD_WHILE;

                        //FUNC_VECTOR.back().BLOCK.insert(map<int,set<int> >::value_type(tmp_reg,tmp_set));
                        //FUNC_VECTOR.back().BLOCK.insert(map<int,set<int>>::value_type(number+1,tmp_set));
                        for(tmp_jter = FUNC_VECTOR.back().BLOCK.begin();tmp_jter!=FUNC_VECTOR.back().BLOCK.end();tmp_jter++)
                        {
                            if(tmp_jter->first >= tmp_reg)
                                break;
                        }
                        tmp_jter--;
                        tmp_set = tmp_jter->second;
                        tmp_jter->second.clear();
                        tmp_jter->second.insert(tmp_reg);
                        //cout<<"insert "<<tmp_jter->first<<' '<<tmp_reg<<endl;
                        FUNC_VECTOR.back().BLOCK.insert(map<int,set<int> >::value_type(tmp_reg,tmp_set));

                        for(tmp_jter = FUNC_VECTOR.back().BLOCK.begin();tmp_jter!=FUNC_VECTOR.back().BLOCK.end();tmp_jter++)
                        {
                            if(tmp_jter->first > tmp_reg)
                            {
                                //cout<<"erase "<<tmp_jter->first<<' '<<number+1<<endl;
                                tmp_jter->second.erase(number+1);
                            }
                            if(tmp_jter->first >= number+1)
                                break;
                        }
                        tmp_jter--;
                        tmp_jter->second.insert(tmp_reg);
                        //cout<<"insert "<<tmp_jter->first<<' '<<tmp_reg<<endl;
                        tmp_set.clear();
                        
                        FUNC_VECTOR.back().BLOCK.insert(map<int,set<int> >::value_type(number+1,tmp_set));
                        break;
                    }
                }
            }
            else
            {
                while(IF_stack.size()!=0)
                {
                    LINE stack_line = IF_stack.top();
                    IF_stack.pop();
                    if(number == stack_line.first_end && tmp_reg >= stack_line.first_end && LINE_VECTOR[stack_line.number-1].output_type == METHOD_IF)
                    {
                        LINE_VECTOR[stack_line.number-1].output_type = METHOD_ELSE;
                        LINE_VECTOR[stack_line.number-1].second_end = tmp_reg;
                       break; 
                    }
                }

                tmp_set.clear();
                FUNC_VECTOR.back().BLOCK.insert(map<int,set<int> >::value_type(number+1,tmp_set));
                FUNC_VECTOR.back().BLOCK.insert(map<int,set<int> >::value_type(tmp_reg,tmp_set));
                
                for(tmp_jter = FUNC_VECTOR.back().BLOCK.begin();tmp_jter!=FUNC_VECTOR.back().BLOCK.end();tmp_jter++)
                {
                    if(tmp_jter->first >= number+1)
                        break;
                }
                tmp_jter--;
                tmp_jter->second.insert(tmp_reg);
                tmp_jter->second.erase(number+1);

                for(tmp_jter = FUNC_VECTOR.back().BLOCK.begin();tmp_jter!=FUNC_VECTOR.back().BLOCK.end();tmp_jter++)
                {
                    if(tmp_jter->first >= tmp_reg)
                        break;
                }
                tmp_jter--;
                tmp_jter->second.insert(tmp_reg);
                //cout<<"insert"<<endl;
            }
        }
        else if(tmp_line->instruction == "wrl")
        {
            tmp_line->func_number = (int)FUNC_VECTOR.size()-1;
            tmp_line->output_type = OUT_LINE;
            tmp_line->output_string = "WriteLine()";
        }
        else if(tmp_line->instruction == "add"||tmp_line->instruction == "sub"||tmp_line->instruction == "mul"||tmp_line->instruction == "div"||tmp_line->instruction == "mod"||tmp_line->instruction == "cmpeq"||tmp_line->instruction == "cmple"||tmp_line->instruction == "cmplt")
        {
            tmp_line->func_number = (int)FUNC_VECTOR.size()-1;
            tmp_line->output_type = TMP_LINE;
            // instr    type
            // num      0
            // (num)    1
            // [num]    2
            // FP       3
            // GP       4
            // x#num    5
            // x_base#  6
            // x_offset#7
            if(tmp_line->instruction == "add" && tmp_line->instr_type[1] == 4)
            {
                int tmp_addr = tmp_line->instr_number[0];
                string tmp_var = tmp_line->instr[0];
                PARAM *tmp_param = new PARAM();
                tmp_param->name = tmp_var;
                global.insert(map<int,PARAM>::value_type(tmp_addr,*tmp_param));
                
                tmp_line->output_type = ADDRESS;
                tmp_line->output_string = tmp_param->name;
                tmp_line->address_number = tmp_addr;
                tmp_line->address_type = 1;
                tmp_line->index = "";
                tmp_line->arrayname = tmp_var;
            }
            else if(tmp_line->instruction == "add" && tmp_line->instr_type[1] == 3)
            {
                tmp_line->output_string = tmp_line->instr[0];
                int tmp_addr = tmp_line->instr_number[0];
                string tmp_var = tmp_line->instr[0];
                
                PARAM *tmp_param = new PARAM();
                tmp_param->name = tmp_var;
                FUNC_VECTOR[FUNC_VECTOR.size()-1].local_stack.insert(map<int,PARAM>::value_type(tmp_addr,*tmp_param));
                
                tmp_line->output_type = ADDRESS;
                tmp_line->output_string = tmp_var;
                tmp_line->address_number = tmp_addr;
                tmp_line->address_type = 1;
                tmp_line->index = "";
                tmp_line->arrayname = tmp_var;

            }
            else if(tmp_line->instruction == "add" && tmp_line->instr_type[1] == 7)
            {
                LINE first_line = LINE_VECTOR[tmp_line->instr_number[0]-1];
                tmp_line->output_type = TMP_LINE;
                
                tmp_line->address_number = first_line.address_number + tmp_line->instr_number[1];
                tmp_line->address_type = 1;
                tmp_line->arrayname = first_line.arrayname;
                if(first_line.index == "" || first_line.index == "0")
                {
                    tmp_line->index = tostring(tmp_line->instr_number[1]/8);
                }
                else if(first_line.index != "" && tmp_line->instr_number[1] != 0)
                {
                    tmp_line->index = first_line.index + "+" +tostring(tmp_line->instr_number[1]/8);
                }
                else
                {
                    tmp_line->index = first_line.index;
                }
                
                if(tmp_line->index == "")
                    tmp_line->output_string = first_line.arrayname;
                else
                    tmp_line->output_string = first_line.arrayname +"["+tmp_line->index+"]";

                //cout<<tmp_line->raw_instr<<endl;
                //cout<<tmp_line->instr[0]<<' '<<tmp_line->instr_type[0]<<endl;
                //cout<<tmp_line->instr[1]<<' '<<tmp_line->instr_type[1]<<endl;
                //cout<<tmp_line->output_string<<' '<<tmp_line->index<<endl;
            }
            else
            {
                if(tmp_line->instr_type[0] == 0 && tmp_line->instr_type[1] == 1)
                {
                    tmp_line->output_type = TMP_LINE;
                    tmp_line->output_string = "(" + to_string(tmp_line->instr_number[0]) + tooperator(tmp_line->instruction)+LINE_VECTOR[tmp_line->instr_number[1]-1].output_string+ ")";
                }
                else if(tmp_line->instr_type[0] == 1 && tmp_line->instr_type[1] == 0)
                {
                    tmp_line->output_type = TMP_LINE;
                    tmp_line->output_string = "(" + LINE_VECTOR[tmp_line->instr_number[0]-1].output_string + tooperator(tmp_line->instruction) + to_string(tmp_line->instr_number[1]) + ")";
                }
                else if(tmp_line->instr_type[0] == 1 && tmp_line->instr_type[1] == 5)
                {
                    tmp_line->output_type = TMP_LINE;
                    tmp_line->output_string = "(" + LINE_VECTOR[tmp_line->instr_number[0]-1].output_string + tooperator(tmp_line->instruction) + tmp_line->instr[1] + ")";
                }
                else if(tmp_line->instr_type[0] == 1 && tmp_line->instr_type[1] == 1)
                {
                    //cout<<"log array step 1"<<endl;
                    //cout<<tmp_line->raw_instr<<endl;
                    //cout<<tmp_line->instr[0]<<' '<<tmp_line->instr_number[0]<<' '<<LINE_VECTOR[tmp_line->instr_number[0]-1].output_string<<endl;
                    //cout<<tmp_line->instr[1]<<' '<<tmp_line->instr_number[1]<<' '<<LINE_VECTOR[tmp_line->instr_number[1]-1].output_string<<endl;
                    
                    tmp_line->output_type = TMP_LINE;
                    LINE line_instr1 = LINE_VECTOR[tmp_line->instr_number[1]-1];
                    //cout<<"log array step 2"<<endl;
                    if(line_instr1.instruction == "mul" && line_instr1.instr_type[1] == 0)
                    {   
                        //cout<<"here 1"<<endl;
                        if(line_instr1.instr_number[1]%8 == 0)
                        {
                            tmp_line->arrayname = LINE_VECTOR[tmp_line->instr_number[0]-1].arrayname;
                            tmp_line->address_type = LINE_VECTOR[tmp_line->instr_number[0]-1].address_type;
                            if(line_instr1.instr_type[0] == 0)
                            {
                                tmp_line->output_string =LINE_VECTOR[tmp_line->instr_number[0]-1].output_string + "["+ to_string(line_instr1.instr_number[0])+"]";
                                tmp_line->index = to_string(line_instr1.instr_number[0]);   
                            }
                            else if(line_instr1.instr_type[0] == 5)
                            {
                                tmp_line->output_string =LINE_VECTOR[tmp_line->instr_number[0]-1].output_string + "["+ line_instr1.instr[0]+"]";
                                tmp_line->index = line_instr1.instr[0];
                            }
                            else if(line_instr1.instr_type[0] == 1)
                            {
                                tmp_line->output_string =LINE_VECTOR[tmp_line->instr_number[0]-1].output_string + "["+ LINE_VECTOR[line_instr1.instr_number[0]-1].output_string+"]";
                                tmp_line->index = LINE_VECTOR[line_instr1.instr_number[0]-1].output_string;
                            }
                            if(line_instr1.instr_number[1] != 8)
                            {
                                tmp_line->index += "*" + to_string(line_instr1.instr_number[1]/8);
                                tmp_line->output_string = tmp_line->arrayname +"["+tmp_line->index+"]";
                                tmp_line->address_type = 1;
                            }
                            else if(tmp_line->address_type == 1)
                            {
                                if(LINE_VECTOR[tmp_line->instr_number[0]-1].index != "" && LINE_VECTOR[tmp_line->instr_number[0]-1].index != "0")
                                    tmp_line->index = LINE_VECTOR[tmp_line->instr_number[0]-1].index + "+" +tmp_line->index;
                                else
                                    tmp_line->index = tmp_line->index;
                                tmp_line->output_string = tmp_line->arrayname +"["+tmp_line->index+"]";
                            }
                            //tmp_line->parent_line = line_instr1.parent_line;
                            
                            
                        }
                        else
                        {
                            //cout<<"here 2"<<endl;
                            tmp_line->output_type = TMP_LINE;
                            tmp_line->output_string = "("+LINE_VECTOR[tmp_line->instr_number[0]-1].output_string + tooperator(tmp_line->instruction)+LINE_VECTOR[tmp_line->instr_number[1]-1].output_string+")";
                        }
                        //tmp_line->output_string =LINE_VECTOR[tmp_line->instr_number[0]-1].output_string + "["+ to_string(LINE_VECTOR[tmp_line->instr_number[1]-1].output_number/8)+"]";
                    }
                    else if(LINE_VECTOR[tmp_line->instr_number[1]-1].output_type == ADDRESS)
                    {
                        cout<<"cunzai"<<endl;
                        //暂时不存在
                    }
                    else if(LINE_VECTOR[tmp_line->instr_number[0]-1].output_type != ADDRESS)
                    {
                        //cout<<"!ADDRESS"<<endl;
                        tmp_line->output_type = TMP_LINE;
                        tmp_line->output_string = "("+LINE_VECTOR[tmp_line->instr_number[0]-1].output_string + tooperator(tmp_line->instruction)+LINE_VECTOR[tmp_line->instr_number[1]-1].output_string+")";
                        //cout<<tmp_line->raw_instr<<endl;
                        //cout<<tmp_line->instr[0]<<' '<<tmp_line->instr_number[0]<<' '<<tmp_line->instr_type[0]<<endl;
                        //cout<<tmp_line->instr[1]<<' '<<tmp_line->instr_number[1]<<' '<<tmp_line->instr_type[1]<<endl;
                        //cout<<LINE_VECTOR[tmp_line->instr_number[0]-1].output_string<<' '<<LINE_VECTOR[tmp_line->instr_number[1]-1].output_string<<endl;
                        //cout<<tmp_line->output_string<<endl;
                    }
                    else
                    {
                        cout<<"here 2"<<endl;
                        tmp_line->output_type = TMP_LINE;
                        tmp_line->output_string = "("+LINE_VECTOR[tmp_line->instr_number[0]-1].output_string + tooperator(tmp_line->instruction)+LINE_VECTOR[tmp_line->instr_number[1]-1].output_string+")";
                    }
                }
                else if(tmp_line->instr_type[0] == 0 && tmp_line->instr_type[1] == 0)
                {
                    tmp_line->output_type = TMP_LINE;
                    tmp_line->output_string = "("+ to_string(tmp_line->instr_number[0]) + tooperator(tmp_line->instruction)+ to_string(tmp_line->instr_number[1])+")";
                    //if(tmp_line->instruction == "mul")
                    //{
                    //    tmp_line->output_number = tmp_line->instr_number[0] * tmp_line->instr_number[1];
                    //}
                }
                else if(tmp_line->instr_type[0] == 5 && tmp_line->instr_type[1] == 0)
                {
                    //cout<<tmp_line->raw_instr<<endl;
                    //cout<<tmp_line->instr[0]<<' '<<tmp_line->instr_number[0]<<' '<<tmp_line->instr_type[0]<<endl;
                    //cout<<tmp_line->instr[1]<<' '<<tmp_line->instr_number[1]<<' '<<tmp_line->instr_type[1]<<endl;
                    tmp_line->output_type = TMP_LINE;
                    tmp_line->output_string = "("+ tmp_line->instr[0] + tooperator(tmp_line->instruction)+ to_string(tmp_line->instr_number[1])+")";
                    //cout<<tmp_line->output_string<<endl;
                }
                else if(tmp_line->instr_type[0] == 5 && tmp_line->instr_type[1] == 1)
                {
                    tmp_line->output_type = TMP_LINE;
                    tmp_line->output_string = "("+ tmp_line->instr[0] + tooperator(tmp_line->instruction)+ LINE_VECTOR[tmp_line->instr_number[1]-1].output_string+")";
                }
                else if(tmp_line->instr_type[0] == 5 && tmp_line->instr_type[1] == 5)
                {
                    tmp_line->output_type = TMP_LINE;
                    tmp_line->output_string = "("+ tmp_line->instr[0] + tooperator(tmp_line->instruction)+ tmp_line->instr[1]+")";
                }
                else if(tmp_line->instr_type[0] == 0 && tmp_line->instr_type[1] == 5)
                {
                    tmp_line->output_type = TMP_LINE;
                    tmp_line->output_string = "("+ to_string(tmp_line->instr_number[0]) + tooperator(tmp_line->instruction) +tmp_line->instr[1]+")";
                }
                //处理数组，struct的情况
            }
        }
        else if(tmp_line->instruction == "move")
        {
            tmp_line->output_type = OUT_LINE;
            tmp_line->func_number = (int)FUNC_VECTOR.size()-1;
            //cout<<tmp_line->raw_instr<<' '<<tmp_line->raw_instr.find("_base#")<<' '<<tmp_line->raw_instr.find("#")<<endl;
            //cout<<tmp_line->instr[0]<<' '<<tmp_line->instr_number[0]<<' '<<tmp_line->instr_type[0]<<endl;
            //cout<<tmp_line->instr[1]<<' '<<tmp_line->instr_number[1]<<' '<<tmp_line->instr_type[1]<<endl;
            if(tmp_line->instr_type[0] == 1 && tmp_line->instr_type[1] == 5)
            {
                tmp_line->output_string = tmp_line->instr[1] + "=" + LINE_VECTOR[tmp_line->instr_number[0]-1].output_string;
                //cout<<tmp_line->instr[1]<<' '<<LINE_VECTOR[tmp_line->instr_number[0]-1].output_string<<endl;
            }
            else if(tmp_line->instr_type[0] == 0 && tmp_line->instr_type[1] == 5)
            {
                tmp_line->output_string = tmp_line->instr[1] + "=" + to_string(tmp_line->instr_number[0]);
                //cout<<tmp_line->instr[1]<<' '<<tmp_line->instr_number[0]<<endl;
            }
            else if(tmp_line->instr_type[0] == 5 && tmp_line->instr_type[1] == 5)
            {
                tmp_line->output_string = tmp_line->instr[1] + "=" + tmp_line->instr[0];
            }
        }
        else if(tmp_line->instruction == "load")
        {
            LINE first_line = LINE_VECTOR[tmp_line->instr_number[0]-1];
            tmp_line->func_number = (int)FUNC_VECTOR.size()-1;
            //cout<<tmp_line->raw_instr<<endl;
            //cout<<tmp_line->instr[0]<<' '<<tmp_line->instr_type[0]<<endl;
            //cout<<tmp_line->instr[1]<<' '<<tmp_line->instr_type[1]<<endl;
            tmp_line->output_type = TMP_LINE;
            tmp_line->output_string = LINE_VECTOR[tmp_line->instr_number[0]-1].output_string;
            /*if(first_line.address_type)
            {
                if(first_line.index == "")
                    tmp_line->output_string = first_line.arrayname;
                else
                    tmp_line->output_string = first_line.arrayname +"["+first_line.index+"]";
                    
            }
            else
                tmp_line->output_string = LINE_VECTOR[tmp_line->instr_number[0]-1].output_string;*/
            //cout<<tmp_line->output_string<<endl;
        }
        else if(tmp_line->instruction == "store")
        {
            //cout<<"log store"<<endl;
            //cout<<tmp_line->raw_instr<<endl;
            //cout<<tmp_line->instr[0]<<' '<<tmp_line->instr_type[0]<<tmp_line->instr_number[0]<<endl;
            //cout<<tmp_line->instr[1]<<' '<<tmp_line->instr_type[1]<<tmp_line->instr_number[1]<<' '<<LINE_VECTOR[tmp_line->instr_number[1]-1].output_string<<endl;
            LINE first_line = LINE_VECTOR[tmp_line->instr_number[1]-1];
            string tmp_out = "";
            tmp_line->output_type = OUT_LINE;
            tmp_line->func_number = (int) FUNC_VECTOR.size()-1;
            tmp_line->output_string = LINE_VECTOR[tmp_line->instr_number[1]-1].output_string + " = ";
            if(tmp_line->instr_type[0] == 0)
                tmp_line->output_string += to_string(tmp_line->instr_number[0]);
            else if(tmp_line->instr_type[0] == 1)
            {
                tmp_line->output_string += LINE_VECTOR[tmp_line->instr_number[0]-1].output_string;
                //cout<<' '<<LINE_VECTOR[tmp_line->instr_number[0]-1].output_string<<endl;
            }
            else if(tmp_line->instr_type[0] == 5)
                tmp_line->output_string += tmp_line->instr[0];
            
            /*if(first_line.address_type)
            {
                if(first_line.index == "")
                {
                    tmp_line->output_string = first_line.arrayname;
                }
                else
                {
                    tmp_line->output_string = first_line.arrayname +"["+first_line.index+"]";
                }
            }
            tmp_line->output_string += "=" + tmp_out;*/
        }
        else if(tmp_line->instruction == "param")
        {
            tmp_line->func_number = (int)FUNC_VECTOR.size()-1;
            if(tmp_line->instr_type[0] == 0)
                FUNC_param_vector.push_back(to_string(tmp_line->instr_number[0]));
            else if(tmp_line->instr_type[0] == 1)
                FUNC_param_vector.push_back(LINE_VECTOR[tmp_line->instr_number[0]-1].output_string);
            else if(tmp_line->instr_type[0] == 5)
                FUNC_param_vector.push_back(tmp_line->instr[0]);
        }
        else if(tmp_line->instruction == "call")
        {
            tmp_line->output_type = OUT_LINE;
            tmp_line->func_number = (int) FUNC_VECTOR.size()-1;
            FUNC_VECTOR[LINE_VECTOR[tmp_line->instr_number[0]-1].func_number].param_num = (int)FUNC_param_vector.size();
            tmp_line->output_string = FUNC_VECTOR[LINE_VECTOR[tmp_line->instr_number[0]-1].func_number].name;
            tmp_line->output_string += "(";
            for(int i=0;i<FUNC_param_vector.size();i++)
            {
                if(i == 0)
                {
                    tmp_line->output_string += FUNC_param_vector[i];
                }
                else
                {
                    tmp_line->output_string += ","+FUNC_param_vector[i];
                }
            }
            tmp_line->output_string += ")";
            FUNC_param_vector.clear();
            
            
            
            for(tmp_jter = FUNC_VECTOR.back().BLOCK.begin();tmp_jter!=FUNC_VECTOR.back().BLOCK.end();tmp_jter++)
            {
                if(tmp_jter->first > number)
                    break;
            }
            tmp_jter--;
            tmp_set = tmp_jter->second;
            tmp_jter->second.clear();
            tmp_jter->second.insert(number+1);
            FUNC_VECTOR.back().BLOCK.insert(map<int,set<int> >::value_type(number+1,tmp_set));
            
        }
        else if(tmp_line->instruction == "write")
        {
            
            tmp_line->output_type = OUT_LINE;
            tmp_line->func_number = (int)FUNC_VECTOR.size()-1;
            tmp_line->output_string = "WriteLong(";
            if(tmp_line->instr_type[0] == 0)
                tmp_line->output_string += tmp_line->instr_number[0];
            else if(tmp_line->instr_type[0] == 1)
                tmp_line->output_string += LINE_VECTOR[tmp_line->instr_number[0]-1].output_string;
            else if(tmp_line->instr_type[0] == 5)
                tmp_line->output_string += tmp_line->instr[0];
            tmp_line->output_string += ")";
        }
        
        if(tmp_line->instr_type[1] != 3 && tmp_line->instr_type[1] != 4)
        {
            if(tmp_line->instr_type[0] == 5 || tmp_line->instr_type[0] == 6)
            {
                int tmp_addr = tmp_line->instr_number[0];
                string tmp_var = tmp_line->instr[0];
                PARAM *tmp_param = new PARAM();
                tmp_param->name = tmp_var;
                FUNC_VECTOR[FUNC_VECTOR.size()-1].local_stack.insert(map<int,PARAM>::value_type(tmp_addr,*tmp_param));
            }
            
            if(tmp_line->instr_type[1] == 5 || tmp_line->instr_type[1] == 6)
            {
                int tmp_addr = tmp_line->instr_number[1];
                string tmp_var = tmp_line->instr[1];
                PARAM *tmp_param = new PARAM();
                tmp_param->name = tmp_var;
                FUNC_VECTOR[FUNC_VECTOR.size()-1].local_stack.insert(map<int,PARAM>::value_type(tmp_addr,*tmp_param));
            }
            
        }
        LINE_VECTOR.push_back(*tmp_line);
    }
}
void SCP_function()
{
    for(int i=0;i<LINE_VECTOR.size();i++)
    {
        int tmp_FUNC_NUMBER =LINE_VECTOR[i].func_number;
        if(tmp_FUNC_NUMBER >= 0)
        {
            if(LINE_VECTOR[i].instruction == "enter" )
            {
                map<int,PARAM>::iterator iter;
                for(iter = FUNC_VECTOR[tmp_FUNC_NUMBER].local_stack.begin();iter!= FUNC_VECTOR[tmp_FUNC_NUMBER].local_stack.end();iter++)
                {
                    FUNC_VECTOR[tmp_FUNC_NUMBER].varname.insert(iter->second.name);
                    //cout<<"varname "<<iter->second.name<<endl;
                }
            }
            else if(LINE_VECTOR[i].instruction == "store"||LINE_VECTOR[i].instruction == "move")
            {
                if(!defarray(LINE_VECTOR[i].output_string))
                {
                    if(FUNC_VECTOR[tmp_FUNC_NUMBER].varname.find(getdef(LINE_VECTOR[i].output_string)) != FUNC_VECTOR[tmp_FUNC_NUMBER].varname.end())
                    {
                        FUNC_VECTOR[tmp_FUNC_NUMBER].assignment.push_back(i+1);
                        //cout<<"assignment "<<i+1<<endl;
                    }
                }
                else
                {
                    FUNC_VECTOR[tmp_FUNC_NUMBER].varname.erase(getdef(LINE_VECTOR[i].output_string));
                    //cout<<"assignment erase "<<getdef(LINE_VECTOR[i].output_string)<<endl;
                }
            }
            else if(LINE_VECTOR[i].instruction == "ret")
            {
                FUNC *tmp_f = &FUNC_VECTOR[tmp_FUNC_NUMBER];
                tmp_f->constantspropagated = 0;
                tmp_f->buildPRED();
                map<int,set<int> >::iterator iter;
                for(iter = tmp_f->BLOCK.begin();iter!=tmp_f->BLOCK.end();iter++)
                {
                    tmp_f->gen.insert(map<int,set<int> >::value_type(iter->first,set<int>()));
                    tmp_f->kill.insert(map<int,set<int> >::value_type(iter->first,set<int>()));
                    tmp_f->in.insert(map<int,set<int> >::value_type(iter->first,set<int>()));
                    tmp_f->out.insert(map<int,set<int> >::value_type(iter->first,set<int>()));
                }
                for(int i=(int)tmp_f->assignment.size()-1;i>=0;i--)
                {
                    int tmp_num = tmp_f->assignment[i];
                    string tmp_output = LINE_VECTOR[tmp_num-1].output_string;
                    string tmp_def = getdef(tmp_output);
                    int tmp_block = tmp_f->getblock(tmp_num);
                    if(tmp_f->varname.find(tmp_def)!= tmp_f->varname.end() &&
                       tmp_f->genvar[tmp_block].find(tmp_def) == tmp_f->genvar[tmp_block].end())
                    {
                        tmp_f->gen[tmp_block].insert(tmp_num);
                        tmp_f->genvar[tmp_block].insert(tmp_def);
                    }
                }
                for(int i=0;i<tmp_f->assignment.size();i++)
                {
                    int tmp_num = tmp_f->assignment[i];
                    string tmp_output = LINE_VECTOR[tmp_num-1].output_string;
                    string tmp_def = getdef(tmp_output);
                    int tmp_block = tmp_f->getblock(tmp_num);
                    if(tmp_f->varname.find(tmp_def)!=tmp_f->varname.end())
                    {
                        //cout<<"getdef "<<tmp_def<<endl;
                        for(int k=0;k<tmp_f->assignment.size();k++)
                        {
                            //cout<<"outstring def "<<LINE_VECTOR[tmp_f->assignment[k]-1].output_string<<' '<<getdef(LINE_VECTOR[tmp_f->assignment[k]-1].output_string)<<endl;
                            if(i!=k && tmp_def == getdef(LINE_VECTOR[tmp_f->assignment[k]-1].output_string))
                            {
                                tmp_f->kill[tmp_block].insert(tmp_f->assignment[k]);
                                //cout<<"KILL "<<tmp_block<<' '<<tmp_num<<endl;
                            }
                        }
                    }
                }
                
                bool changed =  true;
                while(changed)
                {
                    changed = false;
                    for(iter = tmp_f->BLOCK.begin();iter!=tmp_f->BLOCK.end();iter++)
                    {
                        int tmp_block = iter->first;
                        set<int>::iterator tmp_i_iter,tmp_j_iter;
                        for(tmp_i_iter = tmp_f->PRED[tmp_block].begin();tmp_i_iter!= tmp_f->PRED[tmp_block].end();tmp_i_iter++)
                            for(tmp_j_iter = tmp_f->out[*tmp_i_iter].begin();tmp_j_iter!= tmp_f->out[*tmp_i_iter].end();tmp_j_iter++)
                            {
                                tmp_f->in[tmp_block].insert(*tmp_j_iter);
                            }
                        set<int > tmp_set = tmp_f->in[tmp_block];
                        
                        for(tmp_i_iter = tmp_f->kill[tmp_block].begin();tmp_i_iter != tmp_f->kill[tmp_block].end(); tmp_i_iter++)
                        {
                            tmp_set.erase(*tmp_i_iter);
                        }
                        for(tmp_j_iter = tmp_f->gen[tmp_block].begin();tmp_j_iter != tmp_f->gen[tmp_block].end();tmp_j_iter++)
                        {
                            tmp_set.insert(*tmp_j_iter);
                        }
                        
                        if(tmp_set != tmp_f->out[tmp_block])
                        {
                            changed = true;
                        }
                        tmp_f->out[tmp_block] = tmp_set;
                    }
                }
            }
        }
    }
    set<int > curin;
    /*
    map<int,set<int> >::iterator iter;
    for(int i=0;i<FUNC_VECTOR.size();i++)
        for(iter = FUNC_VECTOR[i].BLOCK.begin();iter!=FUNC_VECTOR[i].BLOCK.end();iter++)
        {
            int tmp_block = iter->first;
            set<int > tmp_set = FUNC_VECTOR[i].in[tmp_block];
            set<int>::iterator tmp_i_iter,tmp_j_iter;
            cout<<"BLOCK: "<<tmp_block<<endl;
            cout<<"IN: "<<endl;
            for(tmp_i_iter = tmp_set.begin();tmp_i_iter != tmp_set.end(); tmp_i_iter++)
            {
                cout<<*tmp_i_iter<<' ';
            }
            cout<<endl;
            cout<<"OUT: "<<endl;
            tmp_set = FUNC_VECTOR[i].out[tmp_block];
            for(tmp_i_iter = tmp_set.begin();tmp_i_iter != tmp_set.end(); tmp_i_iter++)
            {
                cout<<*tmp_i_iter<<' ';
            }
            cout<<endl;
        }
    */
    for(int i=0;i<LINE_VECTOR.size();i++)
    {
        LINE tmp_line = LINE_VECTOR[i];
        if(tmp_line.func_number >= 0 && tmp_line.instruction != "ret")
        {
            FUNC *tmp_f = &FUNC_VECTOR[tmp_line.func_number];
            if(tmp_f->BLOCK.find(tmp_line.number) != tmp_f->BLOCK.end())
            {
                curin.clear();
                curin = tmp_f->in[tmp_line.number];
            }
            if(tmp_line.output_type == TMP_LINE || tmp_line.instruction == "param" || tmp_line.instruction == "wrl" || tmp_line.instruction == "write")
            {
                for(int j=0;j<=1;j++)
                {
                    string varname = "";
                    if(tmp_line.instr_type[j] == 5 && tmp_f->varname.find(tmp_line.instr[j])!=tmp_f->varname.end())
                    {
                        varname = LINE_VECTOR[i].instr[j];
                        int value = 0;bool init = false;
                        bool sameconstant = true;set<int>::iterator iter;
                        
                        for(iter = curin.begin();iter!=curin.end();iter++)
                        {
                            if(sameconstant && varname == getdef(LINE_VECTOR[*iter-1].output_string))
                            {
                                getconstant(LINE_VECTOR[*iter-1].output_string, init, sameconstant, value);
                            }
                            if(!sameconstant)
                                break;
                        }
                        if(init && sameconstant)
                        {
                            LINE_VECTOR[i].instr[j] = tostring(value);
                            LINE_VECTOR[i].instr_type[j] = 0;
                            tmp_f->constantspropagated++;
                            //cout<<"constantspropagated "<<tmp_f->name<<' '<<i<<endl; 
                            if(j==0&&(tmp_line.instruction == "wrl" || tmp_line.instruction=="write"))
                            {
                                set<string> varuse = getuse(LINE_VECTOR[i].output_string);
                                set<string>::iterator tmpset_iter;
                                for(tmpset_iter = varuse.begin();tmpset_iter!=varuse.end();tmpset_iter++)
                                {
                                    if(tmp_f->varname.find(*tmpset_iter) != tmp_f->varname.end())
                                    {
                                        int value = 0;bool init = false;
                                        bool sameconstant = true;set<int>::iterator iter;
                                        for(iter = curin.begin();iter!=curin.end();iter++)
                                        {
                                            if(sameconstant && *tmpset_iter == getdef(LINE_VECTOR[*iter-1].output_string))
                                            {
                                                getconstant(LINE_VECTOR[*iter-1].output_string, init, sameconstant, value);
                                            }
                                            if(!sameconstant)
                                                break;
                                        }
                                        if(init && sameconstant)
                                        {
                                            LINE_VECTOR[i].output_string = varreplace(LINE_VECTOR[i].output_string, *tmpset_iter, tostring(value));
                                            //cout<<"0 change C instr"<<i<<' '<<LINE_VECTOR[i].output_string<<endl;
                                        }
                                    }
                                }
                                LINE_VECTOR[i].output_string = calculate(LINE_VECTOR[i].output_string);
                                //cout<<"1 change C instr"<<i<<' '<<LINE_VECTOR[i].output_string<<endl;
                            }
                        }
                    }
                }
            }
            else if(tmp_line.instruction == "store" || tmp_line.instruction == "move")
            {
                string varname ="";
                if(tmp_line.instr_type[0] == 5 && tmp_f->varname.find(tmp_line.instr[0])!=tmp_f->varname.end())
                {
                    varname = LINE_VECTOR[i].instr[0];
                    int value = 0;bool init = false;
                    bool sameconstant = true;set<int>::iterator iter;
                    
                    for(iter=curin.begin();iter != curin.end();iter++)
                    {
                        if(sameconstant && varname == getdef(LINE_VECTOR[*iter-1].output_string))
                        {
                            getconstant(LINE_VECTOR[*iter-1].output_string, init, sameconstant, value);
                        }
                        if(!sameconstant) break;
                    }
                    if(init && sameconstant)
                    {
                        LINE_VECTOR[i].instr[0] = tostring(value);
                        LINE_VECTOR[i].instr_type[0] = 0;
                        tmp_f->constantspropagated++;
                        //cout<<"constantspropagated "<<tmp_f->name<<' '<<i<<endl;
                    }
                }
                set<string> varuse = getuse(LINE_VECTOR[i].output_string);
                set<string>::iterator tmpset_iter;
                for(tmpset_iter = varuse.begin();tmpset_iter!=varuse.end();tmpset_iter++)
                {
                    if(tmp_f->varname.find(*tmpset_iter) != tmp_f->varname.end())
                    {
                        int value = 0;bool init = false;
                        bool sameconstant = true;set<int>::iterator iter;
                        
                        for(iter = curin.begin();iter!=curin.end();iter++)
                        {
                            //cout<<"2 "<<*tmpset_iter<<' '<<getdef(LINE_VECTOR[*iter].output_string)<<endl;
                            if(sameconstant && *tmpset_iter == getdef(LINE_VECTOR[*iter-1].output_string))
                            {
                                //cout<<"2 "<<*tmpset_iter<<' '<<LINE_VECTOR[*iter-1].output_string<<endl;
                                getconstant(LINE_VECTOR[*iter-1].output_string, init, sameconstant, value);
                            }
                            if(!sameconstant)
                                break;
                        }
                        //cout<<"2 init & sameconstant "<<init<<' '<<sameconstant<<endl;
                        if(init && sameconstant)
                        {
                            LINE_VECTOR[i].output_string = varreplace(LINE_VECTOR[i].output_string, *tmpset_iter, tostring(value));
                            //cout<<"2 change C instr"<<i<<' '<<LINE_VECTOR[i].output_string<<endl;
                        }
                    }
                }
                LINE_VECTOR[i].output_string = calculate(LINE_VECTOR[i].output_string);
                //cout<<"3 change C instr"<<i<<' '<<LINE_VECTOR[i].output_string<<endl;
                
                varname = getdef(LINE_VECTOR[i].output_string);
                if(tmp_f->varname.find(varname) != tmp_f->varname.end())
                {
                    set<int >::iterator iter,jter;
                    for(iter = curin.begin();iter!=curin.end();)
                    {
                        jter = iter;
                        if(varname == getdef(LINE_VECTOR[*iter-1].output_string))
                        {
                            iter++;
                            curin.erase(*jter);
                        }
                        else
                        {
                            iter++;
                        }
                    }
                    curin.insert(tmp_line.number);
                }
            }
            else if(tmp_line.instruction == "call" || tmp_line.instruction == "blbc" || tmp_line.instruction =="blbs")
            {
                set<string > varuse = getuse(tmp_line.output_string);
                set<string >::iterator tmpset_iter;
                for(tmpset_iter = varuse.begin();tmpset_iter!=varuse.end();tmpset_iter++)
                {
                    if(tmp_f->varname.find(*tmpset_iter) != tmp_f->varname.end())
                    {
                        int value = 0;bool init = false;
                        bool sameconstant = true;set<int>::iterator iter;
                        
                        for(iter=curin.begin();iter!=curin.end();iter++)
                        {
                            if(sameconstant && *tmpset_iter == getdef(LINE_VECTOR[*iter-1].output_string))
                            {
                                getconstant(LINE_VECTOR[*iter-1].output_string, init, sameconstant, value);
                            }
                            if(!sameconstant)
                                break;
                        }
                        if(init && sameconstant)
                        {
                            LINE_VECTOR[i].output_string = varreplace(LINE_VECTOR[i].output_string, *tmpset_iter, tostring(value));
                            //cout<<"4 change C instr"<<i<<' '<<LINE_VECTOR[i].output_string<<endl;
                        }
                    }
                }
                LINE_VECTOR[i].output_string = calculate(LINE_VECTOR[i].output_string);
                if((tmp_line.instruction == "blbs" || LINE_VECTOR[i].instruction == "blbc")&&tmp_line.output_string[0]!= '(')
                {
                    LINE_VECTOR[i].output_string = "("+LINE_VECTOR[i].output_string+")";
                    //cout<<"5 change C instr"<<i<<' '<<LINE_VECTOR[i].output_string<<endl;
                }
            }
        }
    }
    if(warningdiv0)
    {
        cerr << "[Warning] division by zero"<<endl;
    }
}
void DSE_function()
{
    for(int i=0;i<FUNC_VECTOR.size();i++)
    {
        map<int, set<int> >::iterator iter;
        set<int>::iterator jter;
        FUNC_VECTOR[i].buildPRED();
        
        for(iter = FUNC_VECTOR[i].BLOCK.begin();iter!=FUNC_VECTOR[i].BLOCK.end();iter++)
            for(jter = iter->second.begin();jter!=iter->second.end();jter++)
            {
                if(*jter < iter->first)
                {
                    FUNC_VECTOR[i].visited.clear();
                    FUNC_VECTOR[i].visited.insert(iter->first);
                    FUNC_VECTOR[i].visited.insert(*jter);
                    FUNC_VECTOR[i].scrblock.insert(*jter);
                    FUNC_VECTOR[i].dfs(iter->first);
                }
            }
        FUNC_VECTOR[i].statementseliminatedinscr = FUNC_VECTOR[i].statementseliminatedoutscr = 0;
        
    }
    /*
    set<int >::iterator tmp_iter;
    for(int i=0;i<FUNC_VECTOR.size();i++)
    {   
        cout<<"FUNC_VECTOR "<<i<<" :"<<endl;
        for(tmp_iter = FUNC_VECTOR[i].scrblock.begin();tmp_iter != FUNC_VECTOR[i].scrblock.end();tmp_iter++)
        {
            cout<<*tmp_iter<<" "<<endl;
        }
        cout<<endl;
    }
    */
    for(int i=0;i<LINE_VECTOR.size();i++)
    {
        int tmp_FUNC_NUMBER =LINE_VECTOR[i].func_number;
        LINE tmp_line = LINE_VECTOR[i];
        if(tmp_FUNC_NUMBER >= 0)
        {
            if(LINE_VECTOR[i].instruction == "enter")
            {
                map<int, PARAM>::iterator iter;
                map<int, set<int> >::iterator jter;
                map<int, string>::iterator kter;
                for(iter = FUNC_VECTOR[tmp_FUNC_NUMBER].local_stack.begin();iter!= FUNC_VECTOR[tmp_FUNC_NUMBER].local_stack.end();iter++)
                {
                    FUNC_VECTOR[tmp_FUNC_NUMBER].varname.insert(iter->second.name);
                }
                for(jter = FUNC_VECTOR[tmp_FUNC_NUMBER].BLOCK.begin();jter != FUNC_VECTOR[tmp_FUNC_NUMBER].BLOCK.end();jter++)
                {
                    FUNC_VECTOR[tmp_FUNC_NUMBER].def.insert(map<int, set<string > >::value_type(jter->first,set<string>()));
                    FUNC_VECTOR[tmp_FUNC_NUMBER].use.insert(map<int, set<string > >::value_type(jter->first,set<string>()));
                    FUNC_VECTOR[tmp_FUNC_NUMBER].invar.insert(map<int, set<string > >::value_type(jter->first,set<string>()));
                    FUNC_VECTOR[tmp_FUNC_NUMBER].outvar.insert(map<int, set<string > >::value_type(jter->first,set<string>()));
                }
            }
            else if(tmp_line.instruction == "move" || tmp_line.instruction == "store" || tmp_line.instruction == "call" || tmp_line.instruction == "wrl" || tmp_line.instruction == "write" || tmp_line.instruction == "blbc" || tmp_line.instruction == "blbs")
            {
                set<string> varuse = getuse(LINE_VECTOR[i].output_string);
                set<string>::iterator iterset;
                int block = FUNC_VECTOR[tmp_FUNC_NUMBER].getblock(i+1);
                for(iterset = varuse.begin();iterset!= varuse.end();iterset++)
                {
                    if(FUNC_VECTOR[tmp_FUNC_NUMBER].def[block].find(*iterset) == FUNC_VECTOR[tmp_FUNC_NUMBER].def[block].end())
                    {
                        FUNC_VECTOR[tmp_FUNC_NUMBER].use[block].insert(*iterset);
                    }
                }
                
                if(tmp_line.instruction == "move" || tmp_line.instruction == "store")
                {
                    string vardef = getdef(LINE_VECTOR[i].output_string);
                    if(FUNC_VECTOR[tmp_FUNC_NUMBER].use[block].find(vardef) == FUNC_VECTOR[tmp_FUNC_NUMBER].use[block].end())
                    {
                        FUNC_VECTOR[tmp_FUNC_NUMBER].def[block].insert(vardef);
                    }
                }
                
            }
            else if(tmp_line.instruction == "ret")
            {
                FUNC *tmp_f = &FUNC_VECTOR[tmp_line.func_number];
                map<int, set<int> >::iterator iter;
                set<string>::iterator iterset;
                bool unchange = false;
                while(!unchange)
                {
                    unchange = true;
                    iter = tmp_f->BLOCK.end();
                    do
                    {
                        iter--;
                        int block = iter->first;
                        set<int>::iterator jter;
                        for(jter = tmp_f->BLOCK[block].begin();jter!= tmp_f->BLOCK[block].end();jter++)
                        {
                            for(iterset = tmp_f->invar[*jter].begin();iterset!= tmp_f->invar[*jter].end();iterset++)
                            {
                                //cout<<"outvar "<<block<<" insert "<<*iterset<<endl;
                                tmp_f->outvar[block].insert(*iterset);
                            }
                        }
                        set<string> tempset = tmp_f->outvar[block];
                        for(iterset = tmp_f->def[block].begin();iterset != tmp_f->def[block].end();iterset++)
                        {
                            //cout<<"invar "<<block<<" erase "<<*iterset<<endl;
                            tempset.erase(*iterset);
                        }
                        for(iterset = tmp_f->use[block].begin();iterset != tmp_f->use[block].end();iterset++)
                        {
                            //cout<<"invar "<<block<<" insert "<<*iterset<<endl;
                            tempset.insert(*iterset);
                        }
                        if(tempset != tmp_f->invar[block])
                        {
                            unchange = false;
                        }
                        tmp_f->invar[block] = tempset;
                    }
                    while(iter!= tmp_f->BLOCK.begin());
                }
            }
        }
        
    }
    set<string> curout;
    for(int i= (int)LINE_VECTOR.size()-1;i>=0;i--)
    {
        int tmp_FUNC_NUMBER =LINE_VECTOR[i].func_number;
        FUNC* tmp_f = &FUNC_VECTOR[tmp_FUNC_NUMBER];
        if(tmp_FUNC_NUMBER>=0 && LINE_VECTOR[i].instruction!= "enter")
        {
            
            if(LINE_VECTOR[i].instruction == "ret" || tmp_f->BLOCK.find(i+2)!= tmp_f->BLOCK.end())
            {
                curout.clear();
                curout = tmp_f->outvar[tmp_f->getblock(i+1)];
                /*
                cout<<"outvar "<<tmp_f->getblock(i+1)<<endl;
                set<string >::iterator tmp_iter;
                for(tmp_iter=curout.begin();tmp_iter!=curout.end();tmp_iter++)
                cout<<*tmp_iter<<" ";
                cout<<endl;
                */
                 
            }
            if(LINE_VECTOR[i].output_type == TMP_LINE || LINE_VECTOR[i].instruction == "wrl" || LINE_VECTOR[i].instruction == "write" || LINE_VECTOR[i].instruction == "param")
            {
                for(int j=0;j<=1;j++)
                {
                    string varname = "";
                    if(LINE_VECTOR[i].instr[j]!="" && LINE_VECTOR[i].instr_type[j]>=5&& tmp_f->varname.find(LINE_VECTOR[i].instr[j])!= tmp_f->varname.end())
                    {
                        //cout<<"insert "<<i<<' '<<LINE_VECTOR[i].instr[j]<<endl;
                        curout.insert(LINE_VECTOR[i].instr[j]);
                    }
                }
            }
            else if(LINE_VECTOR[i].instruction == "store" || LINE_VECTOR[i].instruction == "move")
            {
                string varname = getdef(LINE_VECTOR[i].output_string);
                //cout<<"unlive "<<i<<' '<<varname<<endl;
                if(tmp_f->varname.find(varname) != tmp_f->varname.end())
                {
                    set<string >::iterator tmp_iter;
                    /*
                    for(tmp_iter=curout.begin();tmp_iter!=curout.end();tmp_iter++)
                    cout<<*tmp_iter<<" ";
                    cout<<endl;
                    */
                    if(curout.find(varname) == curout.end())
                    {
                        LINE_VECTOR[i].unlive = true;
                        if(tmp_f->scrblock.find(tmp_f->getblock(i+1)) != tmp_f->scrblock.end())
                        {
                            tmp_f->statementseliminatedinscr++;
                            //cout<<"statementseliminatedinscr "<<tmp_f->name<<' '<<tmp_f->statementseliminatedinscr<<endl;
                        }
                        else
                        {
                            tmp_f->statementseliminatedoutscr++;
                            //cout<<"statementseliminatedoutscr "<<tmp_f->name<<' '<<tmp_f->statementseliminatedoutscr<<endl;
                        }
                    }
                    curout.erase(varname);
                }
                if(LINE_VECTOR[i].instr_type[0]>=5&&LINE_VECTOR[i].instr[0]!=""&&tmp_f->varname.find(LINE_VECTOR[i].instr[0]) != tmp_f->varname.end())
                {
                    curout.insert(LINE_VECTOR[i].instr[0]);
                }
            }
        }
    }
}

void print_indent(int indent)
{
    for(int i=0;i<indent;i++)
        cout<<"    ";
}
void print_line(int begin,int end,int indent)
{
    int i= begin;
    LINE tmp_line;
    map<int, PARAM>::iterator iter = global.end();
    
    while(i<=end)
    {
        tmp_line = LINE_VECTOR[i-1];
        i++;
        if(tmp_line.output_type == FUNC_BEGIN)
        {
            int tmp_func = tmp_line.func_number;
            if(FUNC_VECTOR[tmp_func].name == "main")
                cout<<"int "<<FUNC_VECTOR[tmp_func].name<<"(";
            else
                cout<<"void "<<FUNC_VECTOR[tmp_func].name<<"(";
            // 之后需要添加输出参数
            
            iter = FUNC_VECTOR[tmp_line.func_number].local_stack.end();
            if (!FUNC_VECTOR[tmp_line.func_number].local_stack.empty())
            {
                iter--;
            }
            for(int i=0;i<FUNC_VECTOR[tmp_line.func_number].param_num;i++)
            {
                if(i == 0)
                {
                    cout<<"long ";
                }
                else
                {
                    cout<<",long ";
                }
                if(!FUNC_VECTOR[tmp_line.func_number].local_stack.empty() && iter != FUNC_VECTOR[tmp_line.func_number].local_stack.end() && (iter->first == 8+8*(FUNC_VECTOR[tmp_line.func_number].param_num-i))&& iter->first >= 0)
                {
                    cout << (iter->second).name;
                    if (iter != FUNC_VECTOR[tmp_line.func_number].local_stack.begin())
                    {
                        iter--;
                    }
                    else
                    {
                        iter = FUNC_VECTOR[tmp_line.func_number].local_stack.end();
                    }
                }
                else
                {
                    cout<<"arg_"<<i;
                }
            }
            cout<<")"<<endl;
            cout<<"{"<<endl;
            // 之后需要添加输出临时变量
            if (!FUNC_VECTOR[tmp_line.func_number].local_stack.empty())
            {
                int lastaddr = 0;
                iter = FUNC_VECTOR[tmp_line.func_number].local_stack.end();
                do
                {
                    iter--;
                    //cout<<"last addr "<<lastaddr<<" iter->first "<<iter->first<<endl;
                    if (iter->first <0 && lastaddr-(iter->first) == 8)
                    {
                        print_indent(1);
                        cout << "long " << (iter->second).name << ";" << endl;
                    }
                    else if(iter->first <0 && (iter->second).type != 1)
                    {
                        print_indent(1);
                        cout << "long " << (iter->second).name << "[" << (lastaddr-(iter->first))/8 << "];" << endl;
                    }
                    else if(iter->first > 0)
                        continue;
                    lastaddr = iter->first;
                }
                while (iter != FUNC_VECTOR[tmp_line.func_number].local_stack.begin());
            }
            
            print_line(tmp_line.first_begin+1, tmp_line.first_end, indent+1);
            if(FUNC_VECTOR[tmp_func].name == "main")
            {
                print_indent(1);cout<<"return 0;"<<endl;
            }
            cout<<"}"<<endl;
            i = tmp_line.first_end+1;
        }
        else if(tmp_line.output_type == METHOD_IF)
        {
            print_indent(indent);cout<<"if( "<<tmp_line.output_string<<" )"<<endl;
            print_indent(indent);cout<<"{"<<endl;
            print_line(tmp_line.first_begin +1, tmp_line.first_end, indent+1);
            print_indent(indent);cout<<"}"<<endl;
            i = tmp_line.first_end+1;
        }
        else if(tmp_line.output_type == METHOD_WHILE)
        {
            print_indent(indent);cout<<"while( "<<tmp_line.output_string<<" )"<<endl;
            print_indent(indent);cout<<"{"<<endl;
            print_line(tmp_line.first_begin +1, tmp_line.first_end, indent+1);
            print_indent(indent);cout<<"}"<<endl;
            i = tmp_line.first_end+1;
        }
        else if(tmp_line.output_type == METHOD_ELSE)
        {
            print_indent(indent);cout<<"if( "<<tmp_line.output_string<<" )"<<endl;
            print_indent(indent);cout<<"{"<<endl;
            print_line(tmp_line.first_begin +1, tmp_line.first_end, indent+1);
            print_indent(indent);cout<<"}"<<endl;
            print_indent(indent);cout<<"else"<<endl;
            print_indent(indent);cout<<"{"<<endl;
            print_line(tmp_line.first_end +1, tmp_line.second_end, indent+1);
            print_indent(indent);cout<<"}"<<endl;
            i = tmp_line.second_end +1;
        }
        else if (tmp_line.output_type == OUT_LINE)
        {
            print_indent(indent);
            if(tmp_line.unlive) cout<<"//";
            cout<<tmp_line.output_string<<";"<<endl;
        }
    }
}

void SCR_find(int FUNC_id)
{
    for(int i=0;i<FUNC_VECTOR.size();i++)
    {
        map<int,set<int> >::iterator iter;
        set<int>::iterator jter;
        FUNC_VECTOR[i].buildPRED();
        for(iter = FUNC_VECTOR[i].BLOCK.begin();iter!=FUNC_VECTOR[i].BLOCK.end();iter++)
        {
            for(jter = iter->second.begin();jter!=iter->second.end();jter++)
            {
                if(*jter < iter->first)
                {
                    FUNC_VECTOR[i].visited.clear();
                    FUNC_VECTOR[i].visited.insert(iter->first);
                    FUNC_VECTOR[i].visited.insert(*jter);
                    FUNC_VECTOR[i].scrblock.insert(*jter);
                    FUNC_VECTOR[i].dfs(iter->first);
                }
            }
        }
        
    }
}

void prtin_BLOCK()
{
    set<int>::iterator set_iter;
    for(int i=0;i<FUNC_VECTOR.size();i++)
    {
        cout<<"Function: "<<FUNC_VECTOR[i].start_number<<endl;
        cout<<"Basic blocks:";
        for(tmp_jter = FUNC_VECTOR[i].BLOCK.begin();tmp_jter!=FUNC_VECTOR[i].BLOCK.end();tmp_jter++)
        {
            cout<<" "<<tmp_jter->first;
        }
        cout<<endl;
        cout<<"CFG:"<<endl;
        for(tmp_jter = FUNC_VECTOR[i].BLOCK.begin();tmp_jter!=FUNC_VECTOR[i].BLOCK.end();tmp_jter++)
        {
            cout<<tmp_jter->first<<" ->";
            for(set_iter=tmp_jter->second.begin();set_iter!=tmp_jter->second.end();set_iter++)
            {
                cout<<" "<<*set_iter;
            }
            cout<<endl;
        }
    }
}

void print_3a_LINE()
{
    for(int i=0;i<LINE_VECTOR.size();i++)
    {
        cout<<' '<<"instr "<<i+1<<": ";
        cout<<LINE_VECTOR[i].instruction;
        for(int j=0;j<=1;j++)
        {
            cout<<' '<<LINE_VECTOR[i].instr[j];
            if(LINE_VECTOR[i].instr_type[j] == 5)
                cout<<"#"<<LINE_VECTOR[i].instr_number[j];
            else if(LINE_VECTOR[i].instr_type[j] == 6)
                cout<<"_base#"<<LINE_VECTOR[i].instr_number[j];
            else if(LINE_VECTOR[i].instr_type[j] == 7)
                cout<<"_offset#"<<LINE_VECTOR[i].instr_number[j];
        }
        if(LINE_VECTOR[i].unlive == true) cout<<" unlive";
        cout<<endl;
    }
}

void print_C_LINE()
{
    cout<<"#include <stdio.h>"<<endl;
    cout<<"#define WriteLine() printf(\"\\n\");"<<endl;
    cout<<"#define WriteLong(x) printf(\" %lld\", (long)x);"<<endl;
    cout<<"#define ReadLong(a) if (fscanf(stdin, \"%lld\", &a) != 1) a = 0;"<<endl;
    cout<<"#define long long long"<<endl;
    map<int, PARAM>::iterator iter = global.end();
    int lastaddr = MAX_ADDR;
    if(!global.empty())
    {
        do
        {
            iter--;
            if (lastaddr-(iter->first) == 8)
            {
                cout << "long " << (iter->second).name << ";" << endl;
            }
            else
            {
                cout << "long " << (iter->second).name << "[" << (lastaddr-(iter->first))/8 << "];" << endl;
            }
            lastaddr = iter->first;
        }
        while(iter != global.begin());
        
    }
    print_line(1,(int)LINE_VECTOR.size(),0);
}
int main(int argc, const char * argv[]) {
    char inputstr[10];
    int tital = 0;
    int i,j,k;
    int mode[2] = {OPT_NULL,BACKEND_C};
    if(argc >= 2)
    {
        if (!strcmp(argv[1], "-opt=scp")) mode[0] = OPT_SCP;
        else if (!strcmp(argv[1], "-opt=dse")) mode[0] = OPT_DSE;
        else if (!strcmp(argv[1], "-backend=c")) mode[1] = BACKEND_C;
        else if (!strcmp(argv[1], "-backend=3addr")) mode[1] = BACKEND_3ADDR;
        else if (!strcmp(argv[1], "-backend=cfg")) mode[1] = BACKEND_CFG;
        else if (!strcmp(argv[1], "-backend=rep")) mode[1] = BACKEND_REP;
    }
    if(argc >= 3)
    {
        if (!strcmp(argv[2], "-backend=c")) mode[1] = BACKEND_C;
        else if (!strcmp(argv[2], "-backend=3addr")) mode[1] = BACKEND_3ADDR;
        else if (!strcmp(argv[2], "-backend=cfg")) mode[1] = BACKEND_CFG;
        else if (!strcmp(argv[2], "-backend=rep")) mode[1] = BACKEND_REP;
    }
    // insert code here...
    //cout<<"begin"<<endl;
    readFile("example-scp");
    //cout<<"read done"<<endl;
    if (mode[0] == OPT_SCP) SCP_function();
    else if (mode[0] == OPT_DSE) DSE_function();
    
    if(mode[1] == BACKEND_C) print_C_LINE();
    else if(mode[1] == BACKEND_CFG) prtin_BLOCK();
    else if(mode[1] == BACKEND_3ADDR) print_3a_LINE();
    else if(mode[1] == BACKEND_REP)
    {
        if (mode[0] == OPT_SCP)
        {
            for (i = 0; i < FUNC_VECTOR.size(); i++)
            {
                cout << "Function: " << FUNC_VECTOR[i].BLOCK.begin()->first << endl;
                cout << "Number of constants propagated: " << FUNC_VECTOR[i].constantspropagated << endl;
            }
        }
        else if (mode[0] == OPT_DSE)
        {
            for (i = 0; i < FUNC_VECTOR.size(); i++)
            {
                cout << "Function: " << FUNC_VECTOR[i].BLOCK.begin()->first << endl;
                cout << "Number of statements eliminated in SCR: " << FUNC_VECTOR[i].statementseliminatedinscr << endl;
                cout << "Number of statements eliminated not in SCR: " << FUNC_VECTOR[i].statementseliminatedoutscr << endl;
            }
        }
    }
    //cout<<"scp function done"<<endl;
    //print_3a_LINE();
    
    //print_C_LINE();
    
    //prtin_BLOCK();
    return 0;
}


