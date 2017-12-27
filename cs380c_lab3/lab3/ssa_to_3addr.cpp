#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

#include "lab3.hpp"

using namespace std;

int main()
{
    vector<string> modi3add;
    string buf;
    while (getline(cin, buf))
    {
        modi3add.push_back(buf);
    }
    for (vector<string>::iterator it = modi3add.begin(); it != modi3add.end();
         it++)
    {
        string op = parse_ins_op(*it);
        string param = parse_ins_param(parse_first_param(*it));
        if (op == "fi")
        {
            (*it) = (*it).substr(0, (*it).find("fi")) + "nop";
            vector<string>::iterator nextit = it + 1;
            if (parse_ins_op(*nextit) == "move")
            {
                
                (*nextit) = (*nextit).substr(0, (*nextit).find("move")) + "nop";
            }
        }
        cout << *it << endl;
    }
}
