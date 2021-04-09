#include <iostream>

using namespace std;

vector<string> req_regs;

struct d_request{
    int addr = -1, access_row = -1, access_column =-1, row_access_start=-1, row_access_end = -1, column_access_start = -1, column_access_end = -1, op = -1, put_back_row = -1, put_back_start = -1, put_back_end = -1, data_bus = -1, request_issue = -1;
    string waiting_reg;
    bool issue_msg = false;
};
bool global_has_sw;
vector<d_request> request_queue;
void erase_req(int ind);

void update_cycle_values(){

    int last_buffer = request_queue[0].access_row;
    int last_cycle = request_queue[0].column_access_end;
    
    bool has_sw = false;
    if(request_queue[0].op == 0){
        has_sw = true;
    }
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
            if(req.op == 0){
                has_sw = true;
            }
        }

        else{
            if(has_sw){
                req.put_back_row = last_buffer;
                req.put_back_end = last_cycle + row_delay;
                req.put_back_start = last_cycle + 1;
                req.row_access_start = last_cycle+ 1 + row_delay;
                req.row_access_end = last_cycle+  2*row_delay;
                req.column_access_start = last_cycle+ 1 + 2*row_delay;
                req.column_access_end = last_cycle+ 2*row_delay + column_delay;
            }
            else{
                req.put_back_end = -1;
                req.put_back_start = -1;
                req.row_access_start = last_cycle+ 1;
                req.row_access_end = last_cycle+ row_delay;
                req.column_access_start = last_cycle+ 1 + row_delay;
                req.column_access_end = last_cycle+  row_delay + column_delay;
            }
            if(req.op == 0){
                has_sw = true;
            }
            else{
                has_sw = false;
            }
        }
        last_buffer = req.access_row;
        last_cycle = req.column_access_end;
        request_queue[i] = req;

    }
    global_has_sw = has_sw;
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
            if(last_buffer == -1 || (!global_has_sw)){
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
        if(req.op == 0){
            global_has_sw = true;
        }
        else{
            global_has_sw = false;
        }
        request_queue.push_back(req);
        req_regs.push_back(req.waiting_reg);
        return;
    }
    for(int i=request_queue.size() -1; i>0; i--){
        if(req.op == 0){
            if(request_queue[i].op == 1 && request_queue[i].addr == addr){
                break;
            }
            if(request_queue[i].op == 0 && request_queue[i].addr == addr){
                erase_req(i);
                i++;

            }
        }
        if(req.op == 1){
            if(request_queue[i].op == 1 && request_queue[i].waiting_reg == req.waiting_reg){
                erase_req(i);
                i++;
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

void erase_req(int ind){
    for(int i =ind; i<request_queue.size() -1; i++){
        request_queue[ind] = request_queue[ind+1];
        req_regs[ind] = req_regs[ind+1];
    }
}