// Example program
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <vector>

std::vector<std::tuple<bool,int,const char*>> my_tuple_vector{
	std::tuple<bool,int,const char*>{true,1,"aaa"},
	std::tuple<bool,int,const char*>{false,2,"bbb"},
	std::tuple<bool,int,const char*>{true,3,"ccc"}
};

int TupleTest_main(int argc,char* argv[]){
    for(auto i:my_tuple_vector){
        std::cout << std::get<0>(i) << ":" << std::get<1>(i) << ":" << std::get<2>(i) << "\n";
    }
    return 0;
}
