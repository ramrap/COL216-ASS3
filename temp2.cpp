// c++14
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>


#include "helper.hpp"
#include "reqManager.hpp"
#include "instParser.hpp"
#include "executor.hpp"

using namespace std;

#define ll long long int

// build with: g++ main.cpp -o main -std=c++14
//./main > test.out


// initialize registers and memory array.
void initialise_memory()
{
    //Intialising all registers to 0;
    oinst.resize(num_of_cores);
    inst.resize(num_of_cores);
    instruction_list.resize(num_of_cores);
    labels.resize(num_of_cores);
    inst_size.resize(num_of_cores);
    registers_core.resize(num_of_cores, vector<int32_t>(32, 0));
    int j = 0;
    registers.insert(make_pair("$zero", j++));
    registers.insert(make_pair("$sp", j++));
    registers.insert(make_pair("$gp", j++));
    registers.insert(make_pair("$fp", j++));
    registers.insert(make_pair("$at", j++));
    registers.insert(make_pair("$ra", j++));

    for (int i = 0; i <= 9; i++)
    {
        string s = "$t";
        s += to_string(i);
        registers.insert(make_pair(s, j++));
        if (i <= 7)
        {
            s = "$s";
            s += to_string(i);
            registers.insert(make_pair(s, j++));
        }
    }

    for (int i = 0; i < 4; i++)
    {
        string s = "$a";
        s += to_string(i);
        registers.insert(make_pair(s, j++));
        if (i <= 1)
        {
            s = "$v";
            s += to_string(i);
            registers.insert(make_pair(s, j++));
            s = "$k";
            s += to_string(i);
            registers.insert(make_pair(s, j++));
        }
    }

    for (int i = 0; i < num_of_cores; i++)
    {
        registers_core[i][registers["$sp"]] = max_size * 4 - 4;
        // cout<<registers_core[i][registers["$sp"]]<<endl;
    }

    //allocating size to memory Array
    buffer_row = new int32_t[columns];
    memory = new int32_t *[rows];
    for (int i = 0; i < rows; i++)
    {
        memory[i] = new int32_t[columns];
    }
    memset(queue_sizes, 0, sizeof(queue_sizes));
    memset(assigned_rows, -1, sizeof(assigned_rows));
    memset(first_req, 0, sizeof(first_req));
    memset(assigned_queues, -1, sizeof(assigned_queues));


    struct position pos ={false, -1, -1};
    loadReqs = new position *[num_of_cores];
    for(int i=0;i<num_of_cores;i++){
        loadReqs[i] = new position[32];
    }
    for(int i=0; i< num_of_cores; i++)
        for(int j = 0; j < 32; j++){
            loadReqs[i][j] = pos;
        }
}

// main function
int main(int argc, char *argv[])
{


    // read file name from CLI
    if (argc == 1)
    {
        cout << "Invalid argument: Please provide input file path" << endl;
        return 0;
    }

    // if(argc == 3){
    //     string argv2(argv[2]);
    //     if(argv2.compare("p1") == 0){
    //         no_blocking = false;
    //     }
    //     else if(argv2.compare("p2") == 0){
    //         no_blocking = true;
    //     }
    //     else{
    //         cout<<"Invalid argument: "<<argv2<<" Please provide valid arguments."<<endl;
    //         return 0;
    //     }

    // }
    // if (argc >=4){
    //     row_delay = stoi(argv[2]);
    //     column_delay = stoi(argv[3]);
    //     if(row_delay <0 || column_delay<0){
    //         cout<<"Invalid argument: Row and Column access delay cannot be negative."<<endl;
    //         return 0;
    //     }
    //     if (argc == 5){
    //         string argv4(argv[4]);
    //         if(argv4 == "p1"){
    //             no_blocking = false;
    //         }
    //         else if(argv4 == "p2"){
    //             no_blocking = true;
    //         }
    //         else{
    //             cout<<"Invalid argument: "<<argv4<<" Please provide valid arguments."<<endl;
    //             return 0;
    //         }
    //     }
    // }
try{
    num_of_cores = stoi(argv[1]);
    initialise_memory();
    simulation_time = stoi(argv[2]);
    for (int read = 0; read < num_of_cores; read++)
    {
       
        ifstream file(argv[read + 3]);
        if (!file.is_open())
        {
            cout << "Error: Provide Valid File Path For " << argv[read + 3] << endl;
            return 0;
        }
        if (file.is_open())
        {
            string line, oline;
            ll line_num = 0;
            int num = 1;

            //code for reading from input file and converting each line into parsable strings
            while (getline(file, oline))
            {
              
                line = trimmed(oline);
                oinst[read].push_back(line);
                if (line.compare("") != 0)
                {
                    if (line.find(":") != string::npos)
                    {
                        if (line[line.length() - 1] == ':')
                        {
                            if (line.find(" ") != string::npos || line.find("\t") != string::npos)
                            {
                                throw runtime_error("Syntax Error at line " + to_string(num) + ": " + oline);
                            }
                            if (find(operators.begin(), operators.end(), line.substr(0, line.length() - 1)) != operators.end())
                            {
                                throw runtime_error("An operator cannot be a label. Syntax Error at line " + to_string(num) + ": " + oline);
                            }
                            if (labels[read].find(line.substr(0, line.length() - 1)) != labels[read].end())
                            {
                                throw runtime_error("Label is defined for the second time on line " + to_string(num));
                            }
                            labels[read].insert(pair<string, int>(line.substr(0, line.length() - 1), line_num));
                            num += 1;
                        }
                        else
                        {
                            size_t ind = line.find(':');
                            string line1 = line.substr(0, ind + 1);
                            string line2 = trimmed(line.substr(ind + 1));

                            if (line2.find(':') != string::npos)
                            {
                                throw runtime_error("Syntax Error at line " + to_string(num) + " in : " + oline);
                            }
                            if (line1.find(" ") != string::npos || line1.find("\t") != string::npos)
                            {
                                throw runtime_error("Syntax Error at line " + to_string(num) + ": " + oline);
                            }
                            if (find(operators.begin(), operators.end(), line1.substr(0, line1.length() - 1)) != operators.end())
                            {
                                throw runtime_error("An operator cannot be a label. Syntax Error at line " + to_string(num) + ": " + oline);
                            }
                            if (labels[read].find(line1.substr(0, line1.length() - 1)) != labels[read].end())
                            {
                                throw runtime_error("Label is defined for the second time on line " + to_string(num));
                            }
                            labels[read].insert(pair<string, int>(line1.substr(0, line1.length() - 1), line_num));
                            inst[read].push_back(pair<string, ll>(line2, num));
                            line_num += 1;
                            num += 1;
                        }
                    }
                    else
                    {
                    

                        inst[read].push_back(pair<string, ll>(line, num));
                        line_num += 1;
                        num++;
                    }
                }
                else
                {
                    num++;
                }
            }
        }
    }

    
    row_delay = stoi(argv[num_of_cores + 3]);
    column_delay = stoi(argv[num_of_cores + 4]);
}
    catch (exception e)
    {
        cout << "Invalid Arguments" << endl;
        return 0;
    }

    

    for (int num = 0; num < num_of_cores; num++)
    {
        inst_size[num] = inst[num].size();
        int i = 0;
        while (i != inst_size[num])
        {
            //parsing instruction
            parse(inst[num][i], num);
            i++;
        }
    }
    execute();

    return 0;
}