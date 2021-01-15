#include "TcpServer.h"
#include "HandleTask.h"
#include "EpollWrapper.h"
#include <lzUtils/common/fd_op.h>

#define LINE_BUFFER_SIZE	(1024)
#define LINE_END_MARK		"\r\n"
#define LINE_END_SIZE		(sizeof(LINE_END_MARK)-1)
#define LINE_MID_MARK		": "
#define LINE_MID_SIZE		(sizeof(LINE_MID_MARK)-1)
#define HEADER_END_MARK		"\r\n\r\n"
#define HEADER_END_SIZE		(sizeof(HEADER_END_MARK)-1)

#define WRITE_EVT_FLAGS		(EPOLLOUT|EPOLLET)
#define READ_EVT_FLAGS		(EPOLLIN|EPOLLET)

void TaskHandler::notifyState(SocktState state) {
	std::unique_lock<std::mutex> locker(mu);
	mSktSt = state;//s_inf("state=%d,mSktSt=%d",(int)state,(int)mSktSt);
	locker.unlock();
	cv.notify_one();
}
bool TaskHandler::waitState(SocktState state) {
	std::unique_lock<std::mutex> lk(mu);
	auto unint_dur = std::chrono::milliseconds(100);
	int count = std::chrono::seconds(10) / unint_dur;
	mSktSt = SocktState::min;
	for(int i = 0; i < count; i++) {
		cv.wait_for(lk, unint_dur, [this, state]()->bool{
			return (gotExitFlag || mSktSt == state);});
		if(gotExitFlag) {
			return false;
		}
	}
	return true;
}
bool TaskHandler::getHeader(std::string &header) {
	char buf[LINE_BUFFER_SIZE + 1];
	std::size_t end = header.npos;
	ssize_t res = 0, bakLen = 0;
	header = "";
	s_inf("%s/%d", __func__, mPar->fd);
	for(; !gotExitFlag;) {
		res = read(mPar->fd, buf, LINE_BUFFER_SIZE);
		if(res <= 0) {
			if(errno == EINTR) {
				continue;
			}
			if(errno == EAGAIN || errno == EWOULDBLOCK) {
				if(!waitState(SocktState::rAble)) {
					return false;
				}
				continue;
			}
			show_errno(0, "read failed");
			return false;
		}
		bakLen = header.size();
		header.append(buf, res);
		end = header.find(HEADER_END_MARK);
		if(end == header.npos) {
			continue;
		}
		std::size_t rem_size = res - HEADER_END_SIZE - (end - bakLen);
		mRbVct.resize(rem_size);
		memcpy(mRbVct.data(), buf + res - rem_size, rem_size);
		header = header.substr(0, header.size() - LINE_END_SIZE);
		break;
	}
	return (!gotExitFlag);
}
bool TaskHandler::getHttpMethod(std::string &header) {
	// for(auto i:HttpMethodList){
	// if(header.find(i.second.data()) == 0){
	// mCtxMap["method"]=i;
	// return true;
	// }
	// }
	return false;
}
bool TaskHandler::Send(const char *buf, int size) {
	ssize_t res = 0, count = 0;
	for(; count < size && !gotExitFlag;) {
		res = write(mPar->fd, buf + count, size - count);
		if(res <= 0) {
			if(errno == EINTR) {
				continue;
			}
			if(errno == EAGAIN || errno == EWOULDBLOCK) {
				if(!waitState(SocktState::wAble)) {
					s_err("waitReadAble Except!!!");
					return false;
				}
				continue;
			}
			show_errno(0, "write failed");
			s_err("res=%d", res);
			return false;
		}
		count += res;
	}
	return !gotExitFlag;
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
	size_t bytes = -1;
	if(mCommand.find(".fifo") != mCommand.npos) {
		size_t bytes = get_size_by_path(mCommand.data());
		if(bytes <= 0) {
			s_war("get_size_by_path failed,set Content-Length to -1 .");
			bytes = -1;
		}
	}
	ss << "Content-Length: " << bytes << "\r\n\r\n";
	std::string strBuf = ss.str();

	return Send(strBuf.c_str(), strBuf.length());
}
static std::unordered_map<std::string, std::string> HttpMethodList{
	{"GET", "GET /"},
	{"POST", "POST /"},
	{"PUT", "PUT /"},
};
bool TaskHandler::getContex() {
	std::string header(""), key("");
	std::size_t l(header.npos);
	std::size_t r(header.npos);

	if(!getHeader(header)) {
		s_err("getHeader false！");
		return false;
	}
	for(auto i : HttpMethodList) {
		if(header.find(i.second.data()) == 0) {
			mMethod = i.first;
			header = header.substr(i.second.size());
			break;
		}
	}
	if(0) {
	} else if(mMethod == "GET") {
		if(header[0] != ' ') {
			mCommand = header;
			r = mCommand.find_first_of("? ");
			if(r == mCommand.npos) {
				s_err("get mCommand false！");
				return false;
			}
			mCommand = mCommand.substr(0, r);
			s_inf(mCommand.data());
			if(header[r] == '?') r += 1;
			header = header.substr(r);
			if(header[0] != ' ') {
				for(;;) {
					r = header.find_first_of('=');
					if(r == header.npos) {
						s_err("Err:!");
						return false;
					}
					key = header.substr(0, r);
					header = header.substr(r + 1);
					r = header.find_first_of("& ");
					if(r == header.npos) {
						s_err("Err:!");
						return false;
					}
					mParMap[key] = header.substr(0, r);
					if(header[r] == ' ') {
						r = header.find(LINE_END_MARK);
						if(r == header.npos) {
							s_err("Err:!");
							return false;
						}
						header = header.substr(r + LINE_END_SIZE);
						break;
					}

				}
			} else {
				r = header.find(LINE_END_MARK);
				if(r == header.npos) {
					s_err("Err:!");
					return false;
				}
				header = header.substr(r + LINE_END_SIZE); //s_inf(header.data());
			}
		} else {
			r = header.find(LINE_END_MARK);
			if(r == header.npos) {
				s_err("Err:!");
				return false;
			}
			header = header.substr(r + LINE_END_SIZE);
			s_inf(header.data());
		}
		for(; header != "";) {
			r = header.find(LINE_MID_MARK);
			if(r == header.npos) {
				break;
			}
			key = header.substr(0, r);
			header = header.substr(r + LINE_MID_SIZE);
			r = header.find(LINE_END_MARK);
			if(r == header.npos) {
				break;
			}
			mCtxMap[key] = header.substr(0, r); //s_inf("%s#%s",key.data(),mCtxMap[key].data());
			header = header.substr(r + LINE_END_SIZE);
		}
	} else if(mMethod == "POST") {
		s_inf(header.data());
		s_inf(mMethod.data());
	} else if(mMethod == "PUT") {

	} else {
		s_err("getHeader false！");
		return false;
	}
	return !gotExitFlag;
}
bool TaskHandler::doFilter() {
	return true;
}
bool TaskHandler::responseText(const char *txt) {
	std::stringstream ss;
	ss << "SRPL /";
	ss << "handleRes";
	ss << "?state=";
	ss << txt;
	ss << "\r\n\r\n";
	return Send(ss.str().data(), ss.str().size());
}
bool TaskHandler::sendNormalFile() {
#if 0
	int fd = open(mCommand.data(), O_RDONLY | O_NONBLOCK, 0666);
	if(fd < 0) {
		show_errno(0, "open");
		s_err("open %s failed!", mCommand.data());
		return false;
	}
	// fd_set_flage();
	FILE *fp = fdopen(fd, "rb");
	if(fp == nullptr) {
		show_errno(0, "fdopen");
		close(fd);
		return false;
	}
	char buf[256];
	size_t res(0);
	size_t retryCount(0);
	do {
		res = fread(buf, 1, sizeof(buf), fp);
		if(res <= 0) {
			if(feof(fp)) {
				break;
			}
			show_errno(0, "fread");
			if(++retryCount < mPar->retryMax) {
				break;
			}
			if(gotExitFlag) {
				break;
			}
			continue;
		}
	} while(!ifs.eof());
#else
	std::ifstream ifs(mCommand, std::ios::binary);
	if(!ifs.is_open()) {
		show_errno(0, "open");
		s_err("open %s failed!", mCommand.data());
		return false;
	}
	char buf[1024];
	size_t res(0);
	size_t retryCount(0);
	do {
		ifs.read(buf, sizeof(buf));
		res = ifs.gcount();
		if(res <= 0) {
			if(ifs.eof()) {
				break;
			}
			continue;
		}
		if(!Send(buf, res)) {
			ifs.close();
			return false;
		}
	} while(!ifs.eof());
	ifs.close();
	return true;
#endif
}
bool TaskHandler::sendPcmData(DataBuffer *buf) {
	if(!buf) {
		s_err("");
		return false;
	}
	data_ptr data;
	for(; !gotExitFlag;) {
		if(!buf->pop(data, 3600)) {
			mHandleMsg = "Wait Buffer data timeout!";
			return false;
		}
		if(!Send(data->data(), data->size())) {
			s_err("");
			return false;
		}
	}
	return true;
}
bool TaskHandler::doHandle() {
	if(mMethod == "GET") {
		s_inf("Command=%s", mCommand.data());
		for(auto i : mParMap) {
			s_inf("%s=%s", i.first.data(), i.second.data());
		}
		for(auto i : mCtxMap) {
			s_inf("%s=%s", i.first.data(), i.second.data());
		}
		if(mCommand == "") {
			if(!EpollWrapper::mod_evt(mPar->epfd, mPar->fd, WRITE_EVT_FLAGS)) {
				s_err("EpollWrapper::mod_evt failed!");
				return false;
			}
			mCommand = "index.html";
			mCtxMap["Accept"] = "text/html";
			if(!responseMime()) {
				s_err("responseMime failed!");
				return false;
			}
			return sendNormalFile();
		} else if(mCommand.find(".html") != mCommand.npos) {
			if(!EpollWrapper::mod_evt(mPar->epfd, mPar->fd, WRITE_EVT_FLAGS)) {
				s_err("EpollWrapper::mod_evt failed!");
				return false;
			}
			mCtxMap["Accept"] = "text/html";
			if(!responseMime()) {
				s_err("responseMime failed!");
				return false;
			}
			return sendNormalFile();
		} else if(mCommand == "SendApInf") {
			if(!EpollWrapper::mod_evt(mPar->epfd, mPar->fd, WRITE_EVT_FLAGS)) {
				s_err("EpollWrapper::mod_evt failed!");
				return false;
			}
			mCommand = "connect.html";
			mCtxMap["Accept"] = "text/html";
			if(!responseMime()) {
				s_err("responseMime failed!");
				return false;
			}
			return sendNormalFile();
		} else if(mCommand == "playData.pcm") {
			if(!EpollWrapper::mod_evt(mPar->epfd, mPar->fd, WRITE_EVT_FLAGS)) {
				s_err("EpollWrapper::mod_evt failed!");
				return false;
			}
			mCommand = "playData.fifo";
			mCtxMap["Accept"] = "application/octet-stream";
			if(!responseMime()) {
				s_err("responseMime failed!");
				return false;
			}
			return sendPcmData(mPar->plyBuf);
		} else if(mCommand == "recordData.pcm") {
			if(!EpollWrapper::mod_evt(mPar->epfd, mPar->fd, WRITE_EVT_FLAGS)) {
				s_err("EpollWrapper::mod_evt failed!");
				return false;
			}
			mCommand = "recordData.fifo";
			mCtxMap["Accept"] = "application/octet-stream";
			if(!responseMime()) {
				s_err("responseMime failed!");
				return false;
			}
			return sendPcmData(mPar->plyBuf);
		} else {
			if(!EpollWrapper::mod_evt(mPar->epfd, mPar->fd, WRITE_EVT_FLAGS)) {
				s_err("EpollWrapper::mod_evt failed!");
				return false;
			}
			mCtxMap["Accept"] = "application/octet-stream";
			if(!responseMime()) {
				s_err("responseMime failed!");
				return false;
			}
			return sendNormalFile();
		}
	} else if(mMethod == "POST") {

	}
	return true;
}
bool TaskHandler::run() {
	bool succeed = false;
	do {
		if(set_thread_name(0, "TaskHandler") < 0) {
			s_err("set_thread_name:TaskHandler failed!");
			break;
		}
		succeed = getContex();
		if(!succeed) {
			if(mHandleMsg == "") {
				mHandleMsg = "ERR:server got a Incorrect Contex!";
			}
			responseText(mHandleMsg.data());
			break;
		}
		succeed = doFilter();
		if(!succeed) {
			if(mHandleMsg == "") {
				mHandleMsg = "ERR:the request filtered failed!";
			}
			responseText(mHandleMsg.data());
			break;
		}
		succeed = doHandle();
		if(!succeed) {
			if(mHandleMsg == "") {
				mHandleMsg = "ERR:the request doHandle failed!";
			}
			responseText(mHandleMsg.data());
			break;
		}
		succeed = true;
	} while(0);
	hadExitedFlag = true;
	return succeed;
}
void TaskHandler::notifyReadAble() {
	notifyState(SocktState::rAble);
}
void TaskHandler::notifyWriteAble() {
	notifyState(SocktState::wAble);
}
void TaskHandler::notifyPeerClosed() {
	gotExitFlag = true;
	notifyState(SocktState::closed);
}
void TaskHandler::stop() {
	gotExitFlag = true;
	cv.notify_all();
}
bool TaskHandler::isFinished() {
	return hadExitedFlag;
}
TaskHandler::TaskHandler(std::weak_ptr<TaskHandlerPar> par) {
	mPar = std::move(par.lock());
	gotExitFlag = false;
	hadExitedFlag = false;
	mSktSt = SocktState::min;
	if(fd_set_flag(mPar->fd, O_NONBLOCK) < 0) {
		s_err("fd_set_flag failed!");
		return;
	}
	if(!EpollWrapper::add_evt(mPar->epfd, mPar->fd, READ_EVT_FLAGS)) {
		s_err("EpollWrapper::add_evt failed!");
		return;
	}
	mTrd = std::thread([this]()->bool{
		return run();
	});
	s_war(__func__);
}
TaskHandler::~TaskHandler() {
	if(mTrd.joinable()) {
		mTrd.join();
	}
	close(mPar->fd);
	s_war(__func__);
}
// }