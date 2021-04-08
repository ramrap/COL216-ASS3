#include <iostream>

using namespace std;

vector<string> req_regs;

struct d_request{
    int addr, access_row, access_column, row_access_start, row_access_end, column_access_start, column_access_end, op, put_back_row, put_back_start, put_back_end, data_bus, request_issue;
    string waiting_reg;
    bool issue_msg = false;
};

vector<d_request> request_queue;
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

    if(request_queue.size() != 0){
        last_buffer = request_queue[request_queue.size()-1].access_row;
        last_cycle = request_queue[request_queue.size()-1].column_access_end;
    }
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
            req.column_access_end = last_cycle+ +row_delay +column_delay;
        }
        else{
            req.put_back_row = last_buffer;
            req.put_back_end = last_cycle + row_delay;
            req.put_back_start = last_cycle + 1;
            req.row_access_start = last_cycle+ 1 + row_delay;
            req.row_access_end = last_cycle+ + 2*row_delay;
            req.column_access_start = last_cycle+ 1 + 2*row_delay;
            req.column_access_end = last_cycle+ + 2*row_delay +column_delay;
        }
    }
    request_queue.push_back(req);
    req_regs.push_back(req.waiting_reg);
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