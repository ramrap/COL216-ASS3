// c++14
#include <iostream>
#include <fstream>
//  remove_if, sort, min, max, reverse,  merge, binary_search, is_sorted, unique, replace_if, count, push_heap, pop_heap, make_heap
#include <algorithm>
#include <vector>
#include <stdexcept>
// .push, .pop, .front, .back
#include <queue>
// .front, .back, .push_back, push_front, pop_back, pop_front, .at (slow)
#include <deque>
// map<string, int> m; m["x"] = 2; auto it = m.find("x"); it != m.end(); it->second; m.erase(it); m.erase("x");
#include <map>
// can take custom binary cmp function, 
// set<string> a; a.insert("f"); set<string>iterator it = a.find("f); it != a.end(); *it; a.erase("f");
#include <set> 
#include <cstdio> // printf, scanf // scanf("%d", &i); // read integer
#include <stdlib.h>
#include <assert.h> // assert
#include <utility> // pair, make_pair
#include <functional>
#include <string> 
#include <stack> // .pop .push .size .top .empty
#include <math.h> // cos, sin, tan, acos, asin, atan2, exp, log, log10, log2, pow, sqrt, hypot, cell, floor, round, fabs, abs
#include <unordered_map>
using namespace std;

#define ll long long int
#define fo(i,n) for(int i=0;i<n;i++)
#define fab(i,a,b) for(int i=a;i<b;i++)
#define rfo(i,n) for(int i=n;i>0;i--)

//pair
#define pii pair<int,int>
#define F first
#define S second

// vector
#define pb(x) push_back(x)




typedef pair<int, int> PII; // first, second
typedef vector<char> VC;
typedef vector<VC> VVC;
typedef vector<int> VI;
typedef vector<string> VS;
typedef vector<VI> VVI;
typedef map<int,int> MPII;



// build with: g++ main.cpp -o main -std=c++14
//./main > test.out
ll max_size=(1<<20)/4;
vector<int>t(100000,0);
// t0=0-2000, t1=2000*1-2000*2 , t2= 2000*2 .......   
// vector<int>
vector<string> inst;
ll inst_size;
//map for memory
unordered_map<int,int> memory;
unordered_map<string,int> registers;

const string WS = " \t";
unordered_map<string,int> labels;     // 'branch_mname -> line number
struct Instruction
{
    /* data */
    string kw;
    vector<string>vars;
    vector<int>args;
};
vector<Instruction> instruction_list;

// functions for map reference
// begin() – Returns an iterator to the first element in the map
// end() – Returns an iterator to the theoretical element that follows last element in the map
// size() – Returns the number of elements in the map
// max_size() – Returns the maximum number of elements that the map can hold
// empty() – Returns whether the map is empty
// pair insert(keyvalue, mapvalue) – Adds a new element to the map
// erase(iterator position) – Removes the element at the position pointed by the iterator
// erase(const g)– Removes the key value ‘g’ from the map
// clear() – Removes all the elements from the map



//write fucntions here

void memory_check(vector<Instruction> instruction_list){
    ll size=0;
    for(int i=0;i<instruction_list.size();i++){
        Instruction temp = instruction_list[i];
        string operation =temp.kw;

        if (operation=="addi"){
            // cout<<"addi he"<<endl;
           size++;
        }
        else if(operation=="add"){
            // cout<<"add bro"<<endl;
            size++;
        }
        else if(operation=="sub"){
            size++;
        }
        else if (operation=="mul"){
           size++;
        }
        else if(operation=="beq"){
           size++;

        }
        else if(operation=="bne"){
            size++;

        }
        else if(operation=="slt"){
            size++;
        }
        else if(operation=="j"){
            size++;

        }
        else if(operation=="lw"){
            size++;

        }
        else if(operation=="sw"){
            size++;
        }
        
    }
    if(size>max_size){
        throw invalid_argument("memory exceeded : Too many Instructions");
    }
}

