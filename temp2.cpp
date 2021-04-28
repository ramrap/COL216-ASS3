// c++14
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>
#include "helper.hpp"
#include "a4.hpp"

#include <unordered_map>
using namespace std;

#define ll long long int

// build with: g++ main.cpp -o main -std=c++14
//./main > test.out
ll max_size = (1 << 20) / 4;
int buffer_updates=0;
bool in_buffer = false;
bool no_blocking = true;
ll occupied_mem = 0;
ll inst_size[100];

ll num_of_cores;
ll simulation_time;

int32_t **memory = NULL;

vector<vector<pair<string, ll>>> inst(100);
vector<vector<Instruction>> instruction_list(100);
vector<vector<string>> oinst(100);
int32_t *buffer_row = NULL;
int buffered = -1;
vector<vector<int>> used_mem;
vector<vector<int>> changed_mem;
vector<string> changed_regs;
unordered_map<string, int32_t> registers;
vector<vector<int32_t>> registers_core(100, vector<int32_t>(32, 0));
vector<unordered_map<string, int>> labels(100); // 'branch_mname -> line number

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
void parse(pair<string, ll> instru, int core_num)
{
    string line = instru.first;
    ll num = instru.second;

    //defining variables for Struct Instruction
    string keyword;
    vector<string> variables;
    vector<int> arguments;

    // find the operator
    size_t first = line.find(" ");
    if (first == string::npos)
    {
        first = line.find("\t");
    }
    else if (line.find("\t") != string::npos)
    {
        size_t first = min(first, line.find("\t"));
    }
    if (first == string::npos)
    {

        throw runtime_error("Syntax Error at line " + to_string(num) + ": " + line);
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

        remain = remain.substr(end + 1);
        end = remain.find(",");
    }
    remains.push_back(trimmed(remain));

    //Checking syntax if Instruction is starting with "add","sub","mul","slt" and adding it to instruction_list
    // Throws errors if anything is syntactically incorrect
    if (keyword.compare("add") == 0 || keyword.compare("sub") == 0 || keyword.compare("mul") == 0 || keyword.compare("slt") == 0)
    {
        if (remains.size() != 3)
        {

            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line " + to_string(num) + ": " + line);
        }
        if (registers.find(remains[0]) == registers.end() || registers.find(remains[1]) == registers.end() || registers.find(remains[2]) == registers.end())
        {

            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line " + to_string(num) + ": " + line);
        }
        variables.push_back(remains[0]);
        variables.push_back(remains[1]);
        variables.push_back(remains[2]);
        Instruction i = {keyword, variables, arguments, num};
        instruction_list[core_num].push_back(i);
        return;
    }

    //Checking syntax if Instruction is starting with "j" and adding it to instruction_list
    // Throws errors if anything is syntactically incorrect
    else if (keyword == "j")
    {

        if (remains.size() != 1)
        {

            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line " + to_string(num) + ": " + line);
        }
        if (labels[core_num].find(remains[0]) == labels[core_num].end())
        {

            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line " + to_string(num) + ": " + line);
        }
        variables.push_back(remains[0]);
        Instruction i = {keyword, variables, arguments, num};
        instruction_list[core_num].push_back(i);
        return;
    }

    //Checking syntax if Instruction is starting with "beq","bne" and adding it to instruction_list
    else if (keyword == "beq" || keyword == "bne")
    {
        if (remains.size() != 3)
        {

            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line " + to_string(num) + ": " + line);
        }
        if (registers.find(remains[0]) == registers.end() || registers.find(remains[1]) == registers.end() || labels[core_num].find(remains[2]) == labels[core_num].end())
        {

            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line " + to_string(num) + ": " + line);
        }
        variables.push_back(remains[0]);
        variables.push_back(remains[1]);
        variables.push_back(remains[2]);
        Instruction i = {keyword, variables, arguments, num};
        instruction_list[core_num].push_back(i);
        return;
    }

    //Checking syntax if Instruction is starting with "lw","sw" and adding it to instruction_list
    // Throws errors if anything is syntactically incorrect
    else if (keyword == "lw" || keyword == "sw")
    {

        if (remains.size() != 2)
        {

            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line " + to_string(num) + ": " + line);
        }
        if (registers.find(remains[0]) == registers.end())
        {

            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line " + to_string(num) + ": " + line);
        }
        variables.push_back(remains[0]);
        if (remains[1].find("(") != string::npos)
        {
            int ind = remains[1].find("(");
            string reg = trimmed(remains[1].substr(ind));
            if (reg[reg.length() - 1] == ')' && registers.find(reg.substr(1, reg.length() - 2)) != registers.end())
            {
                variables.push_back(reg.substr(1, reg.length() - 2));
            }
            else
            {
                throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line " + to_string(num) + ": " + line);
            }
            string offset = trimmed(remains[1].substr(0, ind));
            if (offset == "")
            {
                arguments.push_back(0);
            }
            else
            {
                try
                {
                    arguments.push_back(stoi(offset));
                }
                catch (exception)
                {
                    throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line " + to_string(num) + ": " + line);
                }
            }
            Instruction i = {keyword, variables, arguments, num};
            instruction_list[core_num].push_back(i);
            return;
        }
        else
        {

            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line " + to_string(num) + ": " + line);
        }
    }

    //Checking syntax if Instruction is starting with "addi" and adding it to instruction_list
    // Throws errors if anything is syntactically incorrect
    else if (keyword == "addi")
    {

        if (remains.size() != 3)
        {

            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line " + to_string(num) + ": " + line);
        }

        if (registers.find(remains[0]) == registers.end() || registers.find(remains[1]) == registers.end())
        {

            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line " + to_string(num) + ": " + line);
        }
        variables.push_back(remains[0]);
        variables.push_back(remains[1]);
        try
        {

            arguments.push_back(stoi(remains[2]));
        }
        catch (exception)
        {

            throw runtime_error("Syntax Error in \"" + keyword + "\" instruction at line " + to_string(num) + ": " + line);
        }
        Instruction i = {keyword, variables, arguments, num};
        instruction_list[core_num].push_back(i);
        return;
    }

    //Throw Error when given Instruction doesn't match with any operations mention above
    else
    {

        throw runtime_error("syntax error in line " + to_string(num) + ": " + line);
    }
    return;
}

