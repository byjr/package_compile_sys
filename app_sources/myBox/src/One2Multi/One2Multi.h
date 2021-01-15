#ifndef _APP_SOURCES_MYBOX_SRC_ONE2MULTI_ONE2MULTI_H_
#define _APP_SOURCES_MYBOX_SRC_ONE2MULTI_ONE2MULTI_H_
#include <memory>
#include <future>
#include <unordered_map>
#include <cppUtils/base.h>
namespace cppUtils{
	typedef std::shared_ptr<DataBuffer> buf_ptr;
	typedef std::unordered_map<int,buf_ptr> bp_list;
	typedef std::shared_ptr<std::mutex> mtx_ptr;
	struct One2MultiPar{
		buf_ptr inBuf;
		bp_list outBufs;
		std::mutex mu;
	};
	class One2Multi{
		std::shared_ptr<One2MultiPar> mPar;
		bool isReadyFlag,gotExitedFlag;
		bool isGracefulFlag;
		std::future<bool> mThread;
		void setExit(bool graceful);
	public:
		void addOutput(int idx,buf_ptr& buf);
		void delOutput(int idx);
		void stop();
		bool isReady();
		One2Multi(std::shared_ptr<One2MultiPar>& par);
		~One2Multi();
	};
}
#endif//_APP_SOURCES_MYBOX_SRC_ONE2MULTI_ONE2MULTI_H_