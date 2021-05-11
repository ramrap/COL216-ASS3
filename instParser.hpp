#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>
#include <cstring>
#include <cmath>
#include "helper.hpp"

#include <unordered_map>
using namespace std;

#define ll long long int

vector<ll> inst_size;
vector<vector<pair<string, ll>>> inst;
vector<vector<Instruction>> instruction_list;
vector<vector<string>> oinst;
vector<unordered_map<string, int>> labels; 

//Function to parse Current Instruction and store it to Instruction_list
void parse(pair<string, long long> instru, int core_num)
{
    string line = instru.first;
    long long num = instru.second;

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