// //To execute instruction addi
void ADDI(Instruction I,int core_num)
{
    vector<string> vars = I.vars;
    vector<int> args = I.args;
    registers_core[core_num][registers[vars[0]]] = registers_core[core_num][registers[vars[1]]] + args[0];
    changed_regs.push_back(vars[0]);
}

// //To execute instruction add
void ADD(Instruction I,int core_num)
{
    vector<string> vars = I.vars;
    vector<int> args = I.args;
     registers_core[core_num][registers[vars[0]]] =  registers_core[core_num][registers[vars[1]]] +  registers_core[core_num][registers[vars[2]]];
    changed_regs.push_back(vars[0]);
}

//To execute instruction sub
void SUB(Instruction I,int core_num)
{
    vector<string> vars = I.vars;
    vector<int> args = I.args;
     registers_core[core_num][registers[vars[0]]] =  registers_core[core_num][registers[vars[1]]] -  registers_core[core_num][registers[vars[2]]];
    changed_regs.push_back(vars[0]);
}

// To execute instruction mul
void MUL(Instruction I,int core_num)
{
    vector<string> vars = I.vars;
    vector<int> args = I.args;
     registers_core[core_num][registers[vars[0]]] =  registers_core[core_num][registers[vars[1]]] *  registers_core[core_num][registers[vars[2]]];
    changed_regs.push_back(vars[0]);
}

// To execute instruction beq
void BEQ(Instruction I, int &PC, int core_num)
{
    vector<string> vars = I.vars;
    if ( registers_core[core_num][registers[vars[0]]] ==  registers_core[core_num][registers[vars[1]]])
    {
        PC = labels[core_num][vars[2]];
        if (PC >= inst_size[core_num])
        {
            throw runtime_error("attempt to execute non-instruction at PC: " + to_string(PC * 4));
        }
    }
    else
    {
        PC++;
    }
}

