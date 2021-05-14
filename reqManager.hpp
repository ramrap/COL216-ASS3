#pragma once
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "helper.hpp"
using namespace std;

const int max_queue_size = 16;

int32_t **memory = NULL;

int32_t *buffer_row = NULL;
int total_cores;
int write_val = -1;
bool forwarding = false;
bool storeRedun = false;
bool loadRedun = false;
int put_back_end = -1, put_back_start = -1, row_access_end = -1, row_access_start = -1, column_access_start = -1, column_access_end = -1, put_back_row = -1, request_issue_cycle= -1;
struct d_request{
    int addr = 0, access_row = 0, access_column = 0, op = 0, data_bus = -1, core_num = 0;
    int waiting_reg;
    bool issue_msg = false;
    bool is_null = true;
    int has_con = 0;
    int lw_sw_con[max_queue_size] = {0};
    
};

struct position{
    bool check = false;
    int queue;
    int ind;
};

struct blocked_regs{
    int num = 0;
    int reg1 = 0;
    int reg2 = 0;
    int reg3 = 0;
    bool is_lw = false;
};

struct d_request null_req = {0, 0, 0, 0, -1, 0, 0, false, true, 0, {0}};
int curr_queue = -1;            // curr queue
bool in_buffer = false;
d_request request_queue[8][max_queue_size];     //dram request queues
int queue_sizes[8];                      //sizes of dram request queues
int assigned_rows[1024];                //row - queue mapping
int first_req[8];                        //chronologically first req of 
int assigned_queues[8];
int lw_qs[8];
position **loadReqs = NULL;
int num_assigned = 0;
blocked_regs *blocked_inst;
int blocks[8];
int lw_blocks[8];
int cont_reqs = 0;

void switchQueue(int buffer, int cycle);
void issue_next_request(int buffer, int cycle);
void print_reqs();



void assign_cycle_values(int last_buffer, int last_cycle){
    d_request req = request_queue[curr_queue][first_req[curr_queue]];



    if(queue_sizes[curr_queue] == 0){
        switchQueue(last_buffer, last_cycle);
    }
    request_issue_cycle = last_cycle + 1;
    if(last_buffer == req.access_row){
        put_back_end = -1;
        put_back_start = -1;
        row_access_end = -1;
        row_access_start = -1;
        column_access_start = last_cycle+1;
        column_access_end = last_cycle+ column_delay;
    }

    else{
        if(last_buffer == -1){
            put_back_end = -1;
            put_back_start = -1;
            row_access_end = last_cycle + row_delay;
            row_access_start = last_cycle + 1;
            column_access_start = last_cycle+ 1 + row_delay;
            column_access_end = last_cycle+ row_delay + column_delay;
        }
        else{
            put_back_row = last_buffer;
            put_back_end = last_cycle + row_delay;
            put_back_start = last_cycle + 1;
            row_access_start = last_cycle+ 1 + row_delay;
            row_access_end = last_cycle+ + 2*row_delay;
            column_access_start = last_cycle+ 1 + 2*row_delay;
            column_access_end = last_cycle+  2*row_delay + column_delay;
        }
    }

}

void removeRedundAssigned(){
    for(int i =0; i < 8; i++){
        if(queue_sizes[i] == 0){
            assigned_rows[assigned_queues[i]] = -1;
            assigned_queues[i] = -1;
        }
    }
}

