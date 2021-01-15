#include <vector>
#include <fstream>
#include <mutex>
#include "Multi2One.h"
#include <lzUtils/base.h>
using namespace cppUtils;
namespace Multi2One {
#define DATA_UNIT_MAX_BYTES (121*100)
	class InputTask;
	class OutputTask;
	typedef std::vector<std::shared_ptr<InputTask>> itask_vct;
	typedef std::vector<std::shared_ptr<OutputTask>> otask_vct;

	//==============数据输入任务类===================
	struct InputTaskPar {
		int id;
		const char *path;
		buf_ptr buf;
	};
	class InputTask {
		std::shared_ptr<InputTaskPar> mPar;
		bool mIsReadyFlag, mGotExitFlag;
		bool isGracefulFlag;
		std::vector<ProcessInterFace *> mObservers;
		std::ifstream mFs;
		std::future<bool> mTrd;
		bool setExit(bool graceful) {
			isGracefulFlag = graceful;
			mGotExitFlag = true;
			return true;
		}
	public:
		InputTask(std::shared_ptr<InputTaskPar> &par):
			mPar			{	par		},
			mIsReadyFlag	{	false	},
			mGotExitFlag	{	false	},
			isGracefulFlag	{	true	} {
			mTrd = std::async(std::launch::async, [this]()->bool{
				mFs.open(mPar->path, std::ios::binary);
				if(!mFs.is_open()) {
					show_errno(0, open);
					return false;
				}
				for(; !mGotExitFlag;) {
					auto data = std::make_shared<data_unit>(DATA_UNIT_MAX_BYTES);
					if(!data) {
						s_err("");
						return false;
					}
					mFs.read(data->data(), data->size());
					if(mFs.gcount() <= 0 ) {
						if(mFs.eof()) {
							s_war("InputTask read complete!");
							setExit(true);
							break;
						}
						show_errno(0, read);
						setExit(false);
						break;
					}
					data->resize(mFs.gcount());
					mPar->buf->wbWrite(data);
					std::this_thread::yield();
				}
				mFs.close();
				mIsReadyFlag = false;
				return isGracefulFlag;
			});
			mIsReadyFlag = true;
		}
		bool isReady() {
			return mIsReadyFlag;
		}
		bool stop() {
			s_war("stop input ...");
			mGotExitFlag = true;
			return true;
		}
		bool wait() {
			return mTrd.get();
		}
		void addObserver(ProcessInterFace *ob) {
			mObservers.push_back(ob);
		}
		bool notifyExit() {
			for(auto i : mObservers) {
				i->onProcessExit();
			}
			return true;
		}
	};

	//==============数据输出任务类=========================================================
	struct OutputTaskPar {
		int id;
		const char *path;
		buf_ptr buf;
	};
	class OutputTask {
		std::shared_ptr<OutputTaskPar> mPar;
		bool mIsReadyFlag, mGotExitFlag;
		bool isGracefulFlag;
		std::vector<ProcessInterFace *> mObservers;
		std::ofstream mFs;
		std::future<bool> mTrd;
		bool setExit(bool graceful) {
			isGracefulFlag = graceful;
			mGotExitFlag = true;
			return true;
		}
	public:
		OutputTask(std::shared_ptr<OutputTaskPar> &par):
			mPar			{	par		},
			mIsReadyFlag		{	false	},
			mGotExitFlag	{	false	},
			isGracefulFlag	{	true	} {
			mTrd = std::async(std::launch::async, [this]()->bool{
				mFs.open(mPar->path, std::ios::binary);
				if(!mFs.is_open()) {
					show_errno(0, open);
					return false;
				}
				data_ptr data;
				for(; !mGotExitFlag;) {
					// std::this_thread::yield();
					if(!mPar->buf->wbRead(data)) {
						setExit(true);
						break;
					}
					mFs.write(data->data(), data->size());
					if(!mFs.good()) {
						show_errno(0, write);
						setExit(false);
						break;
					}
				}
				mFs.close();
				mIsReadyFlag = false;
				return isGracefulFlag;
			});
			mIsReadyFlag = true;
		}
		bool isReady() {
			return mIsReadyFlag;
		}
		bool stop() {
			s_war("stop output ...");
			mGotExitFlag = true;
			mPar->buf->stop();
			return true;
		}
		bool wait() {
			return mTrd.get();
		}
		void addObserver(ProcessInterFace *ob) {
			mObservers.push_back(ob);
		}
		bool notifyExit() {
			for(auto i : mObservers) {
				i->onProcessExit();
			}
			return true;
		}
	};