//To execute instruction bne
void BNE(Instruction I, int &PC, int core_num)
{
    vector<string> vars = I.vars;
    if ( registers_core[core_num][registers[vars[0]]] ==  registers_core[core_num][registers[vars[1]]])
    {
        PC++;
    }
    else
    {
        PC = labels[core_num][vars[2]];
        if (PC >= inst_size[core_num])
        {
            throw runtime_error("attempt to execute non-instruction at PC: " + to_string(PC * 4));
        }
    }
}

//To execute instruction slt
void SLT(Instruction I,int  core_num)
{
    vector<string> vars = I.vars;
     registers_core[core_num][registers[vars[0]]] =  registers_core[core_num][registers[vars[1]]] <  registers_core[core_num][registers[vars[2]]] ? 1 : 0;
    changed_regs.push_back(vars[0]);
}

//To execute instruction jump
void JUMP(Instruction I, int &PC, int core_num)
{
    vector<string> vars = I.vars;
    PC = labels[core_num][vars[0]];
    if (PC >= inst_size[core_num])
    {
        throw runtime_error("attempt to execute non-instruction at PC: " + to_string(PC * 4));
    }
}

//To execute instruction LW
void LW(Instruction I, int cycles, int core_num)
{
    vector<string> vars = I.vars;
    vector<int> args = I.args;
    int addr;
    string reg = vars[1];
    int offset = args[0];
    addr =  registers_core[core_num][registers[reg]] + offset;

    if (addr < occupied_mem || addr >= (1 << 20))
    {
        throw runtime_error("address(" + to_string(addr) + ") is out of range(" + to_string(occupied_mem) + " to " + to_string((1 << 20) - 1) + ") at line " + to_string(I.line));
    }
    if (addr % 4 != 0)
    {
        throw runtime_error("unaligned address in lw: " + to_string(addr) + " at line " + to_string(I.line));
    }

    // --------------------------------------CHANGED----------------------------------------
    in_buffer = true;
    add_req(I, addr, buffered, cycles, core_num);

    // --------------------------------------CHANGED----------------------------------------
}

//To execute instruction SW
void SW(Instruction I, int cycles, int core_num)
{
    vector<string> vars = I.vars;
    vector<int> args = I.args;
    int addr;
    string reg = vars[1];
    int offset = args[0];
    addr =  registers_core[core_num][registers[reg]] + offset;

    if (addr < occupied_mem || addr >= (1 << 20))
    {
        throw runtime_error("address(" + to_string(addr) + ") is out of range(" + to_string(occupied_mem) + " to " + to_string((1 << 20) - 1) + ") at line " + to_string(I.line));
    }
    if (addr % 4 != 0)
    {
        throw runtime_error("unaligned address in sw: " + to_string(addr) + " at line " + to_string(I.line));
    }

    // --------------------------------------CHANGED----------------------------------------
    in_buffer = true;
    add_req(I, addr, buffered, cycles, core_num, registers[vars[0]]);
    // --------------------------------------CHANGED----------------------------------------

    // cout<<row_access_end<<" "<<column_access_end<<endl;
}

