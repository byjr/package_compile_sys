// unordered_map::erase
#include <iostream>
#include <string>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <future>
#include <atomic>
#include <lzUtils/base.h>
#include <unistd.h>
using namespace std::chrono;
enum class SocktState {
	min = -1,
	rAble,
	wAble,
	closed,
	max
};
class MyClass {
	std::atomic<bool> mGotExitFlag, mIsReadyFlag;
	SocktState mSktSt;
	std::future<bool> mTrdA	;
	std::future<bool> mTrdB	;
	std::mutex mu;
	std::condition_variable cv;
public:
	bool waitState(SocktState state) {
		std::unique_lock<std::mutex> lk(mu);
		auto unint_dur = std::chrono::seconds(1);
		size_t count = std::chrono::seconds(15) / unint_dur;
		if(!count) count = 1;
		s_inf("count=%zu", count);
		mSktSt = SocktState::min;
		bool res = false;
		for(size_t i = 0; i < count; i++) {
			res = cv.wait_for(lk, unint_dur, [this, state]()->bool{return mSktSt == state;});
			s_inf("====i:%zu====res=%d", i, res ? 1 : 0);
			if(res) {
				break;
			}
		}
		return (!mGotExitFlag);
	}
	void notifyState(SocktState state) {
		std::unique_lock<std::mutex> lk(mu);
		mSktSt = state;
		lk.unlock();
		cv.notify_one();
	}
	MyClass():
		mIsReadyFlag{false},
		mGotExitFlag{false},
		mSktSt{SocktState::min} {
		mTrdA = std::async(std::launch::async, [this]()->bool{
			while(1) {
				inf_t("wait:%d", waitState(SocktState::rAble) ? 1 : 0);
			}
			return true;
		});
		mTrdB = std::async(std::launch::async, [this]()->bool {
			do {
				std::this_thread::sleep_for(std::chrono::seconds(3));
				notifyState(SocktState::min);
			} while(1);
			return true;
		});
		mIsReadyFlag = true;
	}
	bool isReady() {
		return mIsReadyFlag;
	}
	bool stop() {
		notifyState(SocktState::rAble);
	}
	bool wait() {
		mTrdB.get();
		mTrdA.get();
		return true;
	}
	~MyClass() {}
};
//==============全局静态资源===================
static std::shared_ptr<MyClass> g_AppManager;
static void stopInstence(int sig) {
	s_war("got signal:%d", sig);
	if(g_AppManager) {
		s_war("g_AppManager->stopAll!");
		g_AppManager->stop();
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
int MyClass_main(int argc, char *argv[]) {
	int opt  = -1;
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
	signal(SIGINT, &stopInstence);
	auto mPtr =	std::make_shared<MyClass>();
	if(!(mPtr && mPtr->isReady())) {
		s_errn();
		return -1;
	}
	g_AppManager = mPtr;
	mPtr->wait();
	return 0;
}
