// c++14
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string> 

#include <unordered_map>
using namespace std;

#define ll long long int

// build with: g++ main.cpp -o main -std=c++14
//./main > test.out
ll max_size=(1<<20)/4;
ll occupied_mem;
ll inst_size;
ll first_byte ;
const string WS = " \t";
struct Instruction
{
    /* data */
    string kw;
    vector<string>vars;
    vector<int>args;
    ll line;
};

int32_t* memory = NULL;

vector<int>t(100000,0);// t0=0-2000, t1=2000*1-2000*2 , t2= 2000*2 .......   
vector<pair<string,ll>> inst;
vector<Instruction> instruction_list;
vector<string> operators = {"add", "sub", "mul", "bne", "beq", "slt", "addi", "lw", "sw", "j"};
vector<string> oinst;
vector<string> reg_name = {"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra" };
vector<int> used_mem;
unordered_map<string,int32_t> registers;
unordered_map<string,int> labels;     // 'branch_mname -> line number




//Function used to trim given string by removing white spaces from it
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

// Convert Int TO Hex
string int32ToHex(int32_t num){
    stringstream sstream;
    sstream<< hex<< num;
    return(sstream.str());
}

// Convert long long to Hex
string llToHex(long long num){
    stringstream sstream;
    sstream<< hex<< num;
    return(sstream.str());
}


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