// execute the instructions one by one and print the outputs
void execute()
{
    // ll PC = 0;
    ll inst_num = 0;
    int fl = 1, last = -1;
    vector<int> num_add(100, 0), num_addi(100, 0), num_j(100, 0), num_sub(100, 0), num_mul(100, 0), num_bne(100, 0), num_beq(100, 0), num_lw(100, 0), num_sw(100, 0), num_slt(100, 0);

    int cycles=1;
    vector<int> PC(100, 0);

    bool to_print = false;

    int curr_req_end = -1;
    Instruction temp;
    int temp_pc;

    int flag = 1;
    while (cycles<=simulation_time)
    {

        
        for (int core_num = 0; core_num < num_of_cores; core_num++)
        {

            if (PC[core_num] != inst_size[core_num] && cycles<=simulation_time)
            {
                temp = instruction_list[core_num][PC[core_num]];
                temp_pc = PC[core_num];
                string operation = temp.kw;
                // cout<<"Instruction to be executed: "<<oinst[temp.line -1]<<endl;

                // Call respective operations for instructions
                if (operation == "addi")
                {
                    if (in_buffer && no_blocking)
                    {
                        if (is_safe(temp, core_num))
                        {
                            ADDI(temp,core_num);
                            num_addi[core_num]++;
                            PC[core_num]++;
                            to_print = true;
                        }
                    }
                    else if (!in_buffer)
                    {
                        ADDI(temp,core_num);
                        num_addi[core_num]++;
                        PC[core_num]++;
                        to_print = true;
                    }
                }
                else if (operation == "add")
                {
                    if (in_buffer && no_blocking)
                    {
                        if (is_safe(temp, core_num))
                        {
                            ADD(temp,core_num);
                            num_add[core_num]++;
                            PC[core_num]++;
                            to_print = true;
                        }
                    }
                    else if (!in_buffer)
                    {
                        ADD(temp,core_num);
                        num_add[core_num]++;
                        PC[core_num]++;
                        to_print = true;
                    }
                }
                else if (operation == "sub")
                {
                    if (in_buffer && no_blocking)
                    {
                        if (is_safe(temp, core_num))
                        {
                            SUB(temp,core_num);
                            num_sub[core_num]++;
                            PC[core_num]++;
                            to_print = true;
                        }
                    }
                    else if (!in_buffer)
                    {
                        SUB(temp,core_num);
                        num_sub[core_num]++;
                        PC[core_num]++;
                        to_print = true;
                    }
                }
                else if (operation == "mul")
                {
                    if (in_buffer && no_blocking)
                    {
                        if (is_safe(temp, core_num))
                        {
                            MUL(temp,core_num);
                            num_mul[core_num]++;
                            PC[core_num]++;
                            to_print = true;
                        }
                    }
                    else if (!in_buffer)
                    {
                        MUL(temp,core_num);
                        num_mul[core_num]++;
                        PC[core_num]++;
                        to_print = true;
                    }
                }
                else if (operation == "beq")
                {
                    if (in_buffer && no_blocking)
                    {
                        if (is_safe(temp, core_num))
                        {
                            BEQ(temp, PC[core_num], core_num);
                            num_beq[core_num]++;
                            to_print = true;
                        }
                    }
                    else if (!in_buffer)
                    {
                        BEQ(temp, PC[core_num], core_num);
                        num_beq[core_num]++;
                        to_print = true;
                    }
                }
                else if (operation == "bne")
                {
                    if (in_buffer && no_blocking)
                    {
                        if (is_safe(temp, core_num))
                        {
                            BNE(temp, PC[core_num], core_num);
                            num_beq[core_num]++;
                            to_print = true;
                        }
                    }
                    else if (!in_buffer)
                    {
                        BNE(temp, PC[core_num], core_num);
                        num_bne[core_num]++;
                        to_print = true;
                    }
                }
                else if (operation == "slt")
                {
                    if (in_buffer && no_blocking)
                    {
                        if (is_safe(temp, core_num))
                        {
                            SLT(temp,core_num);
                            num_slt[core_num]++;
                            PC[core_num]++;
                            to_print = true;
                        }
                    }
                    else if (!in_buffer)
                    {
                        SLT(temp,core_num);
                        num_slt[core_num]++;
                        PC[core_num]++;
                        to_print = true;
                    }
                }
                else if (operation == "j")
                {
                    JUMP(temp, PC[core_num], core_num);
                    num_j[core_num]++;
                    to_print = true;
                }
                else if (operation == "lw")
                {
                    if (in_buffer && no_blocking)
                    {
                        if (is_lw_safe(temp, core_num))
                        {
                            SW(temp, cycles, core_num);
                            num_sw[core_num]++;
                            PC[core_num]++;
                            to_print = true;
                        }
                    }
                    else if (!in_buffer)
                    {
                        SW(temp, cycles, core_num);
                        num_sw[core_num]++;
                        PC[core_num]++;
                        to_print = true;
                    }
                }
                else if (operation == "sw")
                {
                    if (in_buffer && no_blocking)
                    {
                        if (is_safe(temp, core_num))
                        {
                            SW(temp, cycles, core_num);
                            num_sw[core_num]++;
                            PC[core_num]++;
                            to_print = true;
                        }
                    }
                    else if (!in_buffer)
                    {
                        SW(temp, cycles, core_num);
                        num_sw[core_num]++;
                        PC[core_num]++;
                        to_print = true;
                    }
                }
                if (to_print)
                {
                    cout << "cycle " << cycles << " in Core : " << core_num + 1 << endl;
                    last = cycles;
                    cout << "Instruction executed (PC = " << temp_pc * 4 << "): " << oinst[core_num][temp.line - 1] << endl;
                    if (temp.kw == "sw" || temp.kw == "lw")
                    {
                        if (request_queue.size() == 1)
                        {
                            cout << "DRAM request issued.(for memory address " << (request_queue[0].access_row * columns + request_queue[0].access_column) * 4 << ")" << endl;
                            if (row_access_end == -1)
                            {
                                cout << "As row " << request_queue[0].access_row << " is already present in buffer, row activation is not required." << endl;
                            }
                            request_queue[0].issue_msg = true;
                        }
                        else
                        {
                            cout << "Request added to queue." << endl;
                        }
                    }
                }
            }


        }

     //------------------------------------------------------------------------------------------------------------------------------------------------
            //---------------------------------------------------------------------C H A N G E D--------------------------------------------------------------
            if (in_buffer)
            {
                bool DRAM_request = false, put_back_done = false, row_access_done = false, column_access_done = false, print_in_buffer = false;
                d_request curr_req = request_queue[0];
                if (cycles == request_issue_cycle && (!curr_req.issue_msg))
                {
                    DRAM_request = true;
                    request_queue[0].issue_msg = true;
                    print_in_buffer = true;
                }
                if (cycles == put_back_end)
                {
                    put_back_done = true;
                    print_in_buffer = true;
                }
                if (cycles == row_access_end)
                {
                    buffer_row = memory[curr_req.access_row];
                    buffered = curr_req.access_row;
                    buffer_updates++;
                    row_access_done = true;
                    print_in_buffer = true;
                }
                if (cycles == column_access_end)
                {
                    if (curr_req.op == 0)
                    {
                        memory[curr_req.access_row][curr_req.access_column] = curr_req.data_bus;
                        vector<int> temp = {curr_req.access_row, curr_req.access_column};
                        changed_mem.push_back(temp);
                        used_mem.push_back(temp);
                        buffer_updates++;
                    }
                    else
                    {
                        registers[curr_req.waiting_reg] = memory[curr_req.access_row][curr_req.access_column];
                        changed_regs.push_back(curr_req.waiting_reg);
                    }
                    column_access_done = true;
                    print_in_buffer = true;
                }
                if (!to_print && print_in_buffer)
                {
                    to_print = true;
                    if (last == cycles - 1)
                    {
                        cout << "cycle " << cycles << ":" << endl;
                        last = cycles;
                    }
                    else
                    {
                        cout << "cycle " << last + 1 << "-" << cycles << ":" << endl;
                        last = cycles;
                    }
                }
                if (DRAM_request)
                {
                    cout << "DRAM request issued.(for memory address " << (curr_req.access_row * columns + curr_req.access_column) * 4 << ")" << endl;
                    if (row_access_end == -1)
                    {
                        cout << "As row " << request_queue[0].access_row << " is already present in buffer, row activation is not required." << endl;
                    }
                    request_queue[0].issue_msg = true;
                }
                if (put_back_done)
                {
                    cout << "DRAM: Wroteback row " << put_back_row << ".";
                    if (put_back_start < put_back_end)
                    {
                        cout << "(In cycles " << put_back_start << "-" << put_back_end << ")" << endl;
                    }
                    else if (put_back_end == put_back_start)
                    {
                        cout << "(In cycle " << put_back_start << ")" << endl;
                    }
                    else
                    {
                        cout << endl;
                    }
                }
                if (row_access_done)
                {
                    cout << "DRAM: Row " << curr_req.access_row ;
                    if (row_access_start < row_access_end)
                    {
                        cout << "(In cycles " << row_access_start << "-" << row_access_end << ")" << endl;
                    }
                    else if (row_access_end == row_access_start)
                    {
                        cout << "(In cycle " << row_access_start << ")" << endl;
                    }
                    else
                    {
                        cout << endl;
                    }
                }
                if (column_access_done)
                {
                    cout << "DRAM: Column " << curr_req.access_column;
                    if (column_access_start < column_access_end)
                    {
                        cout << "(In cycles " << column_access_start << "-" << column_access_end << ")" << endl;
                    }
                    else if (column_access_end == column_access_start)
                    {
                        cout << "(In cycle " << column_access_start << ")" << endl;
                    }
                    else
                    {
                        cout << endl;
                    }
                    request_queue.erase(request_queue.begin());
                    req_regs.erase(req_regs.begin());
                    if (request_queue.size() == 0)
                    {
                        in_buffer = false;
                    }
                    // else if removed from here
                    if (request_queue.size() != 0)
                    {
                        assign_cycle_values(buffered, cycles);
                    }
                }
            }
            //-----------------------------------------------------------------------------------------------------------------------------------------------------
            //-----------------------------------------------------------------------------------------------------------------------------------------------------
            for(int i=0;i<num_of_cores;i++){
            registers_core[i][registers["$zero"]] = 0;
            }
            if (changed_regs.size() != 0)
            {
                sort(changed_regs.begin(), changed_regs.end());
                if (changed_regs[0] != "$zero")
                {
                    cout << "Updated Registers:  ";
                    cout << changed_regs[0] << " = " << registers[changed_regs[0]];
                    for (int i = 1; i < changed_regs.size(); i++)
                    {
                        if (changed_regs[i] != changed_regs[i - 1] && changed_regs[i] != "$zero")
                        {
                            cout << "    " << changed_regs[i] << " = " << registers[changed_regs[i]];
                        }
                    }
                    cout << endl;
                }
            }

            if (changed_mem.size() != 0)
            {
                if (changed_mem.size() != 1)
                {
                    cout << "error" << endl;
                }
                sort(changed_mem.begin(), changed_mem.end());
                cout << "Memory Address:  ";
                cout << (changed_mem[0][0] * columns + changed_mem[0][1]) * 4 << "-" << (changed_mem[0][0] * columns + changed_mem[0][1]) * 4 + 3 << " = " << memory[changed_mem[0][0]][changed_mem[0][1]] << endl;
            }
            if (to_print)
            {
                cout << endl;
            }
            to_print = false;
            changed_regs.clear();
            changed_mem.clear();
            cycles++;
        }
    
       
    
    
    // Increasing PC until we reach last line
     if (buffered != -1)
        {
            if (row_delay == 0)
            {
                cout << "cycle " << cycles - 1 <<endl;
                cout << "DRAM: Wroteback row " << buffered << "." << endl;
            }
            else if (row_delay == 1)
            {
                cout << "cycle " << cycles << endl;
                cout << "DRAM: Wroteback row " << buffered << ".(In cycle " << cycles << ")" << endl;
            }
            else
            {
                cout << "cycle " << cycles << "-" << cycles + row_delay - 1 << endl;
                cout << "DRAM: Wroteback row " << buffered << ".(In cycles " << cycles << "-" << cycles + row_delay - 1 << ")" << endl;
            }
            cycles = cycles + row_delay;
            cout << endl;
        }


    for (int core_num = 0; core_num < num_of_cores; core_num++)
    {

       
        cout << endl
             << "Program Executed Successfully for Core Number : " << core_num + 1 << ".\n\n";
        cout << "*****Statistics***** \n";
        // cout << "Total no. of clock cycles: " << cycles - 1 << endl;
        cout << "Total number of buffer updates: " << buffer_updates << endl;
        cout << "Number of times instruction were executed: \n";
        cout << "Total number of Instructions executed: " << num_add[core_num] + num_addi[core_num] + num_sub[core_num] + num_mul[core_num] + num_bne[core_num] + num_beq[core_num] + num_j[core_num] + num_slt[core_num] + num_lw[core_num] + num_sw[core_num] << endl;
        cout << "add: " << to_string(num_add[core_num]) << endl;
        cout << "addi: " << to_string(num_addi[core_num]) << endl;
        cout << "sub: " << to_string(num_sub[core_num]) << endl;
        cout << "mul: " << to_string(num_mul[core_num]) << endl;
        cout << "bne: " << to_string(num_bne[core_num]) << endl;
        cout << "beq: " << to_string(num_beq[core_num]) << endl;
        cout << "j: " << to_string(num_j[core_num]) << endl;
        cout << "slt: " << to_string(num_slt[core_num]) << endl;
        cout << "lw: " << to_string(num_lw[core_num]) << endl;
        cout << "sw: " << to_string(num_sw[core_num]) << endl;
        cout << endl;

        cout << "Register Values After Execution" << endl;
        for (int i = 0; i < 32; i++)
        {
            string reg = reg_name[i];
            cout <<registers_core[core_num][registers[reg]] << " ";
        }
        cout << endl
             << endl;
    }

    if (used_mem.size() != 0)
    {
        cout << "Used Data Values During Execution" << endl;
        sort(used_mem.begin(), used_mem.end(), compare_address);
        for (int i = 0; i < used_mem.size(); i++)
        {
            if (i != 0)
            {
                if (used_mem[i] != used_mem[i - 1])
                {
                    cout << (used_mem[i][0] * columns + used_mem[i][1]) * 4 << "-" << (used_mem[i][0] * columns + used_mem[i][1]) * 4 + 3 << ": " << (memory[used_mem[i][0]][used_mem[i][1]]) << endl;
                }
            }
            else
            {
                cout << (used_mem[i][0] * columns + used_mem[i][1]) * 4 << "-" << (used_mem[i][0] * columns + used_mem[i][1]) * 4 + 3 << ": " << (memory[used_mem[i][0]][used_mem[i][1]]) << endl;
            }
        }
    }
}

