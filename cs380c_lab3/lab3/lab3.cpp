#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <stdio.h>
#include <string>
#include <utility>
#include <vector>

#include "lab3.hpp"

using namespace std;

int opt = 0;
int rep = 0;
int cfg = 0;

vector<int> renames;
int recounter = 0;
set<int> visit;
vector<string> code3add;
vector<string> modi3add;

vector<int> cfg_end;
vector<int> cfg_start;
vector<vector<int>> cfg_goto;

map<int, Def> def_t;
set<string> defname;

int counter;

void create_def();
void parse_instr();
string genname();
void insert_fi();
void rename(int cfgb, int cfge, string var);
void print_cfg();
void print_modified_3addr();
void clean_3addr();
int constant_propagate();
void constant_propagate_report(int num);
int statement_hoisted();
void statement_hoisted_report(int num);

int main(int argc, char *argv[])
{
    opt = str2int(argv[1]);
    rep = str2int(argv[2]);
    cfg = str2int(argv[3]);
    freopen("./loop_test.3addr", "r", stdin);
    init();
    string buf;

    while (getline(cin, buf))
    {
        code3add.push_back(buf);
    }
    counter = code3add.size() + 1;

    parse_instr();
    create_def();

    modi3add = code3add;
    for (vector<string>::iterator it = code3add.begin(); it != code3add.end(); it++)
    {
        string op = parse_ins_op(*it);
        if (op == "enter")
        {
            cfg_start.push_back(it - code3add.begin() + 1);
            continue;
        }
        if (op == "blbc" || op == "blbs")
        {
            cfg_start.push_back(parse_bl(*it));
            cfg_start.push_back(it - code3add.begin() + 2);
            continue;
        }
        if (op == "br")
        {
            cfg_start.push_back(parse_br(*it));
            cfg_start.push_back(it - code3add.begin() + 2);
            continue;
        }
        if (op == "call")
        {
            cfg_start.push_back(it - code3add.begin() + 2);
            continue;
        }
        if (op == "ret")
        {
            sort(cfg_start.begin(), cfg_start.end());
            cfg_start.resize(unique(cfg_start.begin(), cfg_start.end()) - cfg_start.begin());
            for (vector<int>::iterator it = cfg_start.begin(); it != cfg_start.end() - 1; it++)
            {
                int offset = *(it + 1) - 1;
                string code = *(code3add.begin() + offset - 1);
                string op = parse_ins_op(code);
                vector<int> go;
                if (op == "blbc" || op == "blbs")
                {
                    go.push_back(parse_bl(code));
                    go.push_back(*(it + 1));
                    cfg_goto.push_back(go);
                    continue;
                }
                if (op == "br")
                {
                    go.push_back(parse_br(code));
                    cfg_goto.push_back(go);
                    continue;
                }
                go.push_back(*(it + 1));
                cfg_goto.push_back(go);
            }
            for (unsigned i = 0; i < cfg_start.size() - 1; i++)
            {
                sort(cfg_goto[i].begin(), cfg_goto[i].end());
            }

            if (cfg == 1)
            {
                print_cfg();
            }

            insert_fi();

            if (opt == 1)
            {
                int num = constant_propagate();
                if (rep == 1)
                {
                    constant_propagate_report(num);
                }
            }

            if (opt == 2)
            {
                int num2 = statement_hoisted();
                if (rep == 2)
                {
                    statement_hoisted_report(num2);
                }
            }
            clean_3addr();
            if (!(cfg == 1 || rep != 0))
            {
                print_modified_3addr();
            }

            cfg_goto.clear();
            cfg_start.clear();
            continue;
        }
    }
    fclose(stdin);
    return 0;
}

void parse_instr()
{
    /*generate the interins for next use*/
    for (vector<string>::iterator it = code3add.begin(); it != code3add.end(); it++)
    {
        string op = parse_ins_op(*(it));
        if (op == "move")
        {
            interins.push_back(parse_move(*it));
            continue;
        }
        if (op == "load")
        {
            interins.push_back(parse_load(*it));
            continue;
        }
        if (op == "store")
        {
            interins.push_back(parse_store(*it));
            continue;
        }
        if (find(ariins.begin(), ariins.end(), op) != ariins.end())
        {
            interins.push_back(parse_ari((*it), op));
            continue;
        }
        interins.push_back("");
    }
}