string trimmed(string line){
    size_t first = line.find_first_not_of(WS);
    size_t last = line.find_last_not_of(WS);
    string ans;
    if (first==string::npos){
        ans= "";
    }
    else {
        ans= line.substr(first, last-first +1);
    }
    return ans;

}
// Instrction in
// kw
// vector<string> temp = in.vars
// var.get_key(temp[0])
// add $t0, $t0, $t1 
// sub $t0, $t0, $t1 
// mul $t0, $t0, $t1 
// beq $t0, $t1, label 
// bne $t0, $t1, label 
// slt $t0, $t1, $t2 
// j label -done -done
// lw $t0, 1000 -done
// sw $t0, 1000 -done
// addi $t0, $t0, 8 -done
// branch:
void parse(string line){
    string keyword;
    vector<string> variables;
    vector<int> arguments;
    size_t first = line.find(" ");
    if (first == string::npos){
        first = line.find("\t");
    }
    else if (line.find("\t")!= string::npos){
        size_t first = min(first, line.find("\t"));
    }
    if (first == string::npos){
        if(line.find(":") != string::npos){
            keyword = line.substr(0, line.length() -1);
            Instruction i = {keyword, variables, arguments};
            instruction_list.push_back(i);
            return;
        }
        else{
            throw invalid_argument("syntax error at: "+line); 
        }
    }
    keyword = line.substr(0, first);
    cout<<keyword<<"end"<<endl;
    string remain = trimmed(line.substr(first));
    vector<string> remains;
    size_t end = remain.find(",");
    while (end != std::string::npos)
    {   
        remains.push_back(trimmed(remain.substr(0, end)));
        cout<<remains[remains.size()-1]<<endl;
        remain = remain.substr(end+1);
        end = remain.find(",");
    }
    remains.push_back(trimmed(remain));
    cout<<remains[remains.size()-1]<<endl;

    if(keyword.compare("add") == 0 || keyword.compare("sub") == 0 || keyword.compare("mul") == 0 || keyword.compare("slt") == 0){
        if (remains.size() != 3){
            cout<<"PARSE ME HE"<<endl;
            throw invalid_argument("syntax error in \"" + keyword + "\" instruction in: "+line);
        }
        if(registers.find(remains[0]) == registers.end() || registers.find(remains[1]) == registers.end() || registers.find(remains[2]) == registers.end() ){
            cout<<"PARSE ME HE"<<endl;
            throw invalid_argument("syntax error in \"" + keyword + "\" instruction in: "+line);
        }
        variables.push_back(remains[0]);
        variables.push_back(remains[1]);
        variables.push_back(remains[2]);
        Instruction i = {keyword, variables, arguments};
        instruction_list.push_back(i);
        return;
    }

    
    else if(keyword == "j"){
        if (remains.size() != 1){
            cout<<"PARSE ME HE"<<endl;
            throw invalid_argument("syntax error in \"" + keyword + "\" instruction in: "+line);
        }
        if(labels.find(remains[0]) == labels.end()){
            cout<<"PARSE ME HE"<<endl;
            throw invalid_argument("syntax error in \"" + keyword + "\" instruction in: "+line);
        }
        variables.push_back(remains[0]);
        Instruction i = {keyword, variables, arguments};
        instruction_list.push_back(i);
        return;
    }
    

    else if(keyword == "beq" || keyword == "bne"){
        if(remains.size() != 3){
            cout<<"PARSE ME HE"<<endl;
            throw invalid_argument("syntax error in \""+keyword+"\" instruction in: "+line);
        }
        if(registers.find(remains[0]) == registers.end() || registers.find(remains[1]) == registers.end() || labels.find(remains[2]) == labels.end()){
            cout<<"PARSE ME HE"<<endl;
            throw invalid_argument("syntax error in \""+keyword+"\" instruction in: "+line);
        }
        variables.push_back(remains[0]);
        variables.push_back(remains[1]);
        variables.push_back(remains[2]);
        Instruction i = {keyword, variables, arguments};
        instruction_list.push_back(i);
        return;
    }
    
     else if(keyword =="lw" || keyword == "sw"){
         
        if(remains.size() != 2){
            cout<<"PARSE ME HE"<<endl;
            throw invalid_argument("syntax error in \""+keyword+"\" instruction in: "+line);
        }
        if(registers.find(remains[0]) == registers.end()){
            cout<<"PARSE ME HE"<<endl;
            throw invalid_argument("syntax error in \""+keyword+"\" instruction in: "+line);
        }
        variables.push_back(remains[0]);
         try{
            arguments.push_back(stoi(remains[1]));
        }
        catch(invalid_argument){
            cout<<"PARSE ME HE"<<endl;
            throw invalid_argument("syntax error in \"" + keyword + "\" instruction in: "+line);
        }
        
        Instruction i = {keyword, variables, arguments};
        instruction_list.push_back(i);
        return;

    }
    else if (keyword == "addi"){
        if (remains.size() != 3){
            cout<<"PARSE ME HE"<<endl;
            throw invalid_argument("syntax error in \"" + keyword + "\" instruction in: "+line);
        }
        if(registers.find(remains[0]) == registers.end() || registers.find(remains[1]) == registers.end()  ){
            cout<<"PARSE ME HE"<<endl;
            throw invalid_argument("syntax error in \"" + keyword + "\" instruction in: "+line);
        }
        variables.push_back(remains[0]);
        variables.push_back(remains[1]);
        try{
            cout<<"PARSE ME HE"<<endl;
            arguments.push_back(stoi(remains[2]));
        }
        catch(invalid_argument){
            cout<<"PARSE ME HE"<<endl;
            throw invalid_argument("syntax error in \"" + keyword + "\" instruction in: "+line);
        }
        Instruction i = {keyword, variables, arguments};
        instruction_list.push_back(i);
        return;
    }
    else{
        cout<<"PARSE ME HE"<<endl;
        throw invalid_argument("syntax error in: "+line);
    }
    return;
}


