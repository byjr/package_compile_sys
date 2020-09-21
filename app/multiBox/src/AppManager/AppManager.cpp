#include "AppManager.h"
#include <fstream>
class AppManagerTaskTable {
	std::mutex mMtx;
	std::vector<AppManagerTask *> mVct;
	bool taskAdd(const char *argl) {
		bool ret = false;
		char **argv = argl_dup2_argv(argl, NULL);
		if(!argv) {
			s_err("argl_dup2_argv");
			return ret;
		}
		do {
			if(!(argv[1] && argv[1][0])) {
				s_err("lack args");
				break;
			}
			std::string cmd(argv[0]);
			AppManagerTask *task = NULL;
			if(cmd == "path") {
				task = new AppManagerTask(argv[1]);
			} else if(cmd == "cmdline") {
				std::string cmdline = argl;
				std::size_t pos = cmdline.find(argv[1]);
				cmdline = cmdline.substr(pos);
				task = new AppManagerTask(argv[1], cmdline.data());
			}
			if(!task) {
				s_err("new AppManagerTask");
				return ret;
			}
			mVct.push_back(task);
			ret = true;
		} while(0);
		argv_dup_free(argv);
		return ret;
	}
	bool taskDel(const char *name) {
		size_t i = 0;
		for(auto task : mVct) {
			if(task->appName == name) {
				delete task;
				mVct.erase(mVct.begin() + i);
				return true;
			}
			i++;
		}
		s_err("cna't find this app:%s", name);
		return false;
	}
	bool taskStop(const char *name) {
		for(auto task : mVct) {
			if(task->appName == name) {
				task->stop();
				return true;
			}
		}
		s_err("cna't find this app:%s", name);
		return false;
	}
	bool taskStart(const char *name) {
		for(auto task : mVct) {
			if(task->appName == name) {
				task->start();
				return true;
			}
		}
		s_err("cna't find this app:%s", name);
		return false;
	}
public:
	~AppManagerTaskTable() {
		for(auto task : mVct) {
			delete task;
		}
	}
	AppManagerTaskTable(const char *taskConfPath) {
		std::ifstream ifs(taskConfPath);
		if(!ifs.is_open()) {
			show_errno(0, "ifs open");
			return ;
		}
		size_t line_size = 1024;
		std::unique_ptr<char[]> line(new char[line_size]);
		do {
			ifs.getline(line.get(), line_size);
			if(ifs.gcount() <= 0) {
				if(ifs.eof()) {
					break;
				}
				show_errno(0, "ifs.read");
				return ;
			}
			if(!taskAdd(line.get())) {
				s_err("task \"%s\" add failed!", line.get());
				return ;
			}
		} while(ifs.gcount() > 0);
	}
	bool add(const char *argl) {
		std::lock_guard<std::mutex> lck (mMtx);
		return taskAdd(argl);
	}
	bool del(const char *name) {
		std::lock_guard<std::mutex> lck (mMtx);
		return taskDel(name);
	}
	bool stop(const char *name) {
		std::lock_guard<std::mutex> lck (mMtx);
		return taskStop(name);
	}
	bool start(const char *name) {
		std::lock_guard<std::mutex> lck (mMtx);
		return taskStart(name);
	}
	bool restart(const char *name) {
		std::lock_guard<std::mutex> lck (mMtx);
		if(taskStop(name)) {
			return taskStart(name);
		}
		return false;
	}
	bool showSdCount(const char *name) {
		std::lock_guard<std::mutex> lck (mMtx);
		for(auto task : mVct) {
			if(task->appName == name) {
				s_war("%s's SdCount:%u", name, task->getSdCount());
				return true;
			}
		}
		s_err("cna't find this app:%s", name);
		return false;
	}
	bool stopAll() {
		std::lock_guard<std::mutex> lck (mMtx);
		for(auto task : mVct) {
			task->stop();
		}
		return true;
	}
	bool startAll() {
		std::lock_guard<std::mutex> lck (mMtx);
		for(auto task : mVct) {
			if(!task->start()) {
				return false;
			}
		}
		return true;
	}
	bool restartAll() {
		std::lock_guard<std::mutex> lck (mMtx);
		for(auto task : mVct) {
			if(task->stop()) {
				return task->start();
			}
		}
		return true;
	}	
	bool monitorAll() {
		std::lock_guard<std::mutex> lck (mMtx);
		for(auto task : mVct) {
			task->monitor();
		}
		return true;
	}
	bool showAll() {
		std::lock_guard<std::mutex> lck (mMtx);
		for(auto task : mVct) {
			task->show();
		}
		return true;
	}
};
const char	*g_appMtList[] = {
	"path airplaydemo",
	"path SockServ",
	NULL
};
class AppManagerPar {
public:
	const char *taskConfPath;
	const char *ipcFifoPath;
	AppManagerPar() {
		ipcFifoPath =  "/tmp/AppManagerIpc.fifo";
		taskConfPath = "/root/AppManagerTask.conf";
	}
};
class AppManager {
	AppManagerPar *mPar;
	std::thread mIpcThread;
	std::thread mAppMonitorTrd;
	bool mIsExitSeted;
	std::shared_ptr<AppManagerTaskTable> mAppMtTbl;
public:
	~AppManager() {
		if(mAppMonitorTrd.joinable()) {
			mAppMonitorTrd.join();
		}
		if(mIpcThread.joinable()) {
			mIpcThread.join();
		}
	}
	AppManager(AppManagerPar *par) {
		mPar = par;
		mIsExitSeted = false;
		mAppMtTbl = std::make_shared<AppManagerTaskTable>(mPar->taskConfPath);
		if(!mAppMtTbl.get()) {
			s_err("make_shared<AppManagerTaskTable>");
			return;
		}
		mAppMtTbl->startAll();
		mIpcThread = std::thread([this]() {
			IpcThreadProcess();
		});
		mAppMonitorTrd = std::thread([this]() {
			MonitorProdess();
		});
	}
	int IpcThreadProcess() {
		set_thread_name(0, __func__);
		const size_t bytesPerRead = 512;
		char buf[bytesPerRead + 1] = {0};
		if(access(mPar->ipcFifoPath, F_OK) == 0) { //如果文件存在就删除它
			unlink(mPar->ipcFifoPath);
		}
		if(mkfifo(mPar->ipcFifoPath, 0666) < 0) {
			show_errno(0, "mkfifo");
			return -1;
		}
		FILE *fp = fopen(mPar->ipcFifoPath, "r+");
		if(!fp) {
			show_errno(0, "fopen");
			return -1;
		}
		std::string cmdCut;
		for(; !mIsExitSeted;) {
			fgets(buf, bytesPerRead, fp);
			buf[strlen(buf) - 1] = 0;
			s_inf("got cmd:%s", buf);
			char **argv = argl_dup2_argv(buf, NULL);
			if(!(argv)) {
				continue;
			}
			std::string cmd = argv[0];
			if(0) {
			} else if(cmd == "logctrl") {
				if(argv[1]) {
					lzUtils_logInit(argv[1], NULL);
					s_war("logctrl has been set to %s", argv[1]);
				}
			} else if(cmd == "logpath") {
				if(argv[1]) {
					lzUtils_logInit(NULL, argv[1]);
					s_war("logpath has been set to %s", argv[1]);
				}
			} else if(cmd == "add") {
				if(argv[1] && argv[2]) {
					cmdCut = buf;
					std::size_t pos = cmdCut.find(argv[1]);
					if(cmdCut.find(argv[1]) == std::string::npos) {
						continue;
					}
					cmdCut = cmdCut.substr(pos);
					mAppMtTbl->add(cmdCut.data());
				}
			} else if(cmd == "del") {
				if(argv[1] && argv[1][0]) {
					mAppMtTbl->del(argv[1]);
				}
			} else if(cmd == "start") {
				if(argv[1] && argv[1][0]) {
					mAppMtTbl->start(argv[1]);
				}
			} else if(cmd == "stop") {
				if(argv[1] && argv[1][0]) {
					mAppMtTbl->stop(argv[1]);
				}
			} else if(cmd == "restart") {
				if(argv[1] && argv[1][0]) {
					mAppMtTbl->restart(argv[1]);
				}
			} else if(cmd == "startAll") {
				if(argv[0]) {
					mAppMtTbl->startAll();
				}
			} else if(cmd == "stopAll") {
				if(argv[0]) {
					mAppMtTbl->stopAll();
				}
			} else if(cmd == "restartAll") {
				if(argv[0]) {
					mAppMtTbl->restartAll();
				}
			}  else if(cmd == "showAll") {
				if(argv[0]) {
					mAppMtTbl->showAll();
				}
			} else if(cmd == "exit") {
				mAppMtTbl->stopAll();
				exit(0);
			} else if(cmd == "dataReset") {
				cmd_excute("rm -rf /data/overlay/* ; reboot");
			} else {
				s_err("unsurport cmd: %s !!!", cmd.data());
			}
			argv_dup_free(argv);
		}
		fclose(fp);
		return 0;
	}
	int MonitorProdess() {
		set_thread_name(0, __func__);
		for(; !mIsExitSeted;) {
			mAppMtTbl->monitorAll();
			sleep(2);
		}
		return 0;
	}
};
#include <signal.h>
static void signal_handle(int sig) {
	switch(sig) {
	// case SIGCLD: {
		// break;
	// }
	case SIGINT: {
		break;
	}
	case SIGUSR1: {
		break;
	}
	default: {
		break;
	}
	}
}
static int help_info(int argc, char *argv[]) {
	(void)argc;
	printf("%s help:\n", get_last_name(argv[0]));
	printf("\t-l [logLvCtrl]\n");
	printf("\t-p [logPath]\n");
	printf("\t-h show help\n");
	return 0;
}
#include <getopt.h>
int AppManager_main(int argc, char *argv[]) {
	int opt = 0;
	const char *optstr = "l:p:h";
	while ((opt = getopt_long_only(argc, argv, optstr, NULL, NULL)) != -1) {
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
	//打印编译时间
	showCompileTmie(argv[0], s_war);
	WaitOthersInstsExit(argv[0], 20);
	// signal(SIGCLD, &signal_handle);
	std::unique_ptr<AppManagerPar> mPar(new AppManagerPar());
	if(!mPar.get()) {
		s_err("");
		return -1;
	}
	std::unique_ptr<AppManager> mAppManager(new AppManager(mPar.get()));
	if(!mAppManager.get()) {
		s_err("");
		return -1;
	}
	return 0;
}