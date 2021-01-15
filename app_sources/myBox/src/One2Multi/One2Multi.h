#ifndef _APP_SOURCES_MYBOX_SRC_ONE2MULTI_ONE2MULTI_H_
#define _APP_SOURCES_MYBOX_SRC_ONE2MULTI_ONE2MULTI_H_
#include <memory>
#include <future>
#include <mutex>
#include <unordered_map>
#include <cppUtils/base.h>
namespace cppUtils {
	class ProcessInterFace {
	public:
		virtual bool onProcessExit() = 0;
	};
	typedef std::shared_ptr<DataBuffer> buf_ptr;
	typedef std::unordered_map<int, buf_ptr> bp_list;
	typedef std::shared_ptr<std::mutex> mtx_ptr;

	struct One2MultiPar {
		bp_list outBufs;
		bp_list inBufs;
		size_t dumBytes;
		std::shared_ptr<std::mutex> mtxPtr;
	};
	class One2Multi {
		std::shared_ptr<One2MultiPar> mPar;
		bool mIsReadyFlag, mGotExitFlag;
		bool isGracefulFlag;
		std::vector<ProcessInterFace *> mObservers;
		std::future<bool> mThread;
		bool setExit(bool graceful);
	public:
		bool notifyExit();
		void addObserver(ProcessInterFace *ob);
		void addOutput(int i, buf_ptr &buf);
		void delOutput(int i);
		bool stop();
		bool wait();
		bool isReady();
		One2Multi(std::shared_ptr<One2MultiPar> &par);
		~One2Multi();
	};
}
#endif//_APP_SOURCES_MYBOX_SRC_ONE2MULTI_ONE2MULTI_H_