// initialize registers and memory array.
void initialise_memory()
{
    //Intialising all registers to 0;
    int j = 0;
    registers.insert(make_pair("$zero", j++));
    registers.insert(make_pair("$sp", j++));
    registers.insert(make_pair("$gp", j++));
    registers.insert(make_pair("$fp", j++));
    registers.insert(make_pair("$at", j++));
    registers.insert(make_pair("$ra", j++));

    for (int i = 0; i <= 9; i++)
    {
        string s = "$t";
        s += to_string(i);
        registers.insert(make_pair(s, j++));
        if (i <= 7)
        {
            s = "$s";
            s += to_string(i);
            registers.insert(make_pair(s, j++));
        }
    }

    for (int i = 0; i < 4; i++)
    {
        string s = "$a";
        s += to_string(i);
        registers.insert(make_pair(s, j++));
        if (i <= 1)
        {
            s = "$v";
            s += to_string(i);
            registers.insert(make_pair(s, j++));
            s = "$k";
            s += to_string(i);
            registers.insert(make_pair(s, j++));
        }
    }

    for (int i = 0; i < 100; i++)
    {
        registers_core[i][registers["$sp"]] = max_size * 4 - 4;
        // cout<<registers_core[i][registers["$sp"]]<<endl;
    }

    //allocating size to memory Array
    buffer_row = new int32_t[columns];
    memory = new int32_t *[rows];
    for (int i = 0; i < rows; i++)
    {
        memory[i] = new int32_t[columns];
    }
}

