// c++14
#include <iostream>
#include <fstream>
//  remove_if, sort, min, max, reverse,  merge, binary_search, is_sorted, unique, replace_if, count, push_heap, pop_heap, make_heap
#include <algorithm>
#include <vector>

// .push, .pop, .front, .back
#include <queue>
// .front, .back, .push_back, push_front, pop_back, pop_front, .at (slow)
#include <deque>
// map<string, int> m; m["x"] = 2; auto it = m.find("x"); it != m.end(); it->second; m.erase(it); m.erase("x");
#include <map>
// can take custom binary cmp function, 
// set<string> a; a.insert("f"); set<string>iterator it = a.find("f); it != a.end(); *it; a.erase("f");
#include <set> 
#include <cstdio> // printf, scanf // scanf("%d", &i); // read integer
#include <stdlib.h>
#include <assert.h> // assert
#include <utility> // pair, make_pair
#include <functional>
#include <string> 
#include <stack> // .pop .push .size .top .empty
#include <math.h> // cos, sin, tan, acos, asin, atan2, exp, log, log10, log2, pow, sqrt, hypot, cell, floor, round, fabs, abs

using namespace std;

#define ll long long int
#define fo(i,n) for(int i=0;i<n;i++)
#define fab(i,a,b) for(int i=a;i<b;i++)
#define rfo(i,n) for(int i=n;i>0;i--)

//pair
#define pii pair<int,int>
#define F first
#define S second

// vector
#define pb(x) push_back(x)




typedef pair<int, int> PII; // first, second
typedef vector<char> VC;
typedef vector<VC> VVC;
typedef vector<int> VI;
typedef vector<string> VS;
typedef vector<VI> VVI;
typedef map<int,int> MPII;



// build with: g++ main.cpp -o main -std=c++14
//./main > test.out
vector<int>t(100000,0);
// t0=0-2000, t1=2000*1-2000*2 , t2= 2000*2 .......   
// vector<int>
vector<string> inst;
ll inst_size;
vector<int>registers(32,0);
//map for memory
MPII memory;

// functions for map reference
// begin() – Returns an iterator to the first element in the map
// end() – Returns an iterator to the theoretical element that follows last element in the map
// size() – Returns the number of elements in the map
// max_size() – Returns the maximum number of elements that the map can hold
// empty() – Returns whether the map is empty
// pair insert(keyvalue, mapvalue) – Adds a new element to the map
// erase(iterator position) – Removes the element at the position pointed by the iterator
// erase(const g)– Removes the key value ‘g’ from the map
// clear() – Removes all the elements from the map



//write fucntions here




int main() {
    // read input
     // read input
    ifstream file("test.in");
    if (file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            inst.push_back(line);
            // cout << line << endl;
        }
    }
    inst_size=inst.size();

    int i=0;
    while(i!=inst_size){
        string cur_inst=inst[i];

        //perform various function according to instructionn





        i++; // OR set i according to jump 

    }
   
    
    
    return 0;
}