void ADDI(Instruction I){
    vector<string> vars=I.vars;
    vector<int>args = I.args;
    // cout<<"Addi "<<registers[vars[1]]<<" "<<args[0]<<endl;
    registers[vars[0]]=registers[vars[1]]+args[0];
    
}
void ADD(Instruction I){
    vector<string> vars = I.vars;
    vector<int>args = I.args;
    registers[vars[0]] = registers[vars[1]]+registers[vars[2]];
}
void SUB(Instruction I){
    vector<string> vars = I.vars;
    vector<int>args = I.args;
    registers[vars[0]] = registers[vars[1]]-registers[vars[2]];
}
void MUL(Instruction I){
    vector<string> vars = I.vars;
    vector<int>args = I.args;
    registers[vars[0]] = registers[vars[1]]*registers[vars[2]];
}
void BEQ(Instruction I,ll &PC){
    vector<string> vars = I.vars;
    if(registers[vars[0]] == registers[vars[1]]){
        PC=labels[vars[2]]-1;
    }
    else{
        PC++;
    }
}
void BNE(Instruction I,ll &PC){
    vector<string>vars = I.vars;
    if(registers[vars[0]] == registers[vars[1]]){
        PC++;
    }
    else{
        PC = labels[vars[2]]-1;
    }
}
void SLT(Instruction I){
    vector<string> vars = I.vars;
    registers[vars[0]] = registers[vars[1]] < registers[vars[2]] ? 1 : 0;
}
void JUMP(Instruction I,ll &PC){
    vector<string> vars = I.vars;
    PC = registers[vars[0]]-1;
}
void LW(Instruction I){
    vector<string> vars = I.vars;
    vector<int>args = I.args;
    // cout<<vars.size()<<" "<<args.size()<<endl;
    if(memory.find(args[0])==memory.end()){
        cout<<"HELLO LW ME HE \n";
        throw bad_alloc();
    }
    else{
        registers[vars[0]]=memory[args[0]]+1;
    }
}

