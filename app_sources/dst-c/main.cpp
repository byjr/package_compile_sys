#include <lzUtils/base.h>
#include <getopt.h>
#include <string>
#include <unordered_map>
int BigTopHeap_main(int argc,char* argv[]);
int LinkStack_main(int argc,char* argv[]);

void shift(char* argv[],size_t n){
	int i = 0;
	for(;argv[i+n];i++){
		argv[i]=argv[i+n];
	}
}

typedef int (*MainImpl_t)(int argc,char* argv[]);

int main(int argc,char* argv[]) {
	std::unordered_map<std::string,MainImpl_t> mainMap;
	mainMap["bth"] = BigTopHeap_main;
	mainMap["lst"] = LinkStack_main;
	std::string func(get_last_name(argv[0]));
	if(argv[1] && func == "dst-c"){
		shift(argv,1);
	}
	return mainMap[argv[0]](argc,argv);
}