//Function to parse Current Instruction and store it to Instruction_list
void parse(pair<string,ll> instru){

    string line = instru.first;
    ll num = instru.second;

    //defining variables for Struct Instruction 
    string keyword;
    vector<string> variables;
    vector<int> arguments;

    // find the operator
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
    
    // remaining string
    string remain = trimmed(line.substr(first));

    //vector to store all arguments provided in line 
    vector<string> remains;
    size_t end = remain.find(",");
    while (end != std::string::npos)
    {   
        remains.push_back(trimmed(remain.substr(0, end)));

        remain = remain.substr(end+1);
        end = remain.find(",");
    }
    remains.push_back(trimmed(remain));
    
    //Checking syntax if Instruction is starting with "add","sub","mul","slt" and adding it to instruction_list
    // Throws errors if anything is syntactically incorrect
    if(keyword.compare("add") == 0 || keyword.compare("sub") == 0 || keyword.compare("mul") == 0 || keyword.compare("slt") == 0){
        if (remains.size() != 3){
            
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        if(registers.find(remains[0]) == registers.end() || registers.find(remains[1]) == registers.end() || registers.find(remains[2]) == registers.end() ){
            
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        variables.push_back(remains[0]);
        variables.push_back(remains[1]);
        variables.push_back(remains[2]);
        Instruction i = {keyword, variables, arguments, num};
        instruction_list.push_back(i);
        return;
    }

    //Checking syntax if Instruction is starting with "j" and adding it to instruction_list
    // Throws errors if anything is syntactically incorrect
    else if(keyword == "j"){
        
        if (remains.size() != 1){
            
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        if(labels.find(remains[0]) == labels.end()){
            
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        variables.push_back(remains[0]);
        Instruction i = {keyword, variables, arguments, num};
        instruction_list.push_back(i);
        return;
    }
    
    //Checking syntax if Instruction is starting with "beq","bne" and adding it to instruction_list
    else if(keyword == "beq" || keyword == "bne"){
        if(remains.size() != 3){
            
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        if(registers.find(remains[0]) == registers.end() || registers.find(remains[1]) == registers.end() || labels.find(remains[2]) == labels.end()){
            
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        variables.push_back(remains[0]);
        variables.push_back(remains[1]);
        variables.push_back(remains[2]);
        Instruction i = {keyword, variables, arguments, num};
        instruction_list.push_back(i);
        return;
    }
    
    //Checking syntax if Instruction is starting with "lw","sw" and adding it to instruction_list
    // Throws errors if anything is syntactically incorrect
     else if(keyword =="lw" || keyword == "sw"){
         
        if(remains.size() != 2){
            
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        if(registers.find(remains[0]) == registers.end()){
            
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
                catch(exception){
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
        catch(exception){
            
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        
        Instruction i = {keyword, variables, arguments, num};
        instruction_list.push_back(i);
        return;

    }

    //Checking syntax if Instruction is starting with "addi" and adding it to instruction_list
    // Throws errors if anything is syntactically incorrect
    else if (keyword == "addi"){
        
        if (remains.size() != 3){
            
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        
        if(registers.find(remains[0]) == registers.end() || registers.find(remains[1]) == registers.end()  ){
            
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        variables.push_back(remains[0]);
        variables.push_back(remains[1]);
        try{
            
            arguments.push_back(stoi(remains[2]));
        }
        catch(exception){
            
            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line "+to_string(num)+ ": " + line);
        }
        Instruction i = {keyword, variables, arguments, num};
        instruction_list.push_back(i);
        return;
    }

    //Throw Error when given Instruction doesn't match with any operations mention above
    else{
        
        throw runtime_error("syntax error in line "+to_string(num)+ ": " + line);
    }
    return;
}

//To execute instruction addi
void ADDI(Instruction I){
    vector<string> vars=I.vars;
    vector<int>args = I.args;
    registers[vars[0]]=registers[vars[1]]+args[0]; 
}

//To execute instruction add
void ADD(Instruction I){
    vector<string> vars = I.vars;
    vector<int>args = I.args;
    registers[vars[0]] = registers[vars[1]]+registers[vars[2]];
}

//To execute instruction sub
void SUB(Instruction I){
    vector<string> vars = I.vars;
    vector<int>args = I.args;
    registers[vars[0]] = registers[vars[1]]-registers[vars[2]];
}

// To execute instruction mul
void MUL(Instruction I){
    vector<string> vars = I.vars;
    vector<int>args = I.args;
    registers[vars[0]] = registers[vars[1]]*registers[vars[2]];
}

// To execute instruction beq
void BEQ(Instruction I,ll &PC){
    vector<string> vars = I.vars;
    if(registers[vars[0]] == registers[vars[1]]){
        PC=labels[vars[2]];
        if (PC >= inst_size){
            throw runtime_error("attempt to execute non-instruction at PC: "+llToHex(PC*4));
        }
    }
    else{
        PC++;
    }
}

//To execute instruction bne
void BNE(Instruction I,ll &PC){
    vector<string>vars = I.vars;
    if(registers[vars[0]] == registers[vars[1]]){
        PC++;
    }
    else{
        PC = labels[vars[2]];
        if (PC >= inst_size){
            throw runtime_error("attempt to execute non-instruction at PC: "+llToHex(PC*4));
        }
    }
}

//To execute instruction slt
void SLT(Instruction I){
    vector<string> vars = I.vars;
    registers[vars[0]] = registers[vars[1]] < registers[vars[2]] ? 1 : 0;
}

//To execute instruction jump
void JUMP(Instruction I,ll &PC){
    vector<string> vars = I.vars;
    PC = labels[vars[0]];
    if (PC >= inst_size){
        throw runtime_error("attempt to execute non-instruction at PC: "+llToHex(PC*4));
    }
}

//To execute instruction LW 
void LW(Instruction I){
    
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
        throw runtime_error("unaligned address in lw: "+to_string(addr)+" at line "+to_string(I.line));
    }
    else{
        try{
            registers[vars[0]]=memory[addr/4];
            used_mem.push_back(addr/4);
        }
        catch(exception){
            throw runtime_error("bad address at data read: "+to_string(addr)+" at line: "+to_string(I.line));
        }
    }
}

//To execute instruction SW
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

// execute the instructions one by one and print the outputs 
void execute(){
    ll PC=0;
    ll inst_num=0;int fl=1;
    int num_add =0, num_addi =0, num_j = 0, num_sub =0, num_mul =0, num_bne =0, num_beq =0, num_lw=0, num_sw=0, num_slt =0;
    // Increasing PC until we reach last line
    while(PC!=inst_size){
        Instruction temp = instruction_list[PC];
        string operation =temp.kw;
        cout<<"PC: "<<llToHex(PC*4)<<endl;
        cout<<"Instruction to be executed: "<<oinst[temp.line -1]<<endl;

        // Call respective operations for instructions
        if (operation=="addi"){
            // cout<<"addi he"<<endl;
            ADDI(temp);
            num_addi++;
            PC++;
        }
        else if(operation=="add"){
            // cout<<"add bro"<<endl;
            ADD(temp);
            num_add++;
            PC++;
        }
        else if(operation=="sub"){
            SUB(temp);
            num_sub++;
            PC++;
        }
        else if (operation=="mul"){
            MUL(temp);
            num_mul++;
            PC++;
        }
        else if(operation=="beq"){
            BEQ(temp,PC);
            num_beq++;

        }
        else if(operation=="bne"){
            BNE(temp,PC);
            num_bne++;

        }
        else if(operation=="slt"){
            SLT(temp);
            num_slt++;
            PC++;
        }
        else if(operation=="j"){
            JUMP(temp,PC);
            num_j++;
            

        }
        else if(operation=="lw"){
            LW(temp);
            num_lw++;
            PC++;

        }
        else if(operation=="sw"){
            SW(temp);
            num_sw++;
            PC++;
        }
        registers["$zero"] = 0;
        for (int i =0; i<32; i++) {   
            string reg = reg_name[i];    
            cout << reg << ": "<< int32ToHex(registers[reg])<<"\n";
        }
        cout<<endl;
        
    }
    cout<<"Program Executed Successfully.\n\n";
    cout<<"*****Statistics***** \n";
    cout << "Total no. of clock cycles: "<<to_string(num_add+num_addi+num_beq+num_bne+num_j+num_lw+num_sw+num_sub+num_slt+num_mul)<<endl;
    cout<< "Number of times instruction were executed: \n";
    cout<< "add: "<<to_string(num_add)<<endl;
    cout<< "addi: "<<to_string(num_addi)<<endl;
    cout<< "sub: "<<to_string(num_sub)<<endl;
    cout<< "mul: "<<to_string(num_mul)<<endl;
    cout<< "bne: "<<to_string(num_bne)<<endl;
    cout<< "beq: "<<to_string(num_beq)<<endl;
    cout<< "j: "<<to_string(num_j)<<endl;
    cout<< "slt: "<<to_string(num_slt)<<endl;
    cout<< "lw: "<<to_string(num_lw)<<endl;
    cout<< "sw: "<<to_string(num_sw)<<endl;
    cout<<endl;
    cout<<"Used Data Values During Execution"<<endl;
    sort(used_mem.begin(), used_mem.end());
    for(int i = 0; i < used_mem.size(); i++){
        if(i != 0){
            if(used_mem[i] != used_mem[i-1]){
                cout<<used_mem[i]*4<<"-"<<used_mem[i]*4 + 3<<": "<<int32ToHex(memory[used_mem[i]])<<endl;
            }
        }
        else{
            cout<<used_mem[i]*4<<"-"<<used_mem[i]*4 + 3<<": "<<int32ToHex(memory[used_mem[i]])<<endl;
        }
    }
}

// initialize registers and memory array.
void initialise_memory(){
    //Intialising all registers to 0;
    registers.insert(make_pair("$zero",0));
    registers.insert(make_pair("$sp",max_size*4-4));
    registers.insert(make_pair("$gp",0));
    registers.insert(make_pair("$fp",0));
    registers.insert(make_pair("$at",0));
    registers.insert(make_pair("$ra",0));
    
    for(int i=0;i<=9;i++){
        string s="$t";
        s+=to_string(i);
        registers.insert(make_pair(s,0));
        if (i <= 7){
            s = "$s";
            s+= to_string(i);
            registers.insert(make_pair(s,0));
        }
    }

    for(int i=0;i<4;i++){
        string s="$a";
        s+=to_string(i);
        registers.insert(make_pair(s,0));
        if (i <= 1){
            s = "$v";
            s+= to_string(i);
            registers.insert(make_pair(s,0));
            s = "$k";
            s+= to_string(i);
            registers.insert(make_pair(s,0));
        }
    }
    

    //allocating size to memory Array
    memory = new int32_t[max_size];

}

// main function
int main(int argc, char *argv[]) {
    
    
    initialise_memory();
    
    // read file name from CLI
    if(argc==0){
        cout<<"Please provide input file path"<<endl;
        return 0;
    }
    ifstream file(argv[1]);

    //If file is failed to open or file_path is wrong in argument then throw error
    if(!file.is_open()){
        cout<<"Error: Provide Valid File Path"<<endl;
        return 0;
    }
    
    if (file.is_open())
    {
        string line, oline;
        ll line_num = 0;
        int num = 1;
        
        
        //code for reading from input file and converting each line into parsable strings
        while (getline(file, oline))
        {
            // cout<<line<<endl;
            line = trimmed(oline);
            oinst.push_back(line);
            if (line.compare("") != 0){
                if (line.find(":") != string::npos){
                    if (line[line.length() -1] == ':'){
                        if (line.find(" ") != string::npos || line.find("\t") != string::npos){
                            throw runtime_error("Syntax Error at line "+to_string(num)+ ": " + oline);
                        }
                        if (find(operators.begin(), operators.end(), line.substr(0, line.length() -1)) != operators.end()){
                            throw runtime_error("An operator cannot be a label. Syntax Error at line "+to_string(num)+ ": " + oline);
                        }
                        if(labels.find(line.substr(0, line.length()-1)) != labels.end()){
                            throw runtime_error("Label is defined for the second time on line " +to_string(num));
                        }
                        labels.insert(pair<string,int>(line.substr(0,line.length() - 1),line_num));
                        num+=1;
                    }
                    else{
                        size_t ind = line.find(':');
                        string line1 = line.substr(0,ind+1);
                        string line2 = trimmed(line.substr(ind+1));

                        if(line2.find(':')!= string::npos){
                            throw runtime_error("Syntax Error at line " + to_string(num) + " in : " + oline);
                        }
                        if (line1.find(" ") != string::npos || line1.find("\t") != string::npos){
                            throw runtime_error("Syntax Error at line "+to_string(num)+ ": " + oline);
                        }
                        if (find(operators.begin(), operators.end(), line1.substr(0, line1.length() -1)) != operators.end()){
                            throw runtime_error("An operator cannot be a label. Syntax Error at line "+to_string(num)+ ": " + oline);
                        }
                        if(labels.find(line1.substr(0,line1.length()-1)) != labels.end()){
                            throw runtime_error("Label is defined for the second time on line " +to_string(num));
                        }
                        labels.insert(pair<string,int>(line1.substr(0,line1.length() - 1),line_num));
                        inst.push_back(pair<string,ll>(line2,num));
                        line_num += 1;
                        num+=1;
                    }
                    
                }
                else{
                inst.push_back(pair<string,ll>(line,num));
                line_num += 1;
                num++;
                
                }
            }
            else{
                num++;
            }   
        }
    }
    inst_size=inst.size();

    int i=0;
    while(i!=inst_size){
        //parsing instruction
        parse(inst[i]);
        i++;
    }
    
    inst_size=instruction_list.size();

    //if instructionList size exceeds allotted memory then throw error
    if(inst_size>max_size){
        throw runtime_error("memory exceeded : Too many Instructions");
    }
    occupied_mem = inst_size*4;

    //function to execute InstructionList obtained after parsing
    execute();    

    return 0;
}