void SW(Instruction I){
    vector<string> vars = I.vars;
    vector<int>args = I.args;
    for(int i=-3;i<=3;i++){
        if(i!=0){
            if(memory.find(args[0])!=memory.end()){
                //memory already occupied
                cout<<"HELLO SW ME HE \n";
                throw bad_alloc();
            }
        }
    }
    memory[args[0]]=registers[vars[0]];
}







int main() {
    
     //Intialising all registers to 0;
    registers.insert(make_pair("$zero",0));
    for(int i=0;i<31;i++){
        string s="$t";
        s+=to_string(i);
        registers.insert(make_pair(s,0));
    }

    // read input
    // read input
    ifstream file("test.in");
    if (file.is_open())
    {
        string line, oline;
        int line_num = 0;
        while (getline(file, oline))
        {
            line = trimmed(oline);
            if (line.compare("") != 0){
                if (line.find(":") != string::npos){
                    if (line[line.length() -1] == ':'){
                        if (line.find(" ") != string::npos || line.find("\t") != string::npos){
                            throw invalid_argument("Syntax Error in Line: " + oline);
                        }
                        labels.insert(pair<string,int>(line.substr(0,line.length() - 1),line_num+1));
                        
                        inst.push_back(line);
                        line_num += 1;
                        cout<<line<<endl;
                    }
                    else{
                        size_t ind = line.find(':');
                        string line1 = line.substr(0,ind+1);
                        string line2 = trimmed(line.substr(ind+1));

                        if(line2.find(':')!= string::npos){
                            throw ("Syntax Error in Line: " + oline);
                        }
                        if (line1.find(" ") != string::npos || line1.find("\t") != string::npos){
                            throw invalid_argument("Syntax Error in Line: " + oline);
                        }
                        labels.insert(pair<string,int>(line1.substr(0,line1.length() - 1),line_num+1));
                        inst.push_back(line1);
                        inst.push_back(line2);
                        line_num += 2;
                        cout<<line1<<endl;
                        cout<<line2<<endl;
                    }
                    
                }
                else{
                inst.push_back(line);
                line_num += 1;
                cout << line << endl;}
            }
            // cout << line << endl;
        }
    }
    inst_size=inst.size();

    int i=0;
    //  sub, mul, beq, bne, slt, j, lw, sw
    while(i!=inst_size){
        parse(inst[i]);
        i++;
    }
    inst_size=instruction_list.size();
    memory_check(instruction_list);


    ll PC=0;
    while(PC!=inst_size){
        Instruction temp = instruction_list[PC];
        string operation =temp.kw;

        if (operation=="addi"){
            // cout<<"addi he"<<endl;
            ADDI(temp);
            PC++;
        }
        else if(operation=="add"){
            // cout<<"add bro"<<endl;
            ADD(temp);
            PC++;
        }
        else if(operation=="sub"){
            SUB(temp);
            PC++;
        }
        else if (operation=="mul"){
            MUL(temp);
            PC++;
        }
        else if(operation=="beq"){
            BEQ(temp,PC);

        }
        else if(operation=="bne"){
            BNE(temp,PC);

        }
        else if(operation=="slt"){
            SLT(temp);
            PC++;
        }
        else if(operation=="j"){
            JUMP(temp,PC);

        }
        else if(operation=="lw"){
            LW(temp);
            PC++;

        }
        else if(operation=="sw"){
            SW(temp);
            PC++;
        }
        else{
           PC++;
        }

    }
    cout<<"PROGRAM ENDED \n";
    for (auto& it: registers) {
    // Do stuff
    cout << it.first<<" "<<it.second<<"\n";
}

    
    return 0;
}