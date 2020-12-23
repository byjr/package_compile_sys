#ifndef _TaskHandler_H
#define _TaskHandler_H
#include <unordered_map>
#include <atomic>
#include <memory>
#include <string.h>
#include <mutex>
#include <condition_variable>
#include <thread>
enum class SocktState{
	min = -1,
	rAble,
	wAble,
	closed,
	max
};
typedef std::pair<struct sockaddr,socklen_t> PerrAddr_t;
struct TaskHandlerPar{
	int fd;
	int epfd;
	size_t retryMax;
	std::shared_ptr<PerrAddr_t> peerAddr;
	DataBuffer* plyBuf;
	DataBuffer* recBuf;
	TaskHandlerPar(){
		fd  = -1;
		epfd = -1;
		retryMax = 10;
		plyBuf = recBuf = nullptr;
	}
};
class TaskHandler{
	std::shared_ptr<TaskHandlerPar> mPar;
	std::mutex mu;
	std::condition_variable cv;
	SocktState mSktSt;
	std::string mMethod;
	std::string mCommand;
	std::string mHandleMsg;
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
	TaskHandler(std::shared_ptr<TaskHandlerPar>& par);
	~TaskHandler();
	bool sendPcmData(DataBuffer* buf);
};
#endif