void create_def()
{
    /*first loop to find all defition*/
    for (vector<string>::iterator it = interins.begin(); it != interins.end(); it++)
    {
        if ((*it).find("=") != string::npos && (*it).find("==") == string::npos && (*it).find("!=") == string::npos)
        {
            Def def;
            def.num = it - interins.begin() + 1;
            def.name = (*it).substr(0, (*it).find("="));
            def.value = (*it).substr((*it).find("=") + 1);
            def.isconst = iscons(def.value);
            def_t.insert(pair<int, Def>(def.num, def));
            defname.insert(def.name);
        }
    }
    for (map<int, Def>::iterator it = def_t.begin(); it != def_t.end(); it++)
    {
        for (map<int, Def>::iterator itt = def_t.begin(); itt != def_t.end(); itt++)
        {
            if ((*it).second.name == (*itt).second.name && (*it).second.num != (*itt).second.num)
            {
                (*it).second.kill.push_back((*itt).second.num);
            }
        }
        sort((*it).second.kill.begin(), (*it).second.kill.end());
    }
}

string genname()
{
    int i = recounter;
    renames.push_back(i);
    recounter = i + 1;
    return int2str(i);
}

void insert_fi()
{
    map<int, vector<int>> cfg_par;
    for (unsigned i = 0; i != cfg_start.size() - 1; i++)
    {
        for (unsigned j = 0; j != cfg_goto[i].size(); j++)
        {
            if (cfg_par.find(cfg_goto[i][j]) == cfg_par.end())
            {
                vector<int> temp;
                temp.push_back(cfg_start[i]);
                cfg_par[cfg_goto[i][j]] = temp;
            }
            else
            {
                (*cfg_par.find(cfg_goto[i][j])).second.push_back(cfg_start[i]);
            }
        }
    }
    map<int, vector<int>> cfg_idom = cfg_par;
    for (map<int, vector<int>>::iterator it = cfg_idom.begin(); it != cfg_idom.end(); it++)
    {
        sort((*it).second.begin(), (*it).second.end());
        while ((*it).second.size() > 0 && (*it).second[0] > (*it).first)
            (*it).second.erase((*it).second.begin());
        int maxsize = (*it).second.size();
        if (maxsize != 1)
        {
            while (equalvec((*it).second.begin(), (*it).second.end()) == false)
            {
                (*it).second[maxsize - 1] = cfg_idom[(*it).second[maxsize - 1]][0];
                sort((*it).second.begin(), (*it).second.end());
            }
        }
    }
    map<int, vector<int>> df;
    for (unsigned i = 0; i != cfg_start.size(); i++)
    {
        int blockbeg = cfg_start[i];
        if (cfg_par.find(blockbeg) == cfg_par.end() || cfg_par[blockbeg].size() == 1)
        {
            ;
        }
        else
        {
            for (unsigned j = 0; j < cfg_par[blockbeg].size(); j++)
            {
                int runner = cfg_par[blockbeg][j];
                while (runner != cfg_idom[blockbeg][0])
                {
                    df[runner].push_back(blockbeg);
                    runner = cfg_idom[runner][0];
                }
            }
        }
    }

    for (vector<int>::iterator it = cfg_start.begin(); it != cfg_start.end() - 1; it++)
    {
        if (*(it + 1) - (*it) > 1)
            cfg_end.push_back(*(it + 1) - 1);
        else
            cfg_end.push_back(*it);
    }
    cfg_end.push_back(code3add.size() - 1);

    /*insert the fi function*/
    for (set<string>::iterator it = defname.begin(); it != defname.end(); it++)
    {
        set<int> hasalready, everonworklist, worklist;
        string var = *it;
        for (map<int, Def>::iterator itt = def_t.begin(); itt != def_t.end(); itt++)
        {
            int num = 0;
            if (var == (*itt).second.name)
                num = (*itt).second.num;
            for (unsigned i = 1; i < cfg_start.size(); i++)
            {
                if (cfg_start[i] > num)
                {
                    num = cfg_start[i - 1];
                    break;
                }
            }
            worklist.insert(num);
            everonworklist.insert(num);
        }
        while (!worklist.empty())
        {
            int num = *(worklist.begin());
            worklist.erase(worklist.begin());
            for (unsigned i = 0; i < df[num].size(); i++)
            {
                int y = df[num][i];
                if (hasalready.find(y) == hasalready.end())
                {
                    // insert(y,var,counter);
                    for (vector<string>::iterator it = modi3add.begin(); it != modi3add.end(); it++)
                    {
                        int num = parse_ins_num(*it);
                        if (num == y)
                        {
                            string temp = "    instr " + int2str(counter) + ": fi " + var + "#* " + var + "#* " + var + "#* ";
                            // cout<<temp<<endl;
                            string movtemp = "    instr " + int2str(counter + 1) + ": move (" + int2str(counter) + ") " + var + "#* ";
                            // cout<<movtemp<<endl;
                            vector<string>::iterator itt;
                            itt = modi3add.insert(it, movtemp);
                            modi3add.insert(itt, temp);
                            break;
                        }
                    }
                    counter += 2;
                    hasalready.insert(y);
                    if (everonworklist.find(y) == everonworklist.end())
                    {
                        everonworklist.insert(y);
                        worklist.insert(y);
                    }
                }
            }
        }
    }
    for (set<string>::iterator it = defname.begin(); it != defname.end(); it++)
    {
        rename(cfg_start[0], cfg_end[0], *it);
        renames.clear();
        recounter = 0;
        visit.clear();
    }
    for (vector<string>::iterator itt = modi3add.begin(); itt != modi3add.end(); itt++)
    {
        string code = *itt;
        if (parse_ins_op(*itt) == "br")
        {
            int br_num = parse_br(*itt);
            string br_next = modi3add[br_num - 1];
            char *p = new char[br_next.size() + 1];
            strcpy(p, br_next.c_str());
            char jump[20];
            sscanf(p, "%*s %s", jump);
            string jjump = jump;
            jjump = jjump.substr(0, jjump.size() - 1);
            delete[] p;
            int br_new = str2int(jjump);
            int index1 = (*itt).find("[");
            jjump = *itt;
            jjump = jjump.substr(0, index1) + "[" + int2str(br_new) + "]";
            *itt = jjump;
        }
    }
}

