#include <iostream>
#include <algorithm>
using namespace std;

vector<string> req_regs;
vector<int> writeback_v(rows); 
int put_back_end = -1, put_back_start = -1, row_access_end = -1, row_access_start = -1, column_access_start = -1, column_access_end = -1, put_back_row = -1, request_issue_cycle= -1;
struct d_request{
    int addr = -1, access_row = -1, access_column =-1, op = -1, data_bus = -1,core_num =0;
    string waiting_reg;
    bool issue_msg = false;
    
};
vector<d_request> request_queue;
void erase_req(int ind);
void print_reqs();
void assign_cycle_values(int last_buffer, int last_cycle){
    d_request req = request_queue[0];
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
        put_back_row = last_buffer;
        put_back_end = last_cycle + row_delay;
        put_back_start = last_cycle + 1;
        row_access_start = last_cycle+ 1 + row_delay;
        row_access_end = last_cycle+  2*row_delay;
        column_access_start = last_cycle+ 1 + 2*row_delay;
        column_access_end = last_cycle+ 2*row_delay + column_delay;

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
    if(request_queue.size() == 0){
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
        request_queue.push_back(req);
        req_regs.push_back(req.waiting_reg);
        return;
    }
    int skip_ind = -1;
    int skip_addr = -1;
    for(int i=request_queue.size() -1; i>0; i--){
        if(req.op == 0){
            if(request_queue[i].op == 1 && request_queue[i].addr == addr){
                break;
            }
            if(request_queue[i].op == 0 && request_queue[i].addr == addr){
                erase_req(i);
                i = min((int)request_queue.size(), i+1);

            }
        }
        if(req.op == 1){
            if(request_queue[i].op == 1 && request_queue[i].waiting_reg == req.waiting_reg && request_queue[i].core_num == req.core_num){
                skip_addr = request_queue[i].addr;
                erase_req(i);
                skip_ind = i;
                break;
            }
        }
    }

    if(skip_ind != -1){
        int eligible_ind = -1;
        int skip_row = getRow(skip_addr/4);
        for(int i = skip_ind; i<request_queue.size(); i++){
            if(request_queue[i].access_row != skip_row){
                break;
            }
            if(request_queue[i].op == 1 && request_queue[i].addr == skip_addr){
                break;
            }
            if(request_queue[i].op == 0 && request_queue[i].addr == skip_addr){
                eligible_ind = i;
                break;
            }

        }
        int k=0;
        for(int i = eligible_ind-1; i>0; i--){
            if(request_queue[i].op == 1 && request_queue[i].addr == skip_addr){
                break;
            }
            if(request_queue[i].op == 0 && request_queue[i].addr == skip_addr){
                erase_req(i);
                k++;
                i = min(eligible_ind -k, i+1);

            }
        }
    }

    int index = -1;
    for(int i = request_queue.size() -1; i >= 0; i--){
        if(request_queue[i].access_row == req.access_row){
            index = i+ 1;
            break;
        }
    }
    if(index == -1 || index == request_queue.size()){
        request_queue.push_back(req);
        req_regs.push_back(req.waiting_reg);
    }
    else{
        request_queue.insert(request_queue.begin() + index, req);
        req_regs.insert(req_regs.begin() + index, req.waiting_reg);
    }
    return;

    
}

bool is_safe(Instruction temp, int core_num){
    for(auto req: request_queue){
        if(req.op == 1 && req.core_num == core_num)
            for(int i = 0; i<temp.vars.size(); i++){
                if(temp.vars[i] == req.waiting_reg){
                    return false;
                }
            }
    }
    return true;
    
}

bool is_lw_safe(Instruction temp ,int core_num){
    string reg = temp.vars[1];
    for(auto req: request_queue){
        if(req.op==1 && req.core_num==core_num){
            if(reg == req.waiting_reg){
                return false;
            }
        }
    }
    return true;
}

void erase_req(int ind){
        request_queue.erase(request_queue.begin() + ind);
        req_regs.erase(req_regs.begin() + ind);
    

}

int reg_blocked(string reg, int core){
    for(auto req: request_queue){
        if(req.op==1){
            if(reg == req.waiting_reg && core == req.core_num){
                return req.access_row;
            }
        }
    }
    return -1;
}


void reord_reg(string reg, int core){
    int index = -1;
    int row = request_queue[0].access_row;
    for(int i =0; i<request_queue.size(); i++){
        if(request_queue[i].op ==1 && request_queue[i].waiting_reg == reg && request_queue[i].core_num == core){
            index = i;
            break;
        }
    }
    if(index!=-1){
        d_request origin_req = request_queue[0];
        int origin_ind =0;
        int addr = request_queue[index].addr;
        while(index>=origin_ind){
            if(request_queue[index].addr == addr){
                d_request req = request_queue[index];
                erase_req(index);
                request_queue.insert(request_queue.begin()+ 0, req);
                req_regs.insert(req_regs.begin()+0,req.waiting_reg);
                origin_ind++;
            }
            else{
                index--;
            }
        }
    }
}

void print_reqs(){
    cout<< "addresses are:";
    for(int i =0; i<request_queue.size(); i++){
        cout<<" "<<request_queue[i].addr;
    }
    cout<<endl;
}