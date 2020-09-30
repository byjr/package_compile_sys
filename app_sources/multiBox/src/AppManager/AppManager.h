extern "C" {
#include <unistd.h>
#include <lzUtils/base.h>
#include <sys/types.h>
#include <sys/stat.h>
}
#include <thread>
#include <string>
#include <memory>
#include <vector>
#include <mutex>
enum class AppTaskState_t {
	min,//0
	started,
	failed,
	stoped,
	running,
	abort,
	max
};
class AppTaskState {
	std::vector<std::pair<AppTaskState_t, const char *>>	mVct;
	volatile AppTaskState_t mState;
public:
	AppTaskState(std::vector<std::pair<AppTaskState_t, const char *>> vct) {
		mVct = vct;
		mState = AppTaskState_t::min;
	}
	const char *getName(AppTaskState_t n = AppTaskState_t::max) {
		if(n == AppTaskState_t::max) {
			n = mState;
		}
		for(auto i : mVct) {
			if(i.first == n) {
				return i.second;
			}
		}
		return "max";
	}
	AppTaskState_t getByName(std::string name) {
		for(auto i : mVct) {
			if(name == i.second) {
				return i.first;
			}
		}
		return AppTaskState_t::max;
	}
	void set(AppTaskState_t n) {
		mState = n;
	}
	AppTaskState_t get() {
		return mState;
	}
};
#define AppTaskState_ITEM(x) {AppTaskState_t::x,#x}
std::vector<std::pair<AppTaskState_t, const char *>> g_AppTaskStateVct = {
	AppTaskState_ITEM(min),
	AppTaskState_ITEM(started),
	AppTaskState_ITEM(failed),
	AppTaskState_ITEM(stoped),
	AppTaskState_ITEM(running),
	AppTaskState_ITEM(abort),
	AppTaskState_ITEM(max),
};
class AppManagerTask {
	std::string cmdline;
	volatile size_t mMshutDownCount;
	std::mutex mMtx;
	std::shared_ptr<AppTaskState> mState;
	void AddFillTail(std::string &ctx) {
		size_t line_lenth = 64;
		ssize_t remSize = line_lenth - ctx.size();
		if(remSize < 0) {
			return ;
		}
		auto fill = new char[remSize];
		if(!fill) {
			return ;
		}
		fill[remSize - 1] = '\0';
		fill[remSize - 2] = '\n';
		memset(fill, '=', remSize - 2);
		ctx += fill;
		delete []fill;
	}
	bool isRunning() {
		if(is_process_has_locked(appName.data()) == 0) {
			return false;
		}
		return true;
	}
	void checkRunUp() {
		if(isRunning()) {
			if(mState->get() != AppTaskState_t::running) {
				mState->set(AppTaskState_t::running);
			}
		} else {
			if(mState->get() != AppTaskState_t::abort) {
				mState->set(AppTaskState_t::abort);
			}
		}
	}
public:
	std::string appName;
	AppManagerTask(const char *path,
				   const char *cmd = NULL
				  ) {
		appName = "";
		cmdline = "";
		mMshutDownCount = 0;
		mState = std::make_shared<AppTaskState>(g_AppTaskStateVct);
		if(!mState.get()) {
			s_err("failed to make_shared<AppTaskState>");
			return ;
		}
		appName = get_last_name(path);
		if(cmd) {
			cmdline += cmd;
			return ;
		}
		cmdline += path;
		cmdline += 	" 2>> /dev/null 1>> /dev/null &";
	}
	bool start() {
		std::lock_guard<std::mutex> lck (mMtx);
		s_inf("start %s ...", appName.data());
		if(isRunning()) {
			s_war("%s already running...", appName.data());
			mState->set(AppTaskState_t::started);
			return true;
		}
		int res = cmd_excute("%s", cmdline.data());
		if(res < 0) {
			mState->set(AppTaskState_t::failed);
			return false;
		}
		size_t retryTimes = 100;
		while(retryTimes --) {
			if(isRunning()) {
				mState->set(AppTaskState_t::started);
				return true;
			}
			usleep(10 * 1000);
		}
		mState->set(AppTaskState_t::failed);
		return false;
	}
	bool stop() {
		std::lock_guard<std::mutex> lck (mMtx);
		s_inf("stop %s ...", appName.data());
		cmd_excute("killall %s 2>> /dev/null 1>> /dev/null", appName.data());
		int retryTimes = 100;
		for(; isRunning() && retryTimes > 0; retryTimes --) {
			usleep(10 * 1000);
		}
		if(retryTimes <= 0) {
			s_war("force stop %s ...", appName.data());
			cmd_excute("killall -KILL %s 2>> /dev/null 1>> /dev/null", appName.data());
		}
		mState->set(AppTaskState_t::stoped);
		return true;
	}
	void monitor() {
		auto state = mState->get();
		if(state == AppTaskState_t::stoped) {
			return ;
		}
		if(state == AppTaskState_t::failed) {
			return ;
		}
		checkRunUp();
		if(mState->get() == AppTaskState_t::abort) {
			mMshutDownCount ++;
			start();
		}
	}
	size_t getSdCount() {
		return mMshutDownCount;
	}
	void show() {
		s_raw("\n");
		std::string ctx("== [");
		ctx += appName;
		ctx += "] ";
		AddFillTail(ctx);
		s_raw(ctx.data());
		s_raw("== mState:%s\n", mState->getName());
		s_raw("== mMshutDownCount:%d\n", mMshutDownCount);
		s_raw("== cmdline:%s\n", cmdline.data());
		ctx = "";
		AddFillTail(ctx);
		s_raw(ctx.data());
		s_raw("\n");
	}
};