void rename(int cfgb, int cfge, string var)
{
    if (visit.find(cfgb) == visit.end())
    {
        visit.insert(cfgb);
        int begnum, endnum, i = 0;
        for (vector<string>::iterator it = modi3add.begin(); it != modi3add.end(); it++)
        {
            int num = parse_ins_num(*it);
            if (num == cfgb - 1)
            {
                begnum = i + 1;
            }
            if (num == cfge)
            {
                endnum = i;
            }
            i++;
        }
        vector<string>::iterator bit = modi3add.begin() + begnum;
        for (int i = 0; i < endnum - begnum + 1; i++)
        {
            string code = *(bit + i);
            string op = parse_ins_op(code);
            string first_param = parse_ins_param(parse_first_param(code));
            if (op == "fi" && first_param == var)
            {
                *(bit + i + 1) = (*(bit + i + 1)).substr(0, (*(bit + i + 1)).size() - 2);
                *(bit + i + 1) = *(bit + i + 1) + genname();
            }
        }
        for (int i = 0; i < endnum - begnum + 1; i++)
        {
            string code = *(bit + i);
            string first_param = parse_ins_param(parse_first_param(code));
            // string second_param=parse_ins_param(parse_second_param(code));
            string op = parse_ins_op(code);
            string second_param = " ";
            if (find(ariins.begin(), ariins.end(), op) != ariins.end())
                second_param = parse_ins_param(parse_second_param(code));
            if (op != "fi")
                if (first_param == var && renames.size() >= 1)
                {
                    int index = renames[renames.size() - 1];
                    int pos = code.find_first_of("#") + 1;
                    (*(bit + i))[pos] = index + '0';
                    // cout<<code<<"is "<<code[pos]<<endl;
                    int spacepos = code.find(" ", pos);
                    // while((*(bit+i))[pos]<'9'&&(*(bit+i))[pos]>'0'&&pos<(*(bit+i)).size()-1)
                    //{
                    (*(bit + i)).erase(pos + 1, spacepos - pos - 1);
                    //}
                }
            if (second_param == var && op != "move" && renames.size() >= 1)
            {
                int pos = code.find_last_of("#");
                *(bit + i) = (*(bit + i)).substr(0, pos + 1) + int2str(renames[renames.size() - 1]);
            }
        }
        for (int i = 0; i < endnum - begnum + 1; i++)
        {
            string code = *(bit + i);
            string op = parse_ins_op(code);
            string frontop = parse_ins_op(*(bit + i - 1));
            if (frontop != "fi")
                if (op == "move")
                {
                    string second_param = parse_ins_param(parse_second_param(code));
                    if (second_param == var)
                    {
                        int pos = code.find_last_of("#");
                        *(bit + i) = (*(bit + i)).substr(0, pos + 1) + genname();
                        // cout<<*(bit+i)<<endl;
                    }
                }
        }
        int bnum = 0;
        while (cfg_start[bnum] != cfgb)
            bnum++;
        if (bnum >= cfg_goto.size())
            return;
        for (unsigned i = 0; i < cfg_goto[bnum].size(); i++)
        {
            int succcfgbeg = cfg_goto[bnum][i];
            int succcfgend;
            int index = 0;
            while (cfg_start[index] != succcfgbeg)
            {
                index++;
            }
            succcfgend = cfg_end[index];
            int offset = 0;
            for (vector<string>::iterator it = modi3add.begin(); it != modi3add.end(); it++)
            {
                int num = parse_ins_num(*it);
                if (num == succcfgbeg - 1)
                {
                    succcfgbeg = offset + 1;
                }
                if (num == succcfgend - 1)
                {
                    succcfgend = offset + 1;
                    break;
                }
                offset++;
            }
            for (vector<string>::iterator it = modi3add.begin() + succcfgbeg; it != modi3add.begin() + succcfgend; it++)
            {
                string code = *it;
                if (parse_ins_op(*it) == "fi")
                {
                    //*(it+1)=(*(it+1)).substr(0,(*(it+1)).find_last_of("#"))+int2str(renames.top());
                    if (parse_ins_param(parse_first_param(*it)) == var && renames.size() >= 1)
                    {
                        int pos = (*it).find_first_of("*");
                        string ren = int2str(renames[renames.size() - 1]);
                        (*it).replace(pos, 1, ren);
                    }
                }
            }
        }
        for (unsigned i = 0; i < cfg_goto[bnum].size(); i++)
        {
            int succcfgbeg = cfg_goto[bnum][i];
            int succcfgend;
            int index = 0;
            while (cfg_start[index] != succcfgbeg)
            {
                index++;
            }
            succcfgend = cfg_end[index];
            rename(succcfgbeg, succcfgend, var);
        }
        for (int i = 0; i < endnum - begnum + 1; i++)
        {
            string code = *(bit + i);
            string op = parse_ins_op(code);
            string second_param = parse_ins_param(parse_second_param(code));
            if (op == "move" && second_param == var)
            {
                renames.erase(renames.end() - 1);
            }
        }
    }
    else
    {
        return;
    }
}

