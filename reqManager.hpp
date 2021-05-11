#pragma once
#include <iostream>
#include <algorithm>

#include "helper.hpp"
using namespace std;

const int max_queue_size = 64;

int32_t **memory = NULL;

int32_t *buffer_row = NULL;
int write_val = -1;
bool forwarding = false;
bool storeRedun = false;
bool loadRedun = false;
int put_back_end = -1, put_back_start = -1, row_access_end = -1, row_access_start = -1, column_access_start = -1, column_access_end = -1, put_back_row = -1, request_issue_cycle= -1;
struct d_request{
    int addr = -1, access_row = -1, access_column =-1, op = -1, data_bus = -1,core_num =-1;
    string waiting_reg;
    bool issue_msg = false;
    
};

struct position{
    bool check = false;
    int queue;
    int ind;
};

struct d_request null_req = {-1,-1,-1,-1,-1,-1,"",false};
int curr_queue = -1;            // curr queue
bool in_buffer = false;
d_request request_queue[8][max_queue_size];     //dram request queues
int queue_sizes[8];                      //sizes of dram request queues
int assigned_rows[1024];                //row - queue mapping
int first_req[8];                        //chronologically first req of 
int assigned_queues[8];
position **loadReqs = NULL;
int num_assigned = 0;

void switchQueue(int buffer, int cycle);
void print_reqs();



void assign_cycle_values(int last_buffer, int last_cycle){
    d_request req;
    while(queue_sizes[curr_queue] != 0){
        req = request_queue[curr_queue][first_req[curr_queue]];
        if(req.addr != -1)
            break;
        first_req[curr_queue] = (first_req[curr_queue] + 1) % max_queue_size;
        queue_sizes[curr_queue]--;
    }

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
    req.core_num= core_num;
    req.access_row = getRow(addr/4);
    req.access_column = getColumn(addr/4);
    req.addr = addr;
    req.waiting_reg = vars[0];
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
        assign_cycle_values(last_buffer, last_cycle);
        return;
    }

    int q = assigned_rows[req.access_row];
    for(int j = 0; j<queue_sizes[q]; j++){
        d_request j_req = request_queue[q][(j+ first_req[q]) % max_queue_size];
        if(j_req.op == 0 && req.addr == j_req.addr){
            if(req.op == 1){
                forwarding = true;
                write_val = j_req.data_bus;
                return;
            }
            if(req.op==0){
                request_queue[q][(j+ first_req[q]) % max_queue_size] = req;
                storeRedun = true;
                return;
            }

        }
    }

    if(req.op == 1){
        position pos = loadReqs[core_num][registers[req.waiting_reg]];
        if(pos.check){
            request_queue[pos.queue][pos.ind] = null_req;
            loadRedun = true;
        }
        loadReqs[core_num][registers[req.waiting_reg]] ={true, q, (queue_sizes[q]+first_req[q]) % max_queue_size};    
    }
    request_queue[q][(queue_sizes[q]+first_req[q]) % max_queue_size] = req;
    queue_sizes[q] += 1;
    return;

    
}

void switchQueue(int buffer, int cycle){
    bool found = false;
    for(int i =0; i<8; i++){
        if(queue_sizes[(i+curr_queue) % 8] != 0){
            curr_queue = (i+curr_queue) % 8;
            found = true;
            break;
        }
    }
    if(found)
        assign_cycle_values(buffer, cycle);
    else
        in_buffer = false;
}

void issue_next_request(int buffer, int cycle){
    if(queue_sizes[curr_queue] != 0){
        assign_cycle_values(buffer, cycle);
    }
    else{
        assigned_rows[assigned_queues[curr_queue]] = -1;
        assigned_queues[curr_queue] = -1;
        switchQueue(buffer, cycle);
    }
}

bool is_safe(Instruction temp, int core_num){
    for(int i = 0; i<8; i++){
        for(int j = 0; j<queue_sizes[i]; j++){
            d_request req = request_queue[i][(j+first_req[i]) % max_queue_size];
            if(req.op == 1 && req.core_num == core_num)
                for(int i = 0; i<temp.vars.size(); i++){
                    if(temp.vars[i] == req.waiting_reg){
                        return false;
                    }
                }
        }
    }
    return true;
    
}

bool is_lw_safe(Instruction temp ,int core_num){
    string reg = temp.vars[1];
    for(int i = 0; i<8; i++){
        for(int j = 0; j<queue_sizes[i]; j++){
            d_request req = request_queue[i][(j+first_req[i]) % max_queue_size];
            if(req.op==1 && req.core_num==core_num){
                if(reg == req.waiting_reg){
                    return false;
                }
            }
        }
    }
    return true;
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

int reg_blocked(string reg, int core){
    for(int i = 0; i<8; i++){
        for(int j = 0; j<queue_sizes[i]; j++){
            d_request req = request_queue[i][(j+first_req[i]) % max_queue_size];
            if(req.op==1){
                if(reg == req.waiting_reg && core == req.core_num){
                    return req.access_row;
                }
            }
        }
    }
    return -1;
}


void print_reqs(){
    cout<< "addresses are:";
    for(int i = 0; i<8; i++){
        for(int j = 0; j<queue_sizes[i]; j++){
            d_request req = request_queue[i][(j+first_req[i]) % max_queue_size];
            cout<<" "<<req.addr;
        }
    }
    cout<<endl;
}


int total_queue_size(){
    int sum = 0;
    for(int i=0;i<8;i++){
        sum+=queue_sizes[i];
    }
    return sum;
}