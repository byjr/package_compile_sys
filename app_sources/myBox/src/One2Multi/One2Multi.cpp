#include "One2Multi.h"
namespace cppUtils{
	void One2Multi::addOutput(int idx,buf_ptr& buf){
		std::unique_lock<std::mutex> lk(mPar->mu);
		mPar->outBufs[idx]=buf;
	}
	void One2Multi::delOutput(int idx){
		std::unique_lock<std::mutex> lk(mPar->mu);
		mPar->outBufs.erase(idx);
	}
	void One2Multi::setExit(bool graceful){
		isGracefulFlag = graceful;
		gotExitedFlag = true;
	}
	void One2Multi::stop(){
		gotExitedFlag = true;
	}
	bool One2Multi::isReady(){
		return isReadyFlag;
	}
	One2Multi::One2Multi(std::shared_ptr<One2MultiPar>& par):
		mPar			{	par		},
		isReadyFlag		{	false	},
		gotExitedFlag	{	false	},
		isGracefulFlag	{	true	}{
		mThread = std::async([this]()->bool{
			data_ptr inData,outData;
			bool res = true;
			for(;!gotExitedFlag;){
				if(!mPar->inBuf->wbRead(inData)){
					s_err("wbRead");
					break;
				}
				if(gotExitedFlag) break;
				{
					std::unique_lock<std::mutex> lk(mPar->mu);
					for(auto o:mPar->outBufs){
						if(gotExitedFlag) break;
						if(!o.second) continue;
						outData = std::make_shared<data_unit>(inData);
						if(!o.second->crcWrite(outData)){
							s_err("wbWrite");
							continue;
						}
					}					
				}
			}
			return isGracefulFlag;
		});
		isReadyFlag = true;
	}
	One2Multi::~One2Multi(){}
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
int One2Multi_main(int argc,char* argv[]){
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
	auto par = std::make_shared<One2MultiPar>();	
	auto ptr = std::make_shared<One2Multi>(par);
	if(!(ptr && ptr->isReady())){
		s_err("new One2Multi failed!");
		return -1;
	}
    return 0;
}
