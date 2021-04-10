#include <iostream>
#include <algorithm>
using namespace std;

vector<string> req_regs;
vector<int> writeback_v(rows); 
struct d_request{
    int addr = -1, access_row = -1, access_column =-1, row_access_start=-1, row_access_end = -1, column_access_start = -1, column_access_end = -1, op = -1, put_back_row = -1, put_back_start = -1, put_back_end = -1, data_bus = -1, request_issue = -1;
    string waiting_reg;
    bool issue_msg = false;
};
vector<d_request> request_queue;
void erase_req(int ind);

void update_cycle_values(){

    int last_buffer = request_queue[0].access_row;
    int last_cycle = request_queue[0].column_access_end;

    for(int i = 1; i < request_queue.size(); i++){
        d_request req = request_queue[i];
        req.request_issue = last_cycle + 1;
        if(last_buffer == req.access_row){
            req.put_back_end = -1;
            req.put_back_start = -1;
            req.row_access_end = -1;
            req.row_access_start = -1;
            req.column_access_start = last_cycle+1;
            req.column_access_end = last_cycle+ column_delay;
        }

        else{
            req.put_back_row = last_buffer;
            req.put_back_end = last_cycle + row_delay;
            req.put_back_start = last_cycle + 1;
            req.row_access_start = last_cycle+ 1 + row_delay;
            req.row_access_end = last_cycle+  2*row_delay;
            req.column_access_start = last_cycle+ 1 + 2*row_delay;
            req.column_access_end = last_cycle+ 2*row_delay + column_delay;

        }
        last_buffer = req.access_row;
        last_cycle = req.column_access_end;
        request_queue[i] = req;

    }
}


void add_req(Instruction I, int addr, int last_buffer, int last_cycle, int data_bus_value = -1){
    string inst = I.kw;
    vector<string> vars = I.vars;
    vector<int>args = I.args;
    d_request req;
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
        req.request_issue = last_cycle + 1;
        if(last_buffer == req.access_row){
            req.put_back_end = -1;
            req.put_back_start = -1;
            req.row_access_end = -1;
            req.row_access_start = -1;
            req.column_access_start = last_cycle+1;
            req.column_access_end = last_cycle+ column_delay;
        }

        else{
            if(last_buffer == -1){
                req.put_back_end = -1;
                req.put_back_start = -1;
                req.row_access_end = last_cycle + row_delay;
                req.row_access_start = last_cycle + 1;
                req.column_access_start = last_cycle+ 1 + row_delay;
                req.column_access_end = last_cycle+ row_delay + column_delay;
            }
            else{
                req.put_back_row = last_buffer;
                req.put_back_end = last_cycle + row_delay;
                req.put_back_start = last_cycle + 1;
                req.row_access_start = last_cycle+ 1 + row_delay;
                req.row_access_end = last_cycle+ + 2*row_delay;
                req.column_access_start = last_cycle+ 1 + 2*row_delay;
                req.column_access_end = last_cycle+  2*row_delay + column_delay;
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
            if(request_queue[i].op == 1 && request_queue[i].waiting_reg == req.waiting_reg){
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
    update_cycle_values();
    return;

    
}

bool is_safe(Instruction temp){
    for(auto req: request_queue){
        if(req.op == 1)
            for(int i = 0; i<temp.vars.size(); i++){
                if(temp.vars[i] == req.waiting_reg){
                    return false;
                }
            }
    }
    return true;
    
}

bool is_lw_safe(Instruction temp){
    string reg = temp.vars[1];
    for(auto req: request_queue){
        if(req.op==1){
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

int reg_blocked(string reg){
    for(auto req: request_queue){
        if(req.op==1){
            if(reg == req.waiting_reg){
                return req.access_row;
            }
        }
    }
    return -1;
}

void update_cycle_values_between(d_request req){
    int row = req.access_row;
    request_queue[0].put_back_row = req.put_back_row;
    request_queue[0].put_back_end = req.put_back_end;
    request_queue[0].put_back_start = req.put_back_start;
    request_queue[0].column_access_end = req.column_access_end;
    request_queue[0].column_access_start = req.column_access_start;
    request_queue[0].row_access_end = req.row_access_end;
    request_queue[0].row_access_start = req.row_access_start;
    request_queue[0].request_issue = req.request_issue;
    update_cycle_values();
}

void reord_reg(string reg){
    int index = -1;
    int row = request_queue[0].access_row;
    for(int i =0; i<request_queue.size(); i++){
        if(request_queue[i].op ==1 && request_queue[i].waiting_reg == reg){
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
        update_cycle_values_between(origin_req);
    }
}