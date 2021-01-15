#include "Multi2One.h"
#include <vector>
#include <fstream>
#include <mutex>
namespace cppUtils{
	void Multi2One::setObserver(ProcessInterFace* ob){
		mObserver = ob;
	}
	void Multi2One::addInput(int i,buf_ptr& buf){
		std::unique_lock<std::mutex> lk(*(mPar->mtxPtr));
		mPar->inBufs[i]=buf;
	}
	void Multi2One::delInput(int i){
		std::unique_lock<std::mutex> lk(*(mPar->mtxPtr));
		mPar->inBufs.erase(i);
	}
	bool Multi2One::setExit(bool graceful){
		isGracefulFlag = graceful;
		gotExitedFlag = true;
		return mObserver?
			mObserver->onProcessExit(this):false;
	}
	bool Multi2One::stop(){
		s_war("stop m2o ...");
		gotExitedFlag = true;
		for(auto i:mPar->inBufs){
			i.second->stop();
		}		
		return true;
	}
	bool Multi2One::wait(){
		return mThread.get();
	}
	bool Multi2One::isReady(){
		return isReadyFlag;
	}
	Multi2One::Multi2One(std::shared_ptr<Multi2OnePar>& par):
		mPar			{	par		},
		isReadyFlag		{	false	},
		gotExitedFlag	{	false	},
		isGracefulFlag	{	true	},
		mObserver		{   nullptr }{
		mThread = std::async(std::launch::async,[this]()->bool{
			data_ptr inData,outData;
			bool res = true;
			for(;!gotExitedFlag;){
				std::unique_lock<std::mutex> lk(*(mPar->mtxPtr));
				outData = std::make_shared<data_unit>(mPar->dumBytes * mPar->inBufs.size());
				size_t count =  0;
				for(auto i:mPar->inBufs){
					if(gotExitedFlag) break;
					if(!i.second) continue;
					if(!i.second->wbRead(inData)){
						setExit(true);
						break;
					}
					memcpy(outData->data()+count,inData->data(),inData->size());
					count += inData->size();
				}				
				if(gotExitedFlag) break;
				lk.unlock();
				outData->resize(count);
				res = mPar->outBufs.begin()->second->wbWrite(outData);
				std::this_thread::yield();
				lk.lock();
				if(!res){
					s_err("wbWrite");
					setExit(res);
					break;
				}
			}
			return isGracefulFlag;
		});
		isReadyFlag = true;
	}
	Multi2One::~Multi2One(){}	
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
#define DATA_UNIT_MAX_BYTES (1024*4)
class InputTask;
class OutputTask;
typedef std::vector<std::shared_ptr<InputTask>> itask_vct;
typedef std::vector<std::shared_ptr<OutputTask>> otask_vct;
struct InputTaskPar{
	int id;
	const char* path;
	buf_ptr buf;	
};
class InputTask{
	std::shared_ptr<InputTaskPar> mPar;
	bool isReadyFlag,gotExitedFlag;
	bool isGracefulFlag;
	ProcessInterFace* mObserver;
	std::ifstream mFs;
	std::future<bool> mTrd;
	bool setExit(bool graceful){
		isGracefulFlag = graceful;
		stop();		
		return mObserver?
			mObserver->onProcessExit(this):false;
	}
public:	
	InputTask(std::shared_ptr<InputTaskPar>& par):
			mPar{par},
			isReadyFlag		{	false	},
			gotExitedFlag	{	false	},
			isGracefulFlag	{	true	},
			mObserver		{	nullptr }{
		mTrd = std::async(std::launch::async,[this]()->bool{
			mFs.open(mPar->path,std::ios::binary);
			if(!mFs.is_open()){
				show_errno(0,open);
				return false;
			}
			for(;!gotExitedFlag;){
				auto data = std::make_shared<data_unit>(DATA_UNIT_MAX_BYTES);
				if(!data){
					s_err("");
					return false;
				}
				mFs.read(data->data(),data->size());
				if(mFs.gcount() <= 0 ){
					if(mFs.eof()){
						s_war("read complete!");
						setExit(true);
						break;
					}
					show_errno(0,read);
					setExit(false);
					break;
				}
				mPar->buf->wbWrite(data);
				std::this_thread::yield();
			}
			mFs.close();
			return isGracefulFlag;			
		});	
		isReadyFlag = true;
	}
	bool isReady(){
		return isReadyFlag;
	}	
	bool stop(){
		s_war("stop input ...");
		gotExitedFlag = true;		
		return true;
	}
	bool wait(){
		return mTrd.get();
	}
	void setObserver(ProcessInterFace* ob){
		mObserver = ob;
	}	
};
struct OutputTaskPar{
	int id;
	const char* path;
	buf_ptr buf;
};
class OutputTask{
	std::shared_ptr<OutputTaskPar> mPar;
	bool isReadyFlag,gotExitedFlag;
	bool isGracefulFlag;
	ProcessInterFace* mObserver;
	std::ofstream mFs;
	std::future<bool> mTrd;
	bool setExit(bool graceful){
		isGracefulFlag = graceful;
		stop();
		return mObserver?
			mObserver->onProcessExit(this):false;
	}	
public:	
	OutputTask(std::shared_ptr<OutputTaskPar>& par):
			mPar{par},
			isReadyFlag		{	false	},
			gotExitedFlag	{	false	},
			isGracefulFlag	{	true	},
			mObserver		{	nullptr }{
		mTrd = std::async(std::launch::async,[this]()->bool{
			mFs.open(mPar->path,std::ios::binary);
			if(!mFs.is_open()){
				show_errno(0,open);
				return false;
			}			
			data_ptr data;
			for(;!gotExitedFlag;){
				std::this_thread::yield();
				if(!mPar->buf->wbRead(data)){
					setExit(true);
					break;
				}
				mFs.write(data->data(),data->size());
				if(!mFs.good()){
					show_errno(0,write);
					setExit(false);
					break;
				}
			}
			mFs.close();
			return isGracefulFlag;
		});		
		isReadyFlag = true;
	}
	bool isReady(){
		return isReadyFlag;
	}
	bool stop(){
		s_war("stop output ...");
		gotExitedFlag = true;
		mPar->buf->stop();		
		return true;
	}
	bool wait(){
		return mTrd.get();
	}
	void setObserver(ProcessInterFace* ob){
		mObserver = ob;
	}	
};
struct ProcessManagerPar{
	std::shared_ptr<Multi2One> m2o;
	std::shared_ptr<itask_vct> itasks;
	std::shared_ptr<otask_vct> otasks;
};
class ProcessManager:public ProcessInterFace{
	std::shared_ptr<ProcessManagerPar> mPar;
public:	
	ProcessManager(std::shared_ptr<ProcessManagerPar>& par):
			mPar{par}{}			
	bool onProcessExit(void* ptr){
		std::vector<bool> res(1 + mPar->itasks->size() + mPar->otasks->size(),true);
		size_t resIdx = 0;
		if(ptr != mPar->m2o.get()){
			res[resIdx] = mPar->m2o->stop();
			if(!res[resIdx]){
				s_err("res[%d] == false",resIdx);
			}
			resIdx ++;			
		}
		for(auto i:*(mPar->itasks)){
			if(ptr != i.get()){
				res[resIdx] = i->stop();
				if(!res[resIdx]){
					s_err("res[%d] == false",resIdx);
				}
				resIdx ++;
			}
		}
		for(auto o:*(mPar->otasks)){
			if(ptr != o.get()){
				res[resIdx] = o->stop();
				if(!res[resIdx]){
					s_err("res[%d] == false",resIdx);
				}
				resIdx++;
			}
		}
		for(auto i:res){
			if(i == false){
				return false;
			}
		}
		return true;
	}
};
int Multi2One_main(int argc,char* argv[]){
	int opt  =-1;
	auto iParListPtr = std::make_shared<std::vector<std::shared_ptr<InputTaskPar>>>();
	if(!iParListPtr){
		s_err("");
		return -1;
	}
	auto oParListPtr = std::make_shared<std::vector<std::shared_ptr<OutputTaskPar>>>();	
	if(!oParListPtr){
		s_err("");
		return -1;
	}	
	while ((opt = getopt(argc, argv, "o:i:l:p:h")) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		case 'i':{
			auto i = std::make_shared<InputTaskPar>();
			i->id = iParListPtr->size();
			i->path = optarg;
			i->buf = std::make_shared<DataBuffer>(3);
			iParListPtr->push_back(i);
			break;			
		}case 'o':{
			auto o = std::make_shared<OutputTaskPar>();
			o->id = oParListPtr->size();
			o->path = optarg;
			o->buf = std::make_shared<DataBuffer>(3);
			oParListPtr->push_back(o);
			break;		
		}default: /* '?' */
			return help_info(argc, argv);
		}
	}
	if(iParListPtr->size() <= 0){
		s_err("iParListPtr == 0");
		return -1;
	}
	if(oParListPtr->size() != 1){
		s_err("oParListPtr != 1");
		return -1;
	}
	auto par = std::make_shared<Multi2OnePar>();
	auto oTasks = std::make_shared<otask_vct>();
	auto oMtxPtr = std::make_shared<std::mutex>();
	for(auto o:*oParListPtr){
		par->outBufs[o->id] = o->buf;
		auto oTask = std::make_shared<OutputTask>(o);
		if(!(oTask && oTask->isReady())){
			s_err("oTask create failed!");
			return -1;
		}
		oTasks->push_back(oTask);
	}
	auto iTasks = std::make_shared<itask_vct>();	
	auto iMtxPtr = std::make_shared<std::mutex>();
	for(auto i:*iParListPtr){
		par->inBufs[i->id] = i->buf;
		auto iTask = std::make_shared<InputTask>(i);
		if(!(iTask && iTask->isReady())){
			s_err("iTask create failed!");
			return -1;
		}
		iTasks->push_back(iTask);
	}	
	par->dumBytes = DATA_UNIT_MAX_BYTES;
	par->mtxPtr = iMtxPtr;
	auto ptr = std::make_shared<Multi2One>(par);
	if(!(ptr && ptr->isReady())){
		s_err("new Multi2One failed!");
		return -1;
	}
	auto mgrPar = std::make_shared<ProcessManagerPar>();
	mgrPar->m2o = ptr;
	mgrPar->itasks = iTasks;
	mgrPar->otasks = oTasks;
	auto mgr = std::make_shared<ProcessManager>(mgrPar);
	if(!mgr){
		s_err("ProcessManager create failed!");
		return -1;
	}
	
	ptr->setObserver(mgr.get());
	for(auto i:*iTasks){
		i->setObserver(mgr.get());
	}
	for(auto o:*oTasks){
		o->setObserver(mgr.get());
	}
	
	if(!ptr->wait()){
		s_err("");
		return -1;
	}
	for(auto i:(*iTasks)){
		if(!i->wait()){
			s_err("");
			return -1;
		}
	}
	for(auto o:(*oTasks)){
		if(!o->wait()){
			s_err("");
			return -1;
		}
	}
    return 0;
}
