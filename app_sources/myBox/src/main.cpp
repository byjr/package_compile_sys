#include <lzUtils/base.h>
#include <getopt.h>
#include <string>
#include <unordered_map>
int One2Multi_main(int argc,char* argv[]);
int Multi2One_main(int argc,char* argv[]);
int MyTmplate_main(int argc,char* argv[]);
int shrbd_main(int argc,char* argv[]);
int shrbc_main(int argc,char* argv[]);
int fifotc_main(int argc,char* argv[]);
int fifot_main(int argc,char* argv[]);
int inTcpSrv_main(int argc,char* argv[]);
int inTcpCli_main(int argc,char* argv[]);
int UnTcpClient_main(int argc,char* argv[]);
int UnTcpServer_main(int argc,char* argv[]);
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
	mainMap["o2m"] = One2Multi_main;
	mainMap["m2o"] = Multi2One_main;
	mainMap["shrbd"] = shrbd_main;
	mainMap["shrbc"] = shrbc_main;
	mainMap["fifotc"] = fifotc_main;
	mainMap["fifot"] = fifot_main;
	mainMap["inTcpSrv"] = inTcpSrv_main;
	mainMap["inTcpCli"] = inTcpCli_main;
	mainMap["UnTcpClient"] = UnTcpClient_main;
	mainMap["UnTcpServer"] = UnTcpServer_main;
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
		argc --;
	}
	MainImpl_t theMain = mainMap[argv[0]];
	if(theMain == nullptr){
		s_err("can't find tool:%s!",argv[0]);
		return -1;
	}
	return mainMap[argv[0]](argc,argv);
}