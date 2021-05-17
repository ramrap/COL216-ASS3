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
    registers.insert(make_pair("$at", j++));
    for(int i=0; i < 2; i++){
        string s = "$v";
        s += to_string(i);
        registers.insert(make_pair(s, j++));
    }

    for (int i = 0; i < 4; i++)
    {
        string s = "$a";
        s += to_string(i);
        registers.insert(make_pair(s, j++));
    }

    for (int i = 0; i < 8; i++)
    {
        string s = "$t";
        s += to_string(i);
        registers.insert(make_pair(s, j++));
    }

    for (int i = 0; i < 8; i++)
    {
        string s = "$s";
        s += to_string(i);
        registers.insert(make_pair(s, j++));
    }
    
    for (int i = 8; i <= 9; i++)
    {
        string s = "$t";
        s += to_string(i);
        registers.insert(make_pair(s, j++));
    }

    for(int i=0; i < 2; i++){
        string s = "$k";
        s += to_string(i);
        registers.insert(make_pair(s, j++));
    }

    registers.insert(make_pair("$gp", j++));
    registers.insert(make_pair("$sp", j++));
    registers.insert(make_pair("$fp", j++));
    registers.insert(make_pair("$ra", j++));

    for (int i = 0; i < num_of_cores; i++)
    {
        registers_core[i][registers["$sp"]] = max_size * 4 - 4;
        // cout<<registers_core[i][registers["$sp"]]<<endl;
    }

    //allocating size to memory Array
    buffer_row = new int32_t[columns];
    memory = new int32_t *[rows];
    memory_offset = floor((rows*1.0)/num_of_cores)*1024;

    for (int i = 0; i < rows; i++)
    {
        memory[i] = new int32_t[columns];
    }
    memset(queue_sizes, 0, sizeof(queue_sizes));
    memset(assigned_rows, -1, sizeof(assigned_rows));
    memset(first_req, 0, sizeof(first_req));
    memset(assigned_queues, -1, sizeof(assigned_queues));
    memset(blocks, 0, sizeof(blocks));
    memset(lw_blocks, 0, sizeof(lw_blocks));
    memset(lw_qs, 0, sizeof(lw_qs));

    blocked_inst = new blocked_regs[num_of_cores];
    curr_write = new write_delay[num_of_cores];
    write_cycles = new int[num_of_cores];
    struct blocked_regs b = {0,0,0,0, false};
    
    struct position pos ={false, -1, -1};
    loadReqs = new position *[num_of_cores];
    for(int i=0;i<num_of_cores;i++){
        loadReqs[i] = new position[32];
    }
    for(int i=0; i< num_of_cores; i++){
        write_cycles[i] = -1;
        blocked_inst[i] = b;
        curr_write[i] = null_write;
        for(int j = 0; j < 32; j++){
            loadReqs[i][j] = pos;
        }
    }

    total_cores = num_of_cores;

    for(int i =0; i<8; i++){
        for(int j = 0; j<max_queue_size; j++){
            request_queue[i][j] = null_req;
        }
    }
    update_copies();

}

// main function
int main(int argc, char *argv[])
{

   

    // read file name from CLI
   

    
    
    try{
        int a;
        int i=0;
        string str;
    
        // cin.ignore();
        istream& ignore (streamsize n = 1, int delim = EOF);
        getline(std::cin, str);
    
        istringstream ss(str);
  
        string word;
        while(ss >> word){
            // cout<<"inside while loop";
            a=stoi(word);
            // cout<<a<<" ";
            if(i==0){
                num_of_cores=a;
            }
            else if(i==1){
                simulation_time=a;
            }
            else if(i==2){
                row_delay=a;

            }
            else if(i==3){
                column_delay=a;
            }
            i++;
        }
        cout<<endl;
        // cout<<i<<" <= \n";
        if(i!=2 && i!=4){
            throw runtime_error("INVALID ARGUMENTS");
        }
        
       
    }
    catch(exception e){
        throw runtime_error("INVALID ARGUMENTS");
        return 0;
    }
     initialise_memory();

    vector<string>file_name(num_of_cores,"");

    for(int i=0;i<num_of_cores;i++){
       
        cin>>file_name[i];
        
    }

    for (int read = 0; read < num_of_cores; read++)
    {

       cout<<file_name[read]<<" \n";
        ifstream file(file_name[read]);
        if (!file.is_open())
        {
            cout << "Error: Provide Valid File Path For " << file_name[read] << endl;
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
                // cout<<oline<<endl;
                line = trimmed(oline);
                // cout<<"trimmed => "<<line<<endl;
                oinst[read].push_back(line);

                cout<<(line.compare("") != 0)<<endl;
                // cout<<"completed \n";
                
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