void print_cfg()
{
    /*putout the cfg result of one function*/
    cout << "Function: " << cfg_start[0] << endl;
    cout << "Basic blocks:";
    for (unsigned i = 0; i < cfg_start.size(); i++)
        cout << " " << cfg_start[i];
    cout << endl;
    cout << "CFG:" << endl;
    for (unsigned i = 0; i < cfg_start.size() - 1; i++)
    {
        cout << cfg_start[i] << " ->";
        for (unsigned j = 0; j < cfg_goto[i].size(); j++)
        {
            cout << " " << cfg_goto[i][j];
        }
        cout << endl;
    }
    cout << cfg_start[cfg_start.size() - 1] << " ->" << endl; // add the last basic
}

void print_modified_3addr()
{
    for (vector<string>::iterator it = modi3add.begin(); it != modi3add.end(); it++)
    {
        cout << *it << endl;
    }
}

void clean_3addr()
{
    for (vector<string>::iterator itt = modi3add.begin(); itt != modi3add.end(); itt++)
    {
        string code = *itt;
        if (parse_ins_op(*itt) == "fi")
        {
            int index1 = (*itt).find("*");
            string jjump = *itt;
            jjump = jjump.substr(0, index1 - 3);
            *itt = jjump;
        }
    }
    for (vector<string>::iterator itt = modi3add.begin(); itt != modi3add.end(); itt++)
    {
        string code = *itt;
        string jjump = *itt;
        jjump = replace_all(jjump, "#", "$");
        *itt = jjump;
    }
}

