#include <iostream>
#include <sstream>
#include <vector>
#include <string> 
using namespace std;


const string WS = " \t";

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
        return v1[0] < v1[1];
    }
}