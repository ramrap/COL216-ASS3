#pragma once
#include <iostream>
#include "helper.hpp"
#include "reqManager.hpp"

using namespace std;

ll max_size = (1 << 20) / 4;
int buffer_updates=0;
bool no_blocking = true;
ll occupied_mem = 0;
ll memory_offset =0;

ll num_of_cores;
ll simulation_time;
int buffered = -1;
vector<vector<int>> used_mem;
vector<vector<int>> changed_mem;
vector<vector<string>> changed_regs;
vector<vector<int32_t>> registers_core;

int get_addr(Instruction I, int core_num){
    vector<string> vars = I.vars;
    vector<int> args = I.args;
    int addr;
    string reg = vars[1];
    int offset = args[0];
    addr =  registers_core[core_num][registers[reg]] + offset + memory_offset*core_num;
    return addr;
}

// //To execute instruction addi
void ADDI(Instruction I,int core_num)
{
    vector<string> vars = I.vars;
    vector<int> args = I.args;
    registers_core[core_num][registers[vars[0]]] = registers_core[core_num][registers[vars[1]]] + args[0];
    changed_regs[core_num].push_back(vars[0]);
}

// //To execute instruction add
void ADD(Instruction I,int core_num)
{
    vector<string> vars = I.vars;
    vector<int> args = I.args;
     registers_core[core_num][registers[vars[0]]] =  registers_core[core_num][registers[vars[1]]] +  registers_core[core_num][registers[vars[2]]];
    changed_regs[core_num].push_back(vars[0]);
}

//To execute instruction sub
void SUB(Instruction I,int core_num)
{
    vector<string> vars = I.vars;
    vector<int> args = I.args;
     registers_core[core_num][registers[vars[0]]] =  registers_core[core_num][registers[vars[1]]] -  registers_core[core_num][registers[vars[2]]];
    changed_regs[core_num].push_back(vars[0]);
}

// To execute instruction mul
void MUL(Instruction I,int core_num)
{
    vector<string> vars = I.vars;
    vector<int> args = I.args;
     registers_core[core_num][registers[vars[0]]] =  registers_core[core_num][registers[vars[1]]] *  registers_core[core_num][registers[vars[2]]];
    changed_regs[core_num].push_back(vars[0]);
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
    changed_regs[core_num].push_back(vars[0]);
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
    int addr = get_addr( I, core_num) ;

    if (addr < occupied_mem || addr >= (1 << 20))
    {
        throw runtime_error("address(" + to_string(addr) + ") is out of range(" + to_string(occupied_mem) + " to " + to_string((1 << 20) - 1) + ") at line " + to_string(I.line));
    }
    if (addr % 4 != 0)
    {
        throw runtime_error("unaligned address in lw: " + to_string(addr) + " at line " + to_string(I.line));
    }

    // --------------------------------------CHANGED----------------------------------------
    add_req(I, addr, buffered, cycles, core_num);

    // --------------------------------------CHANGED----------------------------------------

    if(forwarding){
        registers_core[core_num][registers[vars[0]]] = write_val;
        changed_regs[core_num].push_back(vars[0]);
    }
}

