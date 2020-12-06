#ifndef _TaskHandler_H
#define _TaskHandler_H
#include "TheCommon.h"
enum class SocktState{
	min = -1,
	rAble,
	wAble,
	closed,
	max
};
struct TaskHandlerPar{
	int fd;
	int epfd;
	size_t retryMax;
	std::shared_ptr<PerrAddr_t> peerAddr;
	TaskHandlerPar(){
		fd  = -1;
		epfd = -1;
		retryMax = 10;
	}
};
class TaskHandler{
	std::shared_ptr<TaskHandlerPar> mPar;
	std::mutex mu;
	std::condition_variable cv;
	SocktState mSktSt;
	std::string mMethod;
	std::string mCommand;
	std::unordered_map<std::string,std::string> mParMap;
	std::unordered_map<std::string,std::string> mCtxMap;	
	std::atomic<bool> gotExitFlag;
	std::atomic<bool> hadExitedFlag;
	std::vector<char> mRbVct;
	std::thread mTrd;
	bool waitState(SocktState state);
	void notifyState(SocktState state);
	bool getHeader(std::string& header);
	bool getHttpMethod(std::string& header);
	bool Send(const char *buf, int size);
	bool responseMime();
	bool getContex();
	bool doFilter();
	bool responseText(const char *txt);	
	bool sendNormalFile();
	bool doHandle();
	bool run();
public:
	void notifyReadAble();
	void notifyWriteAble();
	void notifyPeerClosed();
	void stop();
	bool isFinished();
	TaskHandler(std::weak_ptr<TaskHandlerPar> par);
	~TaskHandler();
};
#endif