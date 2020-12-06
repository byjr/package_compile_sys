#include "TcpServer.h"
#include "HandleTask.h"
#include <lzUtils/common/fd_op.h>
#define LINE_BUFFER_SIZE 256
#define LINE_END_MARK "\r\n"
static std::vector<const char*> HttpMethodList={
	"GET","POST","PUT"
};
static int StEvtMap[] = {
	[(int)SocktState::rAble]=EPOLLIN,
	[(int)SocktState::wAble]=EPOLLOUT,
};
void TaskHandler::notifyState(SocktState state){
	std::unique_lock<std::mutex> locker(mu);
	mSktSt = state;
	locker.unlock();
	cv.notify_one();
}
bool TaskHandler::waitState(SocktState state){
	//add evt to epoolfd
	struct epoll_event ev;
	ev.events = EPOLLIN|EPOLLET;
	ev.data.fd = mPar->fd;
	s_inf("epfd=%d,fd=%d",mPar->epfd,mPar->fd);
	int res = epoll_ctl(mPar->epfd,EPOLL_CTL_ADD,mPar->fd,&ev);
	if(res == -1){
		show_errno(0, "epoll_ctl");
		close(mPar->fd);
		return false;			
	}
	std::unique_lock<std::mutex> lk(mu);
	while(mSktSt != state && !gotExitFlag){
		if(cv.wait_for(lk,std::chrono::milliseconds(500),
			[&]()->bool{return (mSktSt == state);})) {
			return true;
		}
	}
	return false;
}
bool TaskHandler::getline(std::string& line){
	if(!waitState(SocktState::rAble)){
		return false;
	}
	char buf[LINE_BUFFER_SIZE+1];
	std::size_t end = std::string::npos;
	ssize_t res = 0;
	line = "";
	size_t retryCount = 0;s_inf("read fd=%d",mPar->fd);
	for(;!gotExitFlag;){
		res = read(mPar->fd,buf,LINE_BUFFER_SIZE);
		if(res <= 0){
			show_errno(0,"read failed");
			s_err("res=%d",res);
			if(errno == EAGAIN || errno == EWOULDBLOCK ||errno == EINTR){
				s_err("retry count %d ...",retryCount);
				if(++retryCount < mPar->retryMax){
					usleep(100*1000);
					continue;
				}
			}
			return false;
		}
		retryCount = 0;
		buf[res] = 0;
		line += buf;
		end = line.find(LINE_END_MARK);
		if(end == line.npos){
			continue;
		}
	}
	if(line == LINE_END_MARK){
		std::size_t rem_size = res-end-sizeof(LINE_END_MARK);
		mRbVct.resize(rem_size);
		memcpy(mRbVct.data(),buf+sizeof(LINE_END_MARK),rem_size);
	}
	line = line.substr(0,end);
	if(gotExitFlag){
		return false;
	}	
	return true;
}
bool TaskHandler::getHttpMethod(std::string& line){
	for(auto i:HttpMethodList){
		if(line.find(i) == 0){
			mCtxMap["method"]=i;
			return true;
		}
	}
	return false;
}
bool TaskHandler::Send(const char *buf, int size) {
	ssize_t res = 0,retryCount = 0,count = 0;
	for(;!gotExitFlag;){
		if(!waitState(SocktState::wAble)){
			s_err("waitReadAble Except!!!");
			return false;
		}
		res = write(mPar->fd,buf+count,size-count);
		if(res <= 0){
			show_errno(0,"write failed");
			if(errno == EAGAIN || errno == EWOULDBLOCK ||errno == EINTR){
				s_err("retry count %d ...",retryCount);
				if(++retryCount < mPar->retryMax){
					continue;
				}
			}
			return false;
		}
		retryCount = 0;
		count += res;
		if(count < size){
			continue;
		}
	}
	if(gotExitFlag){
		return false;
	}
	return true;
}
bool TaskHandler::responseMime() {
	std::stringstream ss;
	time_t ts = time(NULL);
	static char strDate[128];
	struct tm *t = gmtime(&ts);
	strftime(strDate, sizeof(strDate), "%a, %d %b %Y %H:%M:%S GMT", t);
	ss << "HTTP/1.1 200 OK\r\n";
	ss << "Accept-Ranges: bytes\r\n";
	ss << "Date: " << strDate << "\r\n";
	ss << "Content-Type: " << mCtxMap["Accept"] << "\r\n";
	size_t bytes = get_size_by_path(mCommand.data());
	if(bytes <= 0) {
		s_war("get_size_by_path failed,set Content-Length to -1 .");
		bytes = -1;
	}
	ss << "Content-Length: " << bytes << "\r\n\r\n";
	std::string strBuf = ss.str();

	return Send(strBuf.c_str(),strBuf.length());
}
bool TaskHandler::getContex(){
	std::string line;
	if(getline(line) == false){
		s_err("getline false！");
		return false;
	}			
	if(getHttpMethod(line) == false){
		s_err("getHttpMethod false！");
		return false;
	}
	if(0){
	}else if(mCtxMap["method"] == "GET"){
		std::size_t l(std::string::npos);
		std::size_t r(std::string::npos);
		mCommand = line;
		do{
			l = mCommand.find_first_of('/');
			if(l == std::string::npos){
				mCommand = "";
				break;
			}
			r = mCommand.find_first_of('?');
			if(r == std::string::npos){
				mCommand = "";
				break;
			}
			mCommand=mCommand.substr(l+1,r-l-1);				
		}while(l != std::string::npos);
		if(mCommand == ""){
			s_err("getHttp Command false！");
			return false;
		}
		l=r+1;
		r = line.find("HTTP/");
		std::string parm = line.substr(l,r-l);
		std::string key("");
		while(parm != "" && !gotExitFlag){
			r = parm.find_first_of('=');
			if(r == std::string::npos){
				break;
			}
			key = parm.substr(0,r);
			parm = parm.substr(r+1);				
			r = parm.find_first_of("& ");
			if(r == std::string::npos){
				s_err("Err: error happend wehen parse parmenter!");
				return false;
			}
			mParMap[key]=parm.substr(0,r);
			parm = parm.substr(r+1);		
		}
		do{
			if(getline(line) == false){
				s_err("GET/getline false！");
				return false;
			}
			if(line == ""){
				break;
			}
			r = line.find(": ");
			if(r == std::string::npos){
				s_err("Err: error happend wehen parse header!");
				return false;
			}
			key = line.substr(0,r);
			mCtxMap[key]=line.substr(r+2);
		}while(line != "" && !gotExitFlag);
	}else if(mCtxMap["method"] == "POST"){
		
	}else if(mCtxMap["method"] == "PUT"){
		
	}else{
		
	}		
}
bool TaskHandler::doFilter(){
	return true;
}
bool TaskHandler::responseText(const char *txt) {
	std::stringstream ss;
	ss << "SRPL /";
	ss << "handleRes";
	ss << "?state=";
	ss << txt;
	ss << "\r\n\r\n";
	return Send(ss.str().data(),ss.str().size());
}	
bool TaskHandler::sendNormalFile(){
#if 0	
	int fd = open(mCommand.data(), O_RDONLY | O_NONBLOCK, 0666);
	if(fd < 0){
		show_errno(0,"open");
		s_err("open %s failed!",mCommand.data());
		return false;
	}
	// fd_set_flage();
	FILE* fp = fdopen(fd,"rb");
	if(fp == nullptr){
		show_errno(0,"fdopen");	
		close(fd);
		return false;
	}
	char buf[256];
	size_t res(0);
	size_t retryCount(0);
	do{
		res = fread(buf,1,sizeof(buf),fp);
		if(res <= 0){
			if(feof(fp)){
				break;
			}
			show_errno(0,"fread");
			if(++retryCount < mPar->retryMax){
				break;
			}
			if(gotExitFlag){
				break;
			}
			continue;
		}
	}while(!ifs.eof());
#else
	std::ifstream ifs(mCommand,std::ios::binary);
	if(!ifs.is_open()){
		show_errno(0,"open");
		s_err("open %s failed!",mCommand.data());
		return false;			
	}
	char buf[256];
	size_t res(0);
	size_t retryCount(0);		
	do{
		ifs.read(buf,sizeof(buf));
		if(res <= 0){
			if(ifs.eof()){
				break;
			}
			continue;				
		}
		if(Send(buf,res) < 0){
			ifs.close();
			return false;
		}
	}while(ifs.eof() == false);
	ifs.close();
	return true;
#endif	
}
bool TaskHandler::doHandle(){
	if(mCtxMap["method"] == "GET"){
		s_inf("Command=%s",mCommand.data());
		for(auto i:mParMap){
			s_inf("%s=%s",i.first.data(),i.second.data());
		}
		for(auto i:mCtxMap){
			s_inf("%s=%s",i.first.data(),i.second.data());
		}
		if(mCommand == ""){
			mCommand = "index.html";
			mCtxMap["Accept"]="text/html";
			if(!responseMime()){
				s_err("responseMime failed!");
				return false;
			}
			return sendNormalFile();
		}else if(mCommand == "" || mCommand.find_first_of('.')){
			mCtxMap["Accept"]="text/html";
			if(!responseMime()){
				s_err("responseMime failed!");
				return false;
			}
			return sendNormalFile();
		}
	}
}
bool TaskHandler::run(){
	if(set_thread_name(NULL,"TaskHandler")< 0){
		s_err("set_thread_name:TaskHandler failed!");
	}
	if(getContex()){
		return responseText("ERR:server got a Incorrect Contex!");
	}
	if(doFilter()){
		return responseText("ERR:the request was filtered by server!");
	}
	return doHandle();
}
void TaskHandler::notifyReadAble(){
	notifyState(SocktState::rAble);
}
void TaskHandler::notifyWriteAble(){
	notifyState(SocktState::wAble);
}
void TaskHandler::notifyPeerClosed(){
	gotExitFlag = true;
	notifyState(SocktState::closed);	
}
void TaskHandler::stop(){
	gotExitFlag = true;
}
bool TaskHandler::isFinished(){
	return !mTrd.joinable();
}
TaskHandler::TaskHandler(std::shared_ptr<TaskHandlerPar> par){
	mPar = par;
	gotExitFlag = false;
	mSktSt = SocktState::min;
	if(fd_set_flag(mPar->fd,O_NONBLOCK)< 0){
		s_err("fd_set_flag failed!");
		return;
	}
	mTrd = std::thread([this]()->bool{
		return run();
	});
}
TaskHandler::~TaskHandler(){
	if(mTrd.joinable()){
		mTrd.join();
	}
}	
// }