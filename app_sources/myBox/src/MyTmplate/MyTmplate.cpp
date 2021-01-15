#include "MyTmplate.h"
namespace cppUtils{
	bool MyTmplate::isReady(){
		return isReadyFlag;
	}
	MyTmplate::MyTmplate(std::shared_ptr<MyTmplatePar>& par):
		mPar			{	par		},
		isReadyFlag		{	false	},
		gotExitedFlag	{	false	}{

	}
	MyTmplate::~MyTmplate(){
		
	}
}
using namespace cppUtils;
#include <lzUtils/base.h>
static int help_info(int argc, char *argv[]) {
	s_err("%s help:", get_last_name(argv[0]));
	s_err("\t-i [input path]");
	s_err("\t-o [output url]");
	s_err("\t-l [logLvCtrl]");
	s_err("\t-p [logPath]");
	s_err("\t-h show help");
	return 0;
}
int MyTmplate_main(int argc,char* argv[]){
	int opt  =-1;
	while ((opt = getopt(argc, argv, "l:p:h")) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		default: /* '?' */
			return help_info(argc, argv);
		}
	}
	auto par = std::make_shared<MyTmplatePar>();
	auto ptr = std::make_shared<MyTmplate>(par);
	if(!(ptr && ptr->isReady())){
		s_err("new MyTmplate failed!");
		return -1;
	}
    return 0;
}
