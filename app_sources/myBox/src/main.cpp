#include <lzUtils/base.h>
#include <getopt.h>
#include <string>
#include <unordered_map>
int getIp_main(int argc,char* argv[]);
int TupleTest_main(int argc,char* argv[]);
int UnicodeTool_main(int argc,char* argv[]);
int VirtualTest_main(int argc,char* argv[]);
int ChronoTest_main(int argc,char* argv[]);
int EpollTest_main(int argc,char* argv[]);
int TcpServTest_main(int argc,char* argv[]);
int MyClass_main(int argc,char* argv[]);

typedef int (*MainImpl_t)(int argc,char* argv[]);

int main(int argc,char* argv[]) {
	std::unordered_map<std::string,MainImpl_t> mainMap;	
	mainMap["getIp"] = getIp_main;
	mainMap["TupleTest"] = TupleTest_main;
	mainMap["UnicodeTool"] = UnicodeTool_main;
	mainMap["VirtualTest"] = VirtualTest_main;
	mainMap["ChronoTest"] = ChronoTest_main;
	mainMap["MyClass"] = MyClass_main;
	mainMap["EpollTest"] = EpollTest_main;
	mainMap["TcpServTest"] = TcpServTest_main;
	std::string func(get_last_name(argv[0]));
	
	if(argv[1] && func == STR(_PKG_NAME_)){
		argv_shift(argv,1);
	}
	MainImpl_t theMain = mainMap[argv[0]];
	if(theMain == nullptr){
		s_err("can't find tool:%s!",argv[0]);
		return -1;
	}
	return mainMap[argv[0]](argc,argv);
}