#ifndef _TcpServer_H
#define _TcpServer_H
#include "TheCommon.h"
#include "HandleTask.h"
struct TcpServerPar {
	int port;
	int maxevents;
	int timeout_ms;
	int retryMax;
	DataBuffer *plyBuf;
	DataBuffer *recBuf;
	TcpServerPar() {
		port = 10080;
		maxevents = 1024;
		timeout_ms = 1000;
		retryMax = 10;
	}
};
class TcpServer {
	int mSocket;
	int mEpFd;
	std::shared_ptr<TcpServerPar> mPar;
	std::unordered_map<int, std::shared_ptr<TaskHandler>> mThMap;
	std::shared_ptr<TaskHandlerPar> mTaskHandlerPar;
	std::shared_ptr<TaskHandler> mTaskHandler;
	std::thread mTrd;
	std::atomic<bool> gotExitFlag;
	std::shared_ptr<PerrAddr_t> pAddr;
	void stop() {
		gotExitFlag = true;
	}
	bool prepare();
	bool run();
public:
	TcpServer(std::weak_ptr<TcpServerPar> par);
	~TcpServer();
};
extern std::unordered_map<int, const char *> epEvtMap;
#endif