// main function
int main(int argc, char *argv[])
{

    initialise_memory();

    // read file name from CLI
    if (argc == 1)
    {
        cout << "Invalid argument: Please provide input file path" << endl;
        return 0;
    }

    // if(argc == 3){
    //     string argv2(argv[2]);
    //     if(argv2.compare("p1") == 0){
    //         no_blocking = false;
    //     }
    //     else if(argv2.compare("p2") == 0){
    //         no_blocking = true;
    //     }
    //     else{
    //         cout<<"Invalid argument: "<<argv2<<" Please provide valid arguments."<<endl;
    //         return 0;
    //     }

    // }
    // if (argc >=4){
    //     row_delay = stoi(argv[2]);
    //     column_delay = stoi(argv[3]);
    //     if(row_delay <0 || column_delay<0){
    //         cout<<"Invalid argument: Row and Column access delay cannot be negative."<<endl;
    //         return 0;
    //     }
    //     if (argc == 5){
    //         string argv4(argv[4]);
    //         if(argv4 == "p1"){
    //             no_blocking = false;
    //         }
    //         else if(argv4 == "p2"){
    //             no_blocking = true;
    //         }
    //         else{
    //             cout<<"Invalid argument: "<<argv4<<" Please provide valid arguments."<<endl;
    //             return 0;
    //         }
    //     }
    // }
try{
    num_of_cores = stoi(argv[1]);
    simulation_time = stoi(argv[2]);
    for (int read = 0; read < num_of_cores; read++)
    {
       
        ifstream file(argv[read + 3]);
        if (!file.is_open())
        {
            cout << "Error: Provide Valid File Path For " << argv[read + 3] << endl;
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
              
                line = trimmed(oline);
                oinst[read].push_back(line);
                if (line.compare("") != 0)
                {
                    if (line.find(":") != string::npos)
                    {
                        if (line[line.length() - 1] == ':')
                        {
                            if (line.find(" ") != string::npos || line.find("\t") != string::npos)
                            {
                                throw runtime_error("Syntax Error at line " + to_string(num) + ": " + oline);
                            }
                            if (find(operators.begin(), operators.end(), line.substr(0, line.length() - 1)) != operators.end())
                            {
                                throw runtime_error("An operator cannot be a label. Syntax Error at line " + to_string(num) + ": " + oline);
                            }
                            if (labels[read].find(line.substr(0, line.length() - 1)) != labels[read].end())
                            {
                                throw runtime_error("Label is defined for the second time on line " + to_string(num));
                            }
                            labels[read].insert(pair<string, int>(line.substr(0, line.length() - 1), line_num));
                            num += 1;
                        }
                        else
                        {
                            size_t ind = line.find(':');
                            string line1 = line.substr(0, ind + 1);
                            string line2 = trimmed(line.substr(ind + 1));

                            if (line2.find(':') != string::npos)
                            {
                                throw runtime_error("Syntax Error at line " + to_string(num) + " in : " + oline);
                            }
                            if (line1.find(" ") != string::npos || line1.find("\t") != string::npos)
                            {
                                throw runtime_error("Syntax Error at line " + to_string(num) + ": " + oline);
                            }
                            if (find(operators.begin(), operators.end(), line1.substr(0, line1.length() - 1)) != operators.end())
                            {
                                throw runtime_error("An operator cannot be a label. Syntax Error at line " + to_string(num) + ": " + oline);
                            }
                            if (labels[read].find(line1.substr(0, line1.length() - 1)) != labels[read].end())
                            {
                                throw runtime_error("Label is defined for the second time on line " + to_string(num));
                            }
                            labels[read].insert(pair<string, int>(line1.substr(0, line1.length() - 1), line_num));
                            inst[read].push_back(pair<string, ll>(line2, num));
                            line_num += 1;
                            num += 1;
                        }
                    }
                    else
                    {
                    

                        inst[read].push_back(pair<string, ll>(line, num));
                        line_num += 1;
                        num++;
                    }
                }
                else
                {
                    num++;
                }
            }
        }
    }

    
    row_delay = stoi(argv[num_of_cores + 3]);
    column_delay = stoi(argv[num_of_cores + 4]);
}
    catch (exception e)
    {
        cout << "Invalid Arguments" << endl;
        return 0;
    }

    

    for (int num = 0; num < num_of_cores; num++)
    {
        inst_size[num] = inst[num].size();
        int i = 0;
        while (i != inst_size[num])
        {
            //parsing instruction
            parse(inst[num][i], num);
            i++;
        }
    }

    // FOR PRINTING INSTRUCTIONS
    // for (int i = 0; i < num_of_cores; i++)
    // {
    //     cout << "core num : " << i << endl;
    //     cout << inst[i].size() << endl;
    //     for (int j = 0; j < inst[i].size(); j++)
    //     {
    //         cout << inst[i][j].first << " " << inst[i][j].second << endl;
    //     }
    // }

    //If file is failed to open or file_path is wrong in argument then throw error

    // inst_size = inst.size();

    // int i = 0;
    // while (i != inst_size)
    // {
    //     //parsing instruction
    //     parse(inst[i]);
    //     i++;
    // }

    // inst_size = instruction_list.size();

    // //if instructionList size exceeds allotted memory then throw error
    // if (inst_size > max_size)
    // {
    //     throw runtime_error("memory exceeded : Too many Instructions");
    // }

    //-------comment back to original-------
    // occupied_mem = inst_size * 4;

    //function to execute InstructionList obtained after parsing
    execute();

    return 0;
}