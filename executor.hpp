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
    curr_write[core_num] = {true, registers[vars[0]], registers_core[core_num][registers[vars[1]]] + args[0]};
}

// //To execute instruction add
void ADD(Instruction I,int core_num)
{
    vector<string> vars = I.vars;
    vector<int> args = I.args;
    curr_write[core_num] = {true, registers[vars[0]], registers_core[core_num][registers[vars[1]]] +  registers_core[core_num][registers[vars[2]]]};
}

//To execute instruction sub
void SUB(Instruction I,int core_num)
{
    vector<string> vars = I.vars;
    vector<int> args = I.args;
    curr_write[core_num] = {true, registers[vars[0]], registers_core[core_num][registers[vars[1]]] -  registers_core[core_num][registers[vars[2]]]};
}

// To execute instruction mul
void MUL(Instruction I,int core_num)
{
    vector<string> vars = I.vars;
    vector<int> args = I.args;
    curr_write[core_num] = {true, registers[vars[0]], registers_core[core_num][registers[vars[1]]] *  registers_core[core_num][registers[vars[2]]]};
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
    curr_write[core_num] = {true, registers[vars[0]], registers_core[core_num][registers[vars[1]]] <  registers_core[core_num][registers[vars[2]]] ? 1 : 0};
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
    int fl = 1, last = 0;
    vector<int> num_add(num_of_cores, 0), num_addi(num_of_cores, 0), num_j(num_of_cores, 0), num_sub(num_of_cores, 0), num_mul(num_of_cores, 0), num_bne(num_of_cores, 0), num_beq(num_of_cores, 0), num_lw(num_of_cores, 0), num_sw(num_of_cores, 0), num_slt(num_of_cores, 0);
    vector<vector<string>> v(num_of_cores);
    int cycles=1;
    vector<int> PC(num_of_cores, 0);
    vector<bool> done(num_of_cores, false);
    vector<bool> done_msg(num_of_cores, false);
    bool DONE;
    changed_regs.resize(num_of_cores);
    bool to_print = false, issue_write = false;

    int curr_req_end = -1;
    Instruction temp;
    int temp_pc;
    bool busy_write;
    int busy_core1;
    int busy_core2;
    int flag = 1;
    while (cycles<=simulation_time)
    {
        busy_write = false;
        busy_core1 = -1;
        busy_core2 = -1;
        if(in_buffer && cycles == column_access_end && request_queue[curr_queue][first_req[curr_queue]].op == 1){
            busy_core1 = request_queue[curr_queue][first_req[curr_queue]].core_num;
            busy_write = true;
        }
        if((curr_mrm.check && curr_mrm.forwarding && mrm_delay_end == cycles) || cycles == frwd_issue_write_cycle){
            busy_core2 = curr_mrm.core;
            busy_write = true;
        }

        bool inst_rem = false;
        for (int core_num = 0; core_num < num_of_cores; core_num++)
        {   
            bool to_print_inst =false;
            blocked_inst[core_num] = {0, 0, 0, 0, false};
            if (PC[core_num] != inst_size[core_num])
            {
                inst_rem = true;
                temp = instruction_list[core_num][PC[core_num]];
                temp_pc = PC[core_num];
                string operation = temp.kw;
                // cout<<"Instruction to be executed: "<<oinst[temp.line -1]<<endl
                // Call respective operations for instructions
                if (operation == "addi")
                {
                    
                    if (in_buffer && no_blocking)
                    {
                        if(!busy_write || (!busy_core1 == core_num && !busy_core2 == core_num))
                        {
                            blocked_regs b = is_safe(temp, core_num);
                            int k = b.num;
                            if (k == 0)
                            {
                                ADDI(temp,core_num);
                                num_addi[core_num]++;
                                PC[core_num]++;
                                to_print_inst = true;
                                write_cycles[core_num] = cycles;
                                issue_write = true;
                            } else
                                blocked_inst[core_num] = b;
                        }
                    }
                    else if (!in_buffer)
                    {
                        ADDI(temp,core_num);
                        num_addi[core_num]++;
                        PC[core_num]++;
                        to_print_inst = true;
                        write_cycles[core_num] = cycles;
                        issue_write = true;
                        
                    }
                }
                else if (operation == "add")
                {
                    if (in_buffer && no_blocking)
                    {
                        if(!busy_write || (!busy_core1 == core_num && !busy_core2 == core_num))
                        {
                            blocked_regs b = is_safe(temp, core_num);
                            int k = b.num;
                            if (k == 0)
                            {
                                ADD(temp,core_num);
                                num_add[core_num]++;
                                PC[core_num]++;
                                to_print_inst = true;
                                write_cycles[core_num] = cycles;
                                issue_write = true;
                            } else
                                blocked_inst[core_num] = b;
                        }
                    }
                    else if (!in_buffer)
                    {
                        ADD(temp,core_num);
                        num_add[core_num]++;
                        PC[core_num]++;
                        to_print_inst = true;
                        write_cycles[core_num] = cycles;
                        issue_write = true;

                    }
                }
                else if (operation == "sub")
                {
                    if (in_buffer && no_blocking)
                    {
                        if(!busy_write || (!busy_core1 == core_num && !busy_core2 == core_num))
                        {
                            blocked_regs b = is_safe(temp, core_num);
                            int k = b.num;
                            if (k == 0)
                            {
                                SUB(temp,core_num);
                                num_sub[core_num]++;
                                PC[core_num]++;
                                to_print_inst = true;
                                write_cycles[core_num] = cycles;
                                issue_write = true;
                            } else
                                blocked_inst[core_num] = b;
                        }
                    }
                    else if (!in_buffer)
                    {
                        SUB(temp,core_num);
                        num_sub[core_num]++;
                        PC[core_num]++;
                        to_print_inst = true;
                        write_cycles[core_num] = cycles;
                        issue_write = true;
                    }
                }
                else if (operation == "mul")
                {
                    if (in_buffer && no_blocking)
                    {
                        if(!busy_write || (!busy_core1 == core_num && !busy_core2 == core_num))
                        {
                            blocked_regs b = is_safe(temp, core_num);
                            int k = b.num;
                            if (k == 0)
                            {
                                MUL(temp,core_num);
                                num_mul[core_num]++;
                                PC[core_num]++;
                                to_print_inst = true;
                                write_cycles[core_num] = cycles;
                                issue_write = true;
                            } else
                                blocked_inst[core_num] = b;
                        }
                    }
                    else if (!in_buffer)
                    {
                        MUL(temp,core_num);
                        num_mul[core_num]++;
                        PC[core_num]++;
                        to_print_inst = true;
                        write_cycles[core_num] = cycles;
                        issue_write = true;
                    }
                }
                else if (operation == "beq")
                {
                    if (in_buffer && no_blocking)
                    {
                        blocked_regs b = is_safe(temp, core_num);
                        int k = b.num;
                        if (k == 0)
                        {
                            BEQ(temp, PC[core_num], core_num);
                            num_beq[core_num]++;
                            to_print_inst = true;
                        } else
                            blocked_inst[core_num] = b;
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
                        blocked_regs b = is_safe(temp, core_num);
                        int k = b.num;
                        if (k == 0)
                        {
                            BNE(temp, PC[core_num], core_num);
                            num_bne[core_num]++;
                            to_print_inst = true;
                        } else
                            blocked_inst[core_num] = b;
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
                        if(!busy_write || (!busy_core1 == core_num && !busy_core2 == core_num))
                        {
                            blocked_regs b = is_safe(temp, core_num);
                            int k = b.num;
                            if (k == 0)
                            {
                                SLT(temp,core_num);
                                num_slt[core_num]++;
                                PC[core_num]++;
                                to_print_inst = true;
                                write_cycles[core_num] = cycles;
                                issue_write = true;
                            } else
                                blocked_inst[core_num] = b;
                        }
                    }
                    else if (!in_buffer)
                    {
                        SLT(temp,core_num);
                        num_slt[core_num]++;
                        PC[core_num]++;
                        to_print_inst = true;
                        write_cycles[core_num] = cycles;
                        issue_write = true;
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
                        blocked_regs b = is_safe(temp, core_num);
                        int k = b.num;
                        if (k == 0 && (!curr_mrm.check) && (issue_next_cycle!= cycles) && assign_queue(getRow(get_addr(temp, core_num)/4)) )
                        {
                            LW(temp, cycles, core_num);
                            num_lw[core_num]++;
                            PC[core_num]++;
                            to_print_inst = true;
                        } else if(k != 0)
                            blocked_inst[core_num] = b;
                    }
                    else if ((!in_buffer)&& (!curr_mrm.check) && (issue_next_cycle!= cycles)  && assign_queue(getRow(get_addr(temp, core_num)/4)))
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
                        blocked_regs b = is_safe(temp, core_num);
                        int k = b.num;
                        if (k == 0 && (!curr_mrm.check) && (issue_next_cycle!= cycles) && assign_queue(getRow(get_addr(temp, core_num)/4)))
                        {
                            SW(temp, cycles, core_num);
                            num_sw[core_num]++;
                            PC[core_num]++;
                            to_print_inst = true;
                        } else if(k != 0)
                            blocked_inst[core_num] = b;
                    }
                    else if ((!in_buffer) && (!curr_mrm.check) && (issue_next_cycle!= cycles)  && assign_queue(getRow(get_addr(temp, core_num)/4)))
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
                        last = print_cycle(last, cycles);
                        to_print = true;
                        cout<<"Instruction Execution:"<<endl;
                    }
                    cout<< "\tIn core " << core_num + 1 <<" (PC = " << temp_pc * 4 << "): ";
                    cout << oinst[core_num][temp.line - 1] << endl;
                    
                    removeRedundAssigned();
                    
                }
            }
            else{
                done[core_num] = true;
                if(requests_from_core(core_num) == 0 && done_msg[core_num]==false){
                    if(!to_print){
                        to_print = true;
                        last = print_cycle(last, cycles);
                        
                    }
                    done_msg[core_num]=true;
                    cout << "In core " << core_num + 1 << ": Program Executed Successfully!\n";
                }
                DONE = true;
                for(int i=0;i<num_of_cores; i++){
                    if(!done[i]){
                        DONE = false;
                    }
                }
            }


        }
            //------------------------------------------------------------------------------------------------------------------------------------------------
            //---------------------------------------------------------------------C H A N G E D--------------------------------------------------------------
        if (in_buffer)
        {
            inst_rem = true;
            bool DRAM_request = false, put_back_done = false, row_access_done = false, column_access_done = false, print_in_buffer = false, issue_next = false, waiting_mrm= false, waiting_write_port = false;
            d_request curr_req = request_queue[curr_queue][first_req[curr_queue]];
            if(cycles == issue_next_cycle){
                if(!curr_mrm.check){
                    if(curr_req.op == 1){
                        lw_qs[curr_queue]--;
                        if(curr_req.has_con > 0){
                            for(int i =0; i<max_queue_size; i++){
                                if(curr_req.lw_sw_con[i] == 1){
                                    request_queue[curr_queue][i].lw_sw_con[first_req[curr_queue]] = 0;
                                    request_queue[curr_queue][i].has_con--;
                                }
                            }
                        }
                    }
                    cont_reqs++;
                    request_queue[curr_queue][first_req[curr_queue]] = null_req;
                    queue_sizes[curr_queue]-=1;
                    update_copies();
                    first_req[curr_queue] = first_req[curr_queue] + 1 % max_queue_size;
                
                    issue_next_request(buffered, cycles);
                    if(in_buffer){
                        d_request n_req = request_queue[curr_queue][first_req[curr_queue]];
                        if(n_req.op == 1){
                            loadReqs[n_req.core_num][n_req.waiting_reg].check = false;
                        }
                    }
                }
                else
                    issue_next_cycle = cycles + 1;
            }
            else{
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
                        issue_next = true;
                        
                    }
                    else if(!curr_write[curr_req.core_num].check)
                    {
                        curr_write[curr_req.core_num] = {true, curr_req.waiting_reg, memory[curr_req.access_row][curr_req.access_column]};
                        write_cycles[curr_req.core_num] = cycles;
                        issue_write = true;
                        issue_next = true;
                    }
                    else{
                        cout<<"ERRROORRRRR"<<endl;
                        return;
                    }
                    column_access_done = true;
                    print_in_buffer = true;
                
                }
            
                if (print_in_buffer)
                {
                    if(!to_print){
                        to_print = true;
                        last = print_cycle(last, cycles);
                    } else
                        cout<<endl;
                    cout<<"DRAM Updates:\n";
                    
                }
                if (DRAM_request)
                {
                    cout << "\tDRAM request issued.(for memory address " << (curr_req.access_row * columns + curr_req.access_column) * 4 << ")" << endl;
                    if (row_access_end == -1)
                    {
                        cout << "\tAs row " << curr_req.access_row << " is already present in buffer, row activation is not required." << endl;
                    }
                    curr_req.issue_msg = true;
                }
                if (put_back_done)
                {
                    cout << "\tWroteback row " << put_back_row << ".";
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
                    cout << "\t Row " << curr_req.access_row<<" accessesd." ;
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
                    cout << "\tColumn " << curr_req.access_column<<" accessed.";
                    if (column_access_start < column_access_end)
                    {
                        cout << "(In cycles " << column_access_start << "-" << column_access_end << ")";
                    }
                    else if (column_access_end == column_access_start)
                    {
                        cout << "(In cycle " << column_access_start << ")";
                    }
                    if(waiting_write_port){
                        cout <<" [Waiting for write-port to be free].";
                    } else{
                        cout <<" [Write-value sent to write-port of core "<<curr_req.core_num + 1<<"]";
                    }
                    cout<<endl;
                }
                if(issue_next){
                    if(!curr_mrm.check){
                        if(curr_req.op == 1){
                            lw_qs[curr_queue]--;
                            if(curr_req.has_con > 0){
                                for(int i =0; i<max_queue_size; i++){
                                    if(curr_req.lw_sw_con[i] == 1){
                                        request_queue[curr_queue][i].lw_sw_con[first_req[curr_queue]] = 0;
                                        request_queue[curr_queue][i].has_con--;
                                    }
                                }
                            }
                        }
                        cont_reqs++;
                        request_queue[curr_queue][first_req[curr_queue]] = null_req;
                        queue_sizes[curr_queue]-=1;
                        update_copies();
                        first_req[curr_queue] = first_req[curr_queue] + 1 % max_queue_size;
                    
                        issue_next_request(buffered, cycles);
                        if(in_buffer){
                            d_request n_req = request_queue[curr_queue][first_req[curr_queue]];
                            if(n_req.op == 1){
                                loadReqs[n_req.core_num][n_req.waiting_reg].check = false;
                            }
                        }
                    }
                    else{
                        issue_next_cycle = cycles + 1;
                    }

                }
            }
        }

        if(frwd_issue_write_cycle == cycles){
            if(!curr_write[curr_mrm.core].check){
                curr_write[curr_mrm.core] = {true, curr_mrm.reg, curr_mrm.val};
                write_cycles[curr_mrm.core] = cycles;
                issue_write = true;
                curr_mrm = null_mrm;
                update_copies();
            } else{
                frwd_issue_write_cycle = cycles + 1;
            }
        }
        if(curr_mrm.check && cycles == mrm_delay_start - 1){
            if(!to_print){
                last = print_cycle(last, cycles);
                to_print = true;
            } else
                cout<<endl;
            
            cout<<"MRM Updates:\n";
            if(curr_mrm.add_req){
                cout<<"\tDRAM-request sent to MRM (for address "<<curr_mrm.addr<<")"<<endl;
            } else{
                cout<<"\tRequest for issuing next DRAM-request initiated."<<endl;
            }
        }

        if(curr_mrm.check && cycles == mrm_delay_end){
            if(!to_print){
                last = print_cycle(last, cycles);
                to_print = true;
            } else
                cout<<endl;
            cout<<"MRM Updates:\n";
            bool up = true; 
            if(curr_mrm.add_req){
                if(!curr_mrm.forwarding){
                    cout<<"\tDRAM-request added to request-queue (for address "<<curr_mrm.addr<<")";
                    if(curr_mrm.sw_redun){
                        cout<<" [SW-SW redundancy]"<<endl;
                    } else if(curr_mrm.lw_redun){
                        cout<<" [In cycles "<<cycles-1<<"-"<<cycles<<"] "<<"[LW-LW redundancy]"<<endl;
                    } else if(curr_mrm.first_req){
                        cout<<"\n\tSending this request to DRAM\n";
                    } else {
                        cout<<endl;
                    }
                    update_copies(0);
                } else{
                    cout<<"\tForwarded value("<<curr_mrm.val<<") from a store-request to write data bus.";
                    if(!curr_write[curr_mrm.core].check){
                        cout <<" [Write-value sent to write-port of core "<<curr_mrm.core + 1<<"]";
                        curr_write[curr_mrm.core] = {true, curr_mrm.reg, curr_mrm.val};
                        write_cycles[curr_mrm.core] = cycles + 1;
                        issue_write = true;
                        update_copies(0);
                    } else{
                        cout <<" [Waiting for write-port to be free].";
                        frwd_issue_write_cycle = cycles + 1;
                        up = false;
                    }
                }
            } else{
                if(curr_mrm.row_switch){
                    cout<<"\tSwitched to request queue "<<assigned_rows[getRow(curr_mrm.addr/4)]<<".\n";
                }
                cout<<"\tFetched next DRAM-request [for address: "<<curr_mrm.addr<<"]\n";
                cout<<"\tSending this request to DRAM\n";
                update_copies();
            }
            if(up){
                curr_mrm = null_mrm;
            }
        }

        if(issue_write){
            if(!to_print){
                last = print_cycle(last, cycles);
                to_print = true;
            } else
                cout<<endl;
            
            cout<<"Write-port Updates:\n";
            for(int i =0; i< num_of_cores; i++){
                if(curr_write[i].check && write_cycles[i] == cycles){
                    cout<<"\tIn core "<<i + 1<<": Write request issued for register "<<reg_name[curr_write[i].reg]<<". [Register will be updated on clock-edge]"<<endl;
                    registers_core[i][curr_write[i].reg] = curr_write[i].val;
                    changed_regs[i].push_back(reg_name[curr_write[i].reg]);
                    curr_write[i] = null_write;
                }
                
            }
        }
        
        issue_write = false;

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
                        cout << "\nUpdated Registers [After this cycle(on the clock-edge)]: \n";
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
            cout<<endl;
            print_reqs();
            cout << endl;
        }
        to_print = false;
        changed_mem.clear();
        if(!inst_rem){
            break;
        }
        cycles++;
    }
    
       
    

    removeRedundAssigned();
    update_copies();
    // Increasing PC until we reach last line
     if (buffered != -1 && total_queue_size() == 0 && DONE && cycles - 1 + row_delay <= simulation_time)
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
            removeRedundAssigned();
            update_copies();
            print_reqs();
        }
    



    for (int core_num = 0; core_num < num_of_cores; core_num++)
    {

           cout << endl;
        
        cout << "*****Core " << core_num + 1 << " Statistics***** \n";
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