	//==============应用管理类===================
	struct AppManagerPar {
		std::shared_ptr<cppUtils::Multi2One> m2o;
		std::shared_ptr<itask_vct> itasks;
		std::shared_ptr<otask_vct> otasks;
	};
	class AppManager: public ProcessInterFace {
		std::shared_ptr<AppManagerPar> mPar;
	public:
		AppManager(std::shared_ptr<AppManagerPar> &par):
			mPar{par} {}
		bool onProcessExit() {
			return stopAll();
		}
		bool stopAll() {
			std::vector<bool> res(1 + mPar->itasks->size() + mPar->otasks->size(), true);
			size_t resIdx = 0;
			if(mPar->m2o->isReady()) {
				res[resIdx] = mPar->m2o->stop();
				if(!res[resIdx]) {
					s_err("res[%d] == false", resIdx);
				}
				resIdx ++;
			}
			for(auto i : * (mPar->itasks)) {
				if(i->isReady()) {
					res[resIdx] = i->stop();
					if(!res[resIdx]) {
						s_err("res[%d] == false", resIdx);
					}
					resIdx ++;
				}
			}
			for(auto o : * (mPar->otasks)) {
				if(o->isReady()) {
					res[resIdx] = o->stop();
					if(!res[resIdx]) {
						s_err("res[%d] == false", resIdx);
					}
					resIdx++;
				}
			}
			for(auto i : res) {
				if(i == false) {
					return false;
				}
			}
			return true;
		}
	};
}
using namespace Multi2One;
//==============全局静态资源===================
static std::shared_ptr<AppManager> g_AppManager;
static void stopInstence(int sig) {
	s_war("got signal:%d", sig);
	if(g_AppManager) {
		s_war("g_AppManager->stopAll!");
		g_AppManager->stopAll();
	}
}
static int help_info(int argc, char *argv[]) {
	s_err("%s help:", get_last_name(argv[0]));
	s_err("\t-i [input path]");
	s_err("\t-o [output url]");
	s_err("\t-l [logLvCtrl]");
	s_err("\t-p [logPath]");
	s_err("\t-h show help");
	return 0;
}
int Multi2One_main(int argc, char *argv[]) {
	//==============信号注册模块===================
	signal(SIGINT, &stopInstence);

	//==============参数处理模块===================
	int opt  = -1;
	auto iParListPtr = std::make_shared<std::vector<std::shared_ptr<InputTaskPar>>>();
	if(!iParListPtr) {
		s_err("");
		return -1;
	}
	auto oParListPtr = std::make_shared<std::vector<std::shared_ptr<OutputTaskPar>>>();
	if(!oParListPtr) {
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
		case 'i': {
			auto i = std::make_shared<InputTaskPar>();
			i->id = iParListPtr->size();
			i->path = optarg;

			iParListPtr->push_back(i);
			break;
		}
		case 'o': {
			auto o = std::make_shared<OutputTaskPar>();
			o->id = oParListPtr->size();
			o->path = optarg;
			oParListPtr->push_back(o);
			break;
		}
		default: /* '?' */
			return help_info(argc, argv);
		}
	}
	if(iParListPtr->size() <= 0) {
		s_err("iParListPtr == 0");
		return -1;
	}
	if(oParListPtr->size() != 1) {
		s_err("oParListPtr != 1");
		return -1;
	}
	auto main_loop = [&iParListPtr, &oParListPtr]() {
		for(auto i : *iParListPtr) {
			i->buf = std::make_shared<DataBuffer>(3);
		}
		for(auto o : *oParListPtr) {
			o->buf = std::make_shared<DataBuffer>(3);
		}
		//==============启动数据输出任务模块===================
		auto par = std::make_shared<Multi2OnePar>();
		auto oTasks = std::make_shared<otask_vct>();
		auto oMtxPtr = std::make_shared<std::mutex>();
		for(auto o : *oParListPtr) {
			par->outBufs[o->id] = o->buf;
			auto oTask = std::make_shared<OutputTask>(o);
			if(!(oTask && oTask->isReady())) {
				s_err("oTask create failed!");
				return -1;
			}
			oTasks->push_back(oTask);
		}

		//==============启动数据输入任务模块===================
		auto iTasks = std::make_shared<itask_vct>();
		auto iMtxPtr = std::make_shared<std::mutex>();
		for(auto i : *iParListPtr) {
			par->inBufs[i->id] = i->buf;
			auto iTask = std::make_shared<InputTask>(i);
			if(!(iTask && iTask->isReady())) {
				s_err("iTask create failed!");
				return -1;
			}
			iTasks->push_back(iTask);
		}

		//==============启动主任务模块===================
		par->dumBytes = DATA_UNIT_MAX_BYTES;
		par->mtxPtr = iMtxPtr;
		auto ptr = std::make_shared<cppUtils::Multi2One>(par);
		if(!(ptr && ptr->isReady())) {
			s_err("new Multi2One failed!");
			return -1;
		}

		//==============启动应用管理模块===================
		auto mgrPar = std::make_shared<AppManagerPar>();
		mgrPar->m2o = ptr;
		mgrPar->itasks = iTasks;
		mgrPar->otasks = oTasks;
		auto mgr = std::make_shared<AppManager>(mgrPar);
		if(!mgr) {
			s_err("AppManager create failed!");
			return -1;
		}
		g_AppManager = mgr;

		ptr->addObserver(mgr.get());
		for(auto i : *iTasks) {
			i->addObserver(mgr.get());
		}
		for(auto o : *oTasks) {
			o->addObserver(mgr.get());
		}

		//==============等待任务退出===================
		if(!ptr->wait()) {
			s_err("");
			return -1;
		}
		for(auto i : (*iTasks)) {
			if(!i->wait()) {
				s_err("");
				return -1;
			}
		}
		for(auto o : (*oTasks)) {
			if(!o->wait()) {
				s_err("");
				return -1;
			}
		}
		return 0;
	};
	int res = 0;
	for(size_t i = 0;; i++) {
		s_war("run %zu times ...", i);
		res =  main_loop();
		if(res < 0) {
			s_err("main_loop run failed!");
			return -1;
		}
	}
	return 0;
}