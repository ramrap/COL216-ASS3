// c++14
#include <iostream>
#include <fstream>
#include <sstream>
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
ll occupied_mem;
vector<int>t(100000,0);
// t0=0-2000, t1=2000*1-2000*2 , t2= 2000*2 .......   
// vector<int>
vector<pair<string,ll>> inst;
ll inst_size;

unordered_map<string,int32_t> registers;

const string WS = " \t";
unordered_map<string,int> labels;     // 'branch_mname -> line number
struct Instruction
{
    /* data */
    string kw;
    vector<string>vars;
    vector<int>args;
    ll line;
};
int32_t* memory = NULL;
ll first_byte ;
vector<Instruction> instruction_list;
vector<string> operators = {"add", "sub", "mul", "bne", "beq", "slt", "addi", "lw", "sw", "j"};
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
string int32ToHex(int32_t num){
    stringstream sstream;
    sstream<< hex<< num;
    return(sstream.str());
}

string llToHex(long long num){
    stringstream sstream;
    sstream<< hex<< num;
    return(sstream.str());
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

void parse(pair<string,ll> instru){
    string line = instru.first;
    ll num = instru.second;
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
        
        throw runtime_error("Syntax Error at line "+to_string(num)+ ": " + line); 
        
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
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        if(registers.find(remains[0]) == registers.end() || registers.find(remains[1]) == registers.end() || registers.find(remains[2]) == registers.end() ){
            cout<<"PARSE ME HE"<<endl;
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        variables.push_back(remains[0]);
        variables.push_back(remains[1]);
        variables.push_back(remains[2]);
        Instruction i = {keyword, variables, arguments, num};
        instruction_list.push_back(i);
        return;
    }

    
    else if(keyword == "j"){
        if (remains.size() != 1){
            cout<<"PARSE ME HE"<<endl;
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        if(labels.find(remains[0]) == labels.end()){
            cout<<"PARSE ME HE"<<endl;
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        variables.push_back(remains[0]);
        Instruction i = {keyword, variables, arguments, num};
        instruction_list.push_back(i);
        return;
    }
    

    else if(keyword == "beq" || keyword == "bne"){
        if(remains.size() != 3){
            cout<<"PARSE ME HE"<<endl;
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        if(registers.find(remains[0]) == registers.end() || registers.find(remains[1]) == registers.end() || labels.find(remains[2]) == labels.end()){
            cout<<"PARSE ME HE"<<endl;
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        variables.push_back(remains[0]);
        variables.push_back(remains[1]);
        variables.push_back(remains[2]);
        Instruction i = {keyword, variables, arguments, num};
        instruction_list.push_back(i);
        return;
    }
    
     else if(keyword =="lw" || keyword == "sw"){
         
        if(remains.size() != 2){
            cout<<"PARSE ME HE"<<endl;
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        if(registers.find(remains[0]) == registers.end()){
            cout<<"PARSE ME HE"<<endl;
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        variables.push_back(remains[0]);
        if(remains[1].find("(") != string::npos){
            int ind = remains[1].find("(");
            string reg = trimmed(remains[1].substr(ind));
            if(reg[reg.length() -1]==')' && registers.find(reg.substr(1,reg.length()-2)) != registers.end()){
                variables.push_back(reg.substr(1, reg.length()-2));
            }
            else{
                throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
            }
            string offset = trimmed(remains[1].substr(0,ind));
            if(offset==""){
                arguments.push_back(0);
            }
            else{
                try{
                    arguments.push_back(stoi(offset));
                }
                catch(runtime_error){
                    throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);

                }
            }
            Instruction i = {keyword, variables, arguments, num};
            instruction_list.push_back(i);
            return;
            
        }
         try{
            arguments.push_back(stoi(remains[1]));
        }
        catch(runtime_error){
            cout<<"PARSE ME HE"<<endl;
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        
        Instruction i = {keyword, variables, arguments, num};
        instruction_list.push_back(i);
        return;

    }
    else if (keyword == "addi"){
        if (remains.size() != 3){
            cout<<"PARSE ME HE"<<endl;
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        if(registers.find(remains[0]) == registers.end() || registers.find(remains[1]) == registers.end()  ){
            cout<<"PARSE ME HE"<<endl;
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        variables.push_back(remains[0]);
        variables.push_back(remains[1]);
        try{
            cout<<"PARSE ME HE"<<endl;
            arguments.push_back(stoi(remains[2]));
        }
        catch(runtime_error){
            cout<<"PARSE ME HE"<<endl;
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        Instruction i = {keyword, variables, arguments, num};
        instruction_list.push_back(i);
        return;
    }
    else{
        cout<<"PARSE ME HE"<<endl;
        throw runtime_error("syntax error in line "+to_string(num)+ ": " + line);
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
        PC=labels[vars[2]];
        if (PC >= inst_size){
            throw runtime_error("attempt to execute non-instruction at: "+llToHex(PC*4));
        }
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
        PC = labels[vars[2]];
        if (PC >= inst_size){
            throw runtime_error("attempt to execute non-instruction at: "+llToHex(PC*4));
        }
    }
    
}
void SLT(Instruction I){
    vector<string> vars = I.vars;
    registers[vars[0]] = registers[vars[1]] < registers[vars[2]] ? 1 : 0;
}
void JUMP(Instruction I,ll &PC){
    vector<string> vars = I.vars;
    PC = registers[vars[0]];
    if (PC >= inst_size){
        throw runtime_error("attempt to execute non-instruction at: "+llToHex(PC*4));
    }
}
void LW(Instruction I){
    vector<string> vars = I.vars;
    vector<int>args = I.args;
    // cout<<vars.size()<<" "<<args.size()<<endl;
    int addr;
    if (vars.size() == 2){
        string reg =vars[1];
        int offset = args[0];
        addr = registers[reg] + offset;
        
    }
    else{
        addr = args[0];
    }
    if(addr < occupied_mem || addr >= (1<<20)){
        throw runtime_error("address("+to_string(addr)+") is out of range("+to_string(occupied_mem)+" to "+to_string((1<<20)-1) + ") at line "+to_string(I.line));
    }
    if(addr % 4 != 0){
        throw runtime_error("unaligned address in lw: "+to_string(addr)+" at line "+to_string(I.line));
    }
    else{
        try{
            registers[vars[0]]=memory[addr/4];
        }
        catch(exception){
            throw runtime_error("bad address at data read: "+to_string(addr)+" at line: "+to_string(I.line));
        }
    }
}

void SW(Instruction I){
    vector<string> vars = I.vars;
    vector<int>args = I.args;
    int addr;
    if (vars.size() == 2){
        string reg =vars[1];
        int offset = args[0];
        addr = registers[reg] + offset;
        
    }
    else{
        addr = args[0];
    }
    if(addr < occupied_mem || addr >= (1<<20)){
        throw runtime_error("address("+to_string(addr)+") is out of range("+to_string(occupied_mem)+" to "+to_string((1<<20)-1) + ") at line "+to_string(I.line));
    }
    if(addr % 4 != 0){
        throw runtime_error("unaligned address in sw: "+to_string(addr)+" at line "+to_string(I.line));
    }
    memory[addr/4]=registers[vars[0]];
}


int main() {
    
     //Intialising all registers to 0;
    registers.insert(make_pair("$zero",0));
    registers.insert(make_pair("$sp",max_size*4-4));
    for(int i=0;i<30;i++){
        string s="$r";
        s+=to_string(i);
        registers.insert(make_pair(s,0));
    }
    memory = new int32_t[max_size];

    // read input
    // read input
    ifstream file("test.in");
    if (file.is_open())
    {
        string line, oline;
        ll line_num = 0;
        int num = 1;
        while (getline(file, oline))
        {
            line = trimmed(oline);
            if (line.compare("") != 0){
                if (line.find(":") != string::npos){
                    if (line[line.length() -1] == ':'){
                        if (line.find(" ") != string::npos || line.find("\t") != string::npos){
                            throw runtime_error("Syntax Error at line "+to_string(num)+ ": " + oline);
                        }
                        if (find(operators.begin(), operators.end(), line.substr(0, line.length() -1)) != operators.end()){
                            throw runtime_error("Syntax Error at line "+to_string(num)+ ": " + oline);
                        }
                        labels.insert(pair<string,int>(line.substr(0,line.length() - 1),line_num));
                        num+=1;
                        cout<<line<<endl;
                    }
                    else{
                        size_t ind = line.find(':');
                        string line1 = line.substr(0,ind+1);
                        string line2 = trimmed(line.substr(ind+1));

                        if(line2.find(':')!= string::npos){
                            throw ("Syntax Error at line "+to_string(num)+ "in : " + oline);
                        }
                        if (line1.find(" ") != string::npos || line1.find("\t") != string::npos){
                            throw runtime_error("Syntax Error at line "+to_string(num)+ ": " + oline);
                        }
                        if (find(operators.begin(), operators.end(), line.substr(0, line.length() -1)) != operators.end()){
                            throw runtime_error("Syntax Error at line "+to_string(num)+ ": " + oline);
                        }
                        labels.insert(pair<string,int>(line1.substr(0,line1.length() - 1),line_num));
                        inst.push_back(pair<string,ll>(line2,num));
                        line_num += 1;
                        num+=2;
                        cout<<line1<<endl;
                        cout<<line2<<endl;
                    }
                    
                }
                else{
                inst.push_back(pair<string,ll>(line,num));
                line_num += 1;
                num++;
                cout << line << endl;}
            }
            else{
                num++;
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
    if(inst_size>max_size){
        throw runtime_error("memory exceeded : Too many Instructions");
    }
    occupied_mem = inst_size*4;


    ll PC=0;
    while(PC!=inst_size){
        Instruction temp = instruction_list[PC];
        string operation =temp.kw;
        cout<<"PC: "<<llToHex(PC*4)<<endl;
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
        
        for (int i =0; i<30; i++) {   
            string reg = "$r" + to_string(i);    
            cout << reg << " "<< int32ToHex(registers[reg])<<"\n";
        }
        cout << "$sp "<< int32ToHex(registers["$sp"])<<"\n";
        cout << "$zero "<< int32ToHex(0)<<"\n";
    }
    cout<<"PROGRAM ENDED \n";
    

    
    return 0;
}