//To execute instruction SW
void SW(Instruction I, int cycles, int core_num)
{
    vector<string> vars = I.vars;
    vector<int> args = I.args;
    int addr = get_addr( I, core_num);

    if (addr < occupied_mem || addr >= (1 << 20))
    {
        throw runtime_error("address(" + to_string(addr) + ") is out of range(" + to_string(occupied_mem) + " to " + to_string((1 << 20) - 1) + ") at line " + to_string(I.line));
    }
    if (addr % 4 != 0)
    {
        throw runtime_error("unaligned address in sw: " + to_string(addr) + " at line " + to_string(I.line));
    }

    // --------------------------------------CHANGED-----------------------------------------
    add_req(I, addr, buffered, cycles, core_num, registers_core[core_num][registers[vars[0]]]);
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
    vector<vector<string>> v(num_of_cores);
    int cycles=1;
    vector<int> PC(100, 0);
    changed_regs.resize(num_of_cores);
    bool to_print = false;

    int curr_req_end = -1;
    Instruction temp;
    int temp_pc;

    int flag = 1;
    while (cycles<=simulation_time)
    {

        bool inst_rem = false;
        for (int core_num = 0; core_num < num_of_cores; core_num++)
        {   
            bool to_print_inst =false;
            if (PC[core_num] != inst_size[core_num])
            {
                inst_rem = true;
                temp = instruction_list[core_num][PC[core_num]];
                temp_pc = PC[core_num];
                string operation = temp.kw;
                // cout<<"Instruction to be executed: "<<oinst[temp.line -1]<<endl;

                // Call respective operations for instructions
                if (operation == "addi")
                {
                    if (in_buffer && no_blocking)
                    {
                        if(request_queue[curr_queue][first_req[curr_queue]].op != 1 || cycles != column_access_end || request_queue[curr_queue][first_req[curr_queue]].core_num != core_num)
                        {
                            if (is_safe(temp, core_num))
                            {
                                ADDI(temp,core_num);
                                num_addi[core_num]++;
                                PC[core_num]++;
                                to_print_inst = true;
                            }
                        }
                    }
                    else if (!in_buffer)
                    {
                        ADDI(temp,core_num);
                        num_addi[core_num]++;
                        PC[core_num]++;
                        to_print_inst = true;
                    }
                }
                else if (operation == "add")
                {
                    if (in_buffer && no_blocking)
                    {
                        if(request_queue[curr_queue][first_req[curr_queue]].op != 1 || cycles != column_access_end || request_queue[curr_queue][first_req[curr_queue]].core_num != core_num)
                        {
                            if (is_safe(temp, core_num))
                            {
                                ADD(temp,core_num);
                                num_add[core_num]++;
                                PC[core_num]++;
                                to_print_inst = true;
                            }
                        }
                    }
                    else if (!in_buffer)
                    {
                        ADD(temp,core_num);
                        num_add[core_num]++;
                        PC[core_num]++;
                        to_print_inst = true;
                    }
                }
                else if (operation == "sub")
                {
                    if (in_buffer && no_blocking)
                    {
                        if(request_queue[curr_queue][first_req[curr_queue]].op != 1 || cycles != column_access_end || request_queue[curr_queue][first_req[curr_queue]].core_num != core_num)
                        {
                            if (is_safe(temp, core_num))
                            {
                                SUB(temp,core_num);
                                num_sub[core_num]++;
                                PC[core_num]++;
                                to_print_inst = true;
                            }
                        }
                    }
                    else if (!in_buffer)
                    {
                        SUB(temp,core_num);
                        num_sub[core_num]++;
                        PC[core_num]++;
                        to_print_inst = true;
                    }
                }
                else if (operation == "mul")
                {
                    if (in_buffer && no_blocking)
                    {
                        if(request_queue[curr_queue][first_req[curr_queue]].op != 1 || cycles != column_access_end || request_queue[curr_queue][first_req[curr_queue]].core_num != core_num)
                        {
                            if (is_safe(temp, core_num))
                            {
                                MUL(temp,core_num);
                                num_mul[core_num]++;
                                PC[core_num]++;
                                to_print_inst = true;
                            }
                        }
                    }
                    else if (!in_buffer)
                    {
                        MUL(temp,core_num);
                        num_mul[core_num]++;
                        PC[core_num]++;
                        to_print_inst = true;
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
                            to_print_inst = true;
                        }
                    }
                    else if (!in_buffer)
                    {
                        BEQ(temp, PC[core_num], core_num);
                        num_beq[core_num]++;
                        to_print_inst = true;
                    }
                }
                else if (operation == "bne")
                {
                    if (in_buffer && no_blocking)
                    {
                        if (is_safe(temp, core_num))
                        {
                            BNE(temp, PC[core_num], core_num);
                            num_bne[core_num]++;
                            to_print_inst = true;
                        }
                    }
                    else if (!in_buffer)
                    {
                        BNE(temp, PC[core_num], core_num);
                        num_bne[core_num]++;
                        to_print_inst = true;
                    }
                }
                else if (operation == "slt")
                {
                    if (in_buffer && no_blocking)
                    {
                        if(request_queue[curr_queue][first_req[curr_queue]].op != 1 || cycles != column_access_end || request_queue[curr_queue][first_req[curr_queue]].core_num != core_num)
                        {
                            if (is_safe(temp, core_num))
                            {
                                SLT(temp,core_num);
                                num_slt[core_num]++;
                                PC[core_num]++;
                                to_print_inst = true;
                            }
                        }
                    }
                    else if (!in_buffer)
                    {
                        SLT(temp,core_num);
                        num_slt[core_num]++;
                        PC[core_num]++;
                        to_print_inst = true;
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
                        if (is_lw_safe(temp, core_num) && assign_queue(getRow(get_addr(temp, core_num)/4)))
                        {
                            LW(temp, cycles, core_num);
                            num_lw[core_num]++;
                            PC[core_num]++;
                            to_print_inst = true;
                        }
                    }
                    else if ((!in_buffer) && assign_queue(getRow(get_addr(temp, core_num)/4)))
                    {
                        LW(temp, cycles, core_num);
                        num_lw[core_num]++;
                        PC[core_num]++;
                        to_print_inst = true;
                    }
                }
                else if (operation == "sw")
                {
                    if (in_buffer && no_blocking)
                    {
                        if (is_safe(temp, core_num) && assign_queue(getRow(get_addr(temp, core_num)/4)))
                        {
                            SW(temp, cycles, core_num);
                            num_sw[core_num]++;
                            PC[core_num]++;
                            to_print_inst = true;
                        }
                    }
                    else if ((!in_buffer) && assign_queue(getRow(get_addr(temp, core_num)/4)))
                    {
                        SW(temp, cycles, core_num);
                        num_sw[core_num]++;
                        PC[core_num]++;
                        to_print_inst = true;
                    }
                }
                if (to_print_inst)
                {
                    if(!to_print){
                        cout << "cycle " << cycles <<":"<< endl;
                        to_print = true;
                    }
                    cout<< "In Core : " << core_num + 1 << endl;
                    last = cycles;
                    cout << "Instruction executed (PC = " << temp_pc * 4 << "): " << oinst[core_num][temp.line - 1] << endl;
                    if (temp.kw == "sw" || temp.kw == "lw")
                    {
                        if(forwarding){
                            cout<<"DRAM: Forwarded value("<<write_val<<") from queued store request."<<endl;
                        }
                        else if (total_queue_size() == 1)
                        {
                            cout << "DRAM request issued.(for memory address " << (request_queue[curr_queue][first_req[curr_queue]].access_row * columns + request_queue[curr_queue][first_req[curr_queue]].access_column) * 4 << ")" << endl;
                            if (row_access_end == -1)
                            {
                                cout << "As row " << request_queue[curr_queue][first_req[curr_queue]].access_row << " is already present in buffer, row activation is not required." << endl;
                            }
                            request_queue[curr_queue][first_req[curr_queue]].issue_msg = true;
                        }
                        else
                        {
                            cout << "Request added to queue." << endl;
                        }
                        if(forwarding || storeRedun || loadRedun){
                            forwarding = false;
                            storeRedun = false;
                            loadRedun = false;
                            removeRedundAssigned();
                        }
                    }
                }
            }


        }

            //------------------------------------------------------------------------------------------------------------------------------------------------
            //---------------------------------------------------------------------C H A N G E D--------------------------------------------------------------
            if (in_buffer)
            {
                inst_rem = true;
                bool DRAM_request = false, put_back_done = false, row_access_done = false, column_access_done = false, print_in_buffer = false;
                d_request curr_req = request_queue[curr_queue][first_req[curr_queue]];
                if (cycles == request_issue_cycle && (!curr_req.issue_msg))
                {
                    DRAM_request = true;
                    curr_req.issue_msg = true;
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
                        registers_core[curr_req.core_num][registers[curr_req.waiting_reg]] = memory[curr_req.access_row][curr_req.access_column];
                        changed_regs[curr_req.core_num].push_back(curr_req.waiting_reg);
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
                        cout << "As row " << curr_req.access_row << " is already present in buffer, row activation is not required." << endl;
                    }
                    curr_req.issue_msg = true;
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
                    queue_sizes[curr_queue]-=1;
                    first_req[curr_queue] = first_req[curr_queue] + 1 % max_queue_size;
                    issue_next_request(buffered, cycles);
                    if(in_buffer){
                        d_request n_req = request_queue[curr_queue][first_req[curr_queue]];
                        if(n_req.op == 1){
                            loadReqs[n_req.core_num][registers[n_req.waiting_reg]].check = false;
                        }
                    }

                }
            }
            //-----------------------------------------------------------------------------------------------------------------------------------------------------
            //-----------------------------------------------------------------------------------------------------------------------------------------------------
            bool updated_regs_msg = false;
            for(int i=0;i<num_of_cores;i++){
                registers_core[i][registers["$zero"]] = 0;
                if (changed_regs[i].size() != 0)
                {
                    sort(changed_regs[i].begin(), changed_regs[i].end());
                    if (changed_regs[i][0] != "$zero")
                    {
                        if(!updated_regs_msg)
                        {    
                            cout << "Updated Registers: \n";
                            updated_regs_msg = true;
                        }
                        cout<<"\tIn core "<<i+1<<": ";
                        cout << changed_regs[i][0] << " = " << registers_core[i][registers[changed_regs[i][0]]];
                        for (int j = 1; j < changed_regs[i].size(); j++)
                        {
                            if (changed_regs[i][j] != changed_regs[i][j - 1] && changed_regs[i][j] != "$zero")
                            {
                                cout << "    " << changed_regs[i][j] << " = " << registers_core[i][registers[changed_regs[i][j]]];
                            }
                        }
                        cout << endl;
                    }
                    changed_regs[i].clear();
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
            changed_mem.clear();
            if(!inst_rem){
                break;
            }
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
