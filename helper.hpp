#pragma once
#include <iostream>
#include <sstream>
#include <vector>
#include <string> 
using namespace std;


const string WS = " \t";
vector<string> operators = {"add", "sub", "mul", "bne", "beq", "slt", "addi", "lw", "sw", "j"};
vector<string> reg_name = {"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra" };
int row_delay =10, column_delay = 2;
int rows = (1<<10);
int columns = (1<<10)/4;

struct Instruction
{
    /* data */
    string kw;
    vector<string>vars;
    vector<int>args;
    long long line;
};

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

int getRow(int num){
    return num / columns;
}
int getColumn(int num){
    return num % columns;
}

// Convert long long to Hex
string llToHex(long long num){
    stringstream sstream;
    sstream<< hex<< num;
    return(sstream.str());
}

bool compare_address(vector<int> v1, vector<int> v2){
    if(v1[0] == v2[0]){
        return v1[1] < v2[1];
    }
    else{
        return v1[0] < v2[0];
    }
}