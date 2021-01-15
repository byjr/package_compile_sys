#ifndef _APP_SOURCES_MYBOX_SRC_ONE2MULTI_ONE2MULTI_H_
#define _APP_SOURCES_MYBOX_SRC_ONE2MULTI_ONE2MULTI_H_
#include <memory>
#include <future>
#include <mutex>
#include <unordered_map>
#include <cppUtils/base.h>
namespace cppUtils{
	class ProcessInterFace{
	public:		
		virtual bool onProcessExit(void* ptr) = 0;
	};
	typedef std::shared_ptr<DataBuffer> buf_ptr;
	typedef std::unordered_map<int,buf_ptr> bp_list;
	typedef std::shared_ptr<std::mutex> mtx_ptr;
	
	struct Multi2OnePar{
		bp_list outBufs;
		bp_list inBufs;
		size_t dumBytes;
		std::shared_ptr<std::mutex> mtxPtr;
	};
	class Multi2One{
		std::shared_ptr<Multi2OnePar> mPar;
		bool isReadyFlag,gotExitedFlag;
		bool isGracefulFlag;
		ProcessInterFace* mObserver;
		std::future<bool> mThread;
		bool setExit(bool graceful);
	public:
		void setObserver(ProcessInterFace* ob);
		void addInput(int i,buf_ptr& buf);
		void delInput(int i);
		bool stop();
		bool wait();		
		bool isReady();
		Multi2One(std::shared_ptr<Multi2OnePar>& par);
		~Multi2One();
	};
}
#endif//_APP_SOURCES_MYBOX_SRC_ONE2MULTI_ONE2MULTI_H_