int constant_propagate()
{
    map<string, string> def_value;
    for (vector<string>::iterator it = modi3add.begin(); it != modi3add.end(); it++)
    {
        string code = *it;
        string op = parse_ins_op(code);
        if (op == "move")
        {
            string def_name = parse_second_param(code);
            string value = parse_first_param(code);
            if (iscons(value))
            {
                def_value[def_name] = value;
            }
        }
    }
    int erasenum = 0;
    for (map<string, string>::iterator it = def_value.begin(); it != def_value.end(); it++)
    {
        string def = (*it).first;
        for (vector<string>::iterator itt = modi3add.begin(); itt != modi3add.end(); itt++)
        {
            if (parse_ins_op(*itt) != "move")
                if ((*itt).find(def) != string::npos)
                {
                    int pos = (*itt).find(def);
                    int len = def.size();
                    (*itt).replace(pos, len, (*it).second);
                    erasenum++;
                }
        }
    }
    return erasenum;
}

void constant_propagate_report(int num)
{
    cout << "Function: " << cfg_start[0] << endl;
    cout << "Number of constant propagate: " << num << endl;
}

int statement_hoisted()
{
    map<string, string> def_value;
    for (vector<string>::iterator it = modi3add.begin(); it != modi3add.end(); it++)
    {
        string code = *it;
        string op = parse_ins_op(code);
        if (op == "move")
        {
            string def_name = parse_second_param(code);
            string value = parse_first_param(code);
            def_value[def_name] = value;
        }
    }
    int erasenum = 0;
    int change = 1;
    while (change == 1)
    {
        for (map<string, string>::iterator it = def_value.begin(); it != def_value.end(); it++)
        {
            change = 0;
            string def = (*it).first;
            int appear = 0;
            int num = 0;
            for (vector<string>::iterator itt = modi3add.begin(); itt != modi3add.end(); itt++)
            {
                if ((*itt).find(def) != string::npos)
                {
                    if (appear == 0)
                    {
                        num = itt - modi3add.begin();
                        appear = 1;
                    }
                    else
                    {
                        appear = 2;
                        break;
                    }
                }
            }
            if (appear == 1)
            {
                vector<string>::iterator itt = modi3add.begin() + num;
                int rin = (*itt).find("move");
                string rcode = (*itt).substr(0, rin);
                *itt = rcode + "nop";
                if (parse_ins_op(*(itt - 1)) == "fi")
                {
                    int rin = (*(itt - 1)).find("fi");
                    string rcode = (*(itt - 1)).substr(0, rin);
                    *(itt - 1) = rcode + "nop";
                    erasenum++;
                }
                erasenum++;
                change = 1;
            }
        }
    }
    return erasenum;
}

void statement_hoisted_report(int num)
{
    cout << "Function: " << cfg_start[0] << endl;
    cout << "Number of statement hoisted: " << num << endl;
}