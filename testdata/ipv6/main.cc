#include <cstdio>
#include <fstream>
#include <cstring>
#include <string>
#include <map>

int main(){
    // parsing the flow entries in bgptable.txt, and categorizes with different method
    std::string line;
    std::map<int,int> table_entry_count;
    std::ifstream fin("bgptable_ipv6_uniq.txt");

    while(std::getline(fin,line)){
        char *value = std::strtok(const_cast<char*>(line.c_str()),"/");
        char *key = std::strtok(NULL,"/");
        // printf("%s , %s\n",key,value);
        table_entry_count[std::atoi(key)]++;
    }

    // Get total 
    double total_table_entries=0.0;
    for(auto&it: table_entry_count){
        total_table_entries+=it.second;
    }
    for(auto&it: table_entry_count){
        printf("[ %d ]: %d , %lf (%lf%%)\n",it.first,it.second,it.second/total_table_entries,100*it.second/total_table_entries);
    }


    return 0;
}