void add_req(Instruction I, int addr, int last_buffer, int last_cycle,int core_num , int data_bus_value = -1 ){
    string inst = I.kw;
    vector<string> vars = I.vars;
    vector<int>args = I.args;
    d_request req;
    req.is_null = false;
    req.core_num= core_num;
    req.access_row = getRow(addr/4);
    req.access_column = getColumn(addr/4);
    req.addr = addr;
    req.waiting_reg = registers[vars[0]];
    req.has_con = 0;
    if(inst == "sw"){
        req.op =0;
        req.data_bus = data_bus_value;
    }
    else{
        req.op =1;
    }
    if(!in_buffer){
        in_buffer = true;
        curr_queue = assigned_rows[req.access_row];
        request_queue[curr_queue][queue_sizes[curr_queue]+first_req[curr_queue] % max_queue_size] = req;
        queue_sizes[assigned_rows[req.access_row]] += 1;
        if(req.op == 1){
            lw_qs[curr_queue]++;
        }
        assign_cycle_values(last_buffer, last_cycle);
        return;
    }

    int q = assigned_rows[req.access_row];
    int pl = -1;
    for(int j = 0; j<max_queue_size; j++){
        d_request j_req = request_queue[q][(j+ first_req[q]) % max_queue_size];
        if(j_req.is_null){
            if(pl == -1)
                pl = (j+ first_req[q]) % max_queue_size; 
            continue;
        }

        if(req.addr == j_req.addr){
            if(j_req.op == 0){
                if(req.op == 1){
                    forwarding = true;
                    write_val = j_req.data_bus;
                    return;
                }
                if(req.op==0){
                    request_queue[q][(j+ first_req[q]) % max_queue_size].core_num = req.core_num;
                    request_queue[q][(j+ first_req[q]) % max_queue_size].data_bus = req.data_bus;
                    request_queue[q][(j+ first_req[q]) % max_queue_size].waiting_reg = req.waiting_reg;
                    storeRedun = true;
                    return;
                }
            }
            else{
                if(req.op == 0){
                    if(curr_queue != q || first_req[q] != (j+ first_req[q]) % max_queue_size){
                        req.has_con++;
                        req.lw_sw_con[(j+ first_req[q]) % max_queue_size] = 1;
                    }

                }
            }

        }
    }

    if(req.op == 1){
        position pos = loadReqs[core_num][req.waiting_reg];
        if(pos.check && (curr_queue != pos.queue || first_req[curr_queue] != pos.ind) ){
            if(request_queue[pos.queue][pos.ind].has_con > 0){
                for(int i =0; i<max_queue_size; i++){
                    if(request_queue[pos.queue][pos.ind].lw_sw_con[i] == 1){
                        request_queue[pos.queue][i].lw_sw_con[pos.ind] = 0;
                        request_queue[pos.queue][i].has_con--;
                    }
                }
            }
            request_queue[pos.queue][pos.ind] = null_req;
            lw_qs[pos.queue]--;
            loadRedun = true;
        }
        loadReqs[core_num][req.waiting_reg] ={true, q, pl};
        lw_qs[q]++;    
    }
    else{
        if(req.has_con > 0){
            for(int i=0; i<max_queue_size; i++){
                if(req.lw_sw_con[i] == 1){
                    request_queue[q][i].has_con++;
                    request_queue[q][i].lw_sw_con[pl] = 1;
                }
            }
        }
    }
    
    request_queue[q][pl] = req;
    queue_sizes[q] += 1;
    return;

    
}

void switchQueue(int buffer, int cycle){
    bool found = false;

    int max = 0;
    int q_num = -1;
    int ind = -1;

    for(int i =0; i<8; i++){
        blocks[i] = 0;
        lw_blocks[i] = 0;
        int q = (i + curr_queue) % 8;
        if(queue_sizes[q] != 0){
            found = true;
            if(i != 0)
                if(q_num == -1)
                    q_num = q;
                else if(lw_qs[q] > lw_qs[q_num])
                    q_num = q;
        }
    }
    if(!found){
        in_buffer = false;
        return;
    }

    for(int i = 0; i < total_cores; i++){
        blocked_regs b = blocked_inst[i]; 
        int k = b.num;
        if(k > 0 && loadReqs[i][b.reg1].check){
            int qu = loadReqs[i][b.reg1].queue;
            int in = loadReqs[i][b.reg1].ind;
            blocks[qu] += 1;
            if(b.is_lw)
                lw_blocks[qu] += 1;
            if(qu != curr_queue){
                if(blocks[qu] > max || (blocks[qu] == max && lw_blocks[qu] > lw_blocks[q_num])){
                    max = blocks[qu];
                    q_num = qu;
                    ind = in;
                }
            }
        }

        if(k > 1 && loadReqs[i][b.reg2].check){
            int qu = loadReqs[i][b.reg2].queue;
            int in = loadReqs[i][b.reg2].ind;
            blocks[qu] += 1;
            if(qu != curr_queue){
                if(blocks[qu] > max || (blocks[qu] == max && lw_blocks[qu] > lw_blocks[q_num])){
                    max = blocks[qu];
                    q_num = qu;
                    ind = in;
                }
            }
        }

        if(k > 2 && loadReqs[i][b.reg3].check){
            int qu = loadReqs[i][b.reg3].queue;
            int in = loadReqs[i][b.reg3].ind;
            blocks[qu] += 1;
            if(qu != curr_queue){
                if(blocks[qu] > max || (blocks[qu] == max && lw_blocks[qu] > lw_blocks[q_num])){
                    max = blocks[qu];
                    q_num = qu;
                    ind = in;
                }
            }
        }
    }
    

    if(q_num == -1){
        issue_next_request(buffer, cycle);
    }
    else{
        curr_queue = q_num;
        if(ind != -1)
            first_req[curr_queue] = ind;
        issue_next_request(buffer,cycle);
    }
}


void issue_next_request(int buffer, int cycle){
    if(queue_sizes[curr_queue] != 0 && cont_reqs < 11){
        d_request req = request_queue[curr_queue][first_req[curr_queue]];
        while(req.is_null){
            first_req[curr_queue] = (first_req[curr_queue] + 1) % max_queue_size;
            req = request_queue[curr_queue][first_req[curr_queue]];
        }
        if(req.op == 0 && req.has_con >0){
            for(int i = 0; i< max_queue_size; i++){
                if(req.lw_sw_con[i] == 1){
                    first_req[curr_queue] = i;
                    break;
                }
            }
        }
        assign_cycle_values(buffer, cycle);
    }
    else{
        if(queue_sizes[curr_queue] ==0){
            assigned_rows[assigned_queues[curr_queue]] = -1;
            assigned_queues[curr_queue] = -1;
            first_req[curr_queue] = 0;
        }
        cont_reqs = 0;
        switchQueue(buffer, cycle);
    }
}


blocked_regs is_lw_safe(Instruction temp ,int core_num){
    int k =0;
    int r =0;
    string reg = temp.vars[1];
    for(int i = 0; i<8; i++){
        for(int j = 0; j<queue_sizes[i]; j++){
            d_request req = request_queue[i][(j+first_req[i]) % max_queue_size];
            if(req.op==1 && req.core_num==core_num){
                if(registers[reg] == req.waiting_reg){
                    k =1;
                    r = req.waiting_reg;
                    break;
                }
            }
        }
    }
    blocked_regs b = {k,r,0,0, true};
    return b;
}

blocked_regs is_safe(Instruction temp, int core_num){
    if(temp.kw == "lw")
        return is_lw_safe(temp, core_num);
    int k = 0;
    int r[] ={0,0,0};

    for(int i = 0; i<8; i++){
        for(int j = 0; j<queue_sizes[i]; j++){
            d_request req = request_queue[i][(j+first_req[i]) % max_queue_size];
            if(req.op == 1 && req.core_num == core_num)
                for(int l = 0; l<temp.vars.size(); l++){
                    if(registers[temp.vars[l]] == req.waiting_reg){
                        bool boo = true;
                        for(int p = 0; p< k; p++){
                            if(r[p] == req.waiting_reg)
                                boo = false;
                        }
                        if(boo){
                            r[k] = req.waiting_reg;
                            k++;
                        }
                    }
                }
        }
    }
    blocked_regs b = {k,r[0],r[1],r[2], false};
    return b;
    
    
    
}


bool assign_queue(int row){
    if(assigned_rows[row] != -1){
        if(queue_sizes[assigned_rows[row]] == max_queue_size){
            return false;
        }
        return true;
    }
    for(int i = 0; i<8; i++){
        if(queue_sizes[i] == 0){
            assigned_rows[row] = i;
            assigned_queues[i] = row;
            return true;
        }
    }
    return false;
}



void print_reqs(){
    for(int i = 0; i<8; i++){
        string s =" ";
        if(i == curr_queue)
            s="*";
        cout<<s<<i<<"\t";
        if(assigned_queues[i] == -1){
            cout<<"Not Assigned"<<endl;
            continue;
        }
        cout<<assigned_queues[i]<<"\t\t";
        for(int j = 0; j<max_queue_size; j++){
            d_request req = request_queue[i][j];
            string ss = "";
            string sk = "";
            string p ="";
            if(s == "*" && first_req[curr_queue] == j){
                ss = "[";
                sk = "]";
            }
            if(req.is_null){
                p = ss + to_string(-1) +sk;
            }
            else{
                p = ss + to_string(req.addr) +sk;
            }
            cout<<p;
            int x = 10 - p.length();
            if(j!= max_queue_size -1)
                for(int y = 0; y<x; y++){
                    cout<<" ";
                }
        }
        cout<<endl;
    }
    cout<<endl;
    for(int i=0;i<8;i++){
        cout<<lw_qs[i]<<" "<<endl;
    }
}


int total_queue_size(){
    int sum = 0;
    for(int i=0;i<8;i++){
        sum+=queue_sizes[i];
    }
    return sum;
}