#include <lzUtils/base.h>
#include <getopt.h>
#include <string>
#include <map>
#include <future>
#include <iostream>
using namespace std;
 
void map1(){
 
    map<int, string> mapStudent;
	pair<map<int, string>::iterator, bool> Insert_Pair;
	
    mapStudent.insert(pair<int, string>(1, "student_one"));
 
    mapStudent.insert(pair<int, string>(2, "student_two"));
 
    Insert_Pair=mapStudent.insert(pair<int, string>(3, "student_three"));
	cout << "ok:" << Insert_Pair.second << endl;
    map<int, string>::iterator iter;
 
    for(iter = mapStudent.begin(); iter != mapStudent.end(); iter++)
 
       cout<<iter->first<<' '<<iter->second<<endl;

}
int map2(){  
  
    map<int, string> mapStudent;  
  
    mapStudent.insert(map<int, string>::value_type (1, "student_one"));  
  
    mapStudent.insert(map<int, string>::value_type (2, "student_two"));  
  
    mapStudent.insert(map<int, string>::value_type (3, "student_three"));  
  
    map<int, string>::iterator iter;  
  
    for(iter = mapStudent.begin(); iter != mapStudent.end(); iter++)  
  
       cout<<iter->first<<' '<<iter->second<<endl;
  
}  
int map3() 
{  
    map<int, string> mapStudent;
  
    mapStudent[1] = "student_one";  
  
    mapStudent[2] = "student_two";  
  
    mapStudent[3] = "student_three"; 
	
	mapStudent[4] = "student_four"; 
  
	for(auto i:mapStudent){
		cout<<i.first<<' '<<i.second<<endl; 
	}
} 
int map3_1()  
  
{  
  
    map<string, string> mapStudent;
  
    mapStudent["one"] = "student_one";  
  
    mapStudent["two"] = "student_two";  
  
    mapStudent["three"] = "student_three"; 
	
	mapStudent["four"] = "student_four"; 
	
	for(auto i:mapStudent){
		cout<<i.first<<' '<<i.second<<endl; 
	}
} 
void future0(){
	std::future<int> future = std::async(std::launch::async, [](){ 
		std::this_thread::sleep_for(std::chrono::seconds(3));
		return 8;  
	});
	std::cout << "waiting...\n";
	std::future_status status;
	do {
		status = future.wait_for(std::chrono::seconds(1));
		if (status == std::future_status::deferred) {
			std::cout << "deferred\n";
		} else if (status == std::future_status::timeout) {
			std::cout << "timeout\n";
		} else if (status == std::future_status::ready) {
			std::cout << "ready!\n";
		}
	} while (status != std::future_status::ready); 
	std::cout << "result is " << future.get() << '\n';
}
#include <functional>

int hash0 ()
{
  char nts1[] = "Test11111111111";
  char nts2[] = "Test";
  std::string str1 (nts1);
  std::string str2 (nts2);

  std::hash<char*> ptr_hash;
  std::hash<std::string> str_hash;
	cout << "nts1 hash val:" << str_hash(str1) << "\n";
	cout << "nts2 hash val:" << str_hash(str2) << "\n";
  std::cout << "same hashes:\n" << std::boolalpha;
  std::cout << "nts1 and nts2: " << (ptr_hash(nts1)==ptr_hash(nts2)) << '\n';
  std::cout << "str1 and str2: " << (str_hash(str1)==str_hash(str2)) << '\n';

  return 0;
}
int main(int argc, char *argv[]) {
	std::string FuncStr;
	int opt = 0;
	while ((opt = getopt_long_only(argc, argv, "f:l:ph", NULL, NULL)) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		case 'f':
			FuncStr = optarg;
			break;
		default: /* '?' */
			s_err("no para!");
		}
	}
	if(0){		
	}else if(FuncStr == "map1"){
		map1();
	}else if(FuncStr == "map2"){
		map2();
	}else if(FuncStr == "map3"){
		map3();
	}else if(FuncStr == "map3_1"){
		map3_1();
	}else if(FuncStr == "future0"){
		future0();
	}else if(FuncStr == "hash0"){
		hash0();
	}
	return 0;
}