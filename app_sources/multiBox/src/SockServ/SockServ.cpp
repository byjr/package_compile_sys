#include <cstring>
#include <cstdio>
#include <iostream>
#include <errno.h>
#include <sstream>
#include <istream>
#include <fstream>
#include <csignal>
#include <future>
#include <chrono>
#include <thread>
#include <vector>
extern "C" {
#include <lzUtils/common/fp_op.h>
#include <lzUtils/common/fd_op.h>
}
#include "SockServ.h"
#define DEFAULT_SERVER_PORT    10086

#define SOCK_MAX_CONN   64
#define MAX_HEADER_SIZE 4096
#define SEND_LEN        4096
// static char g_BreakenPipeFlag = 0;
// bool IsRcvBreakenPipe() {
// bool isRcvBreakenPipe = false;
// if(g_BreakenPipeFlag) {
// g_BreakenPipeFlag = 0;
// isRcvBreakenPipe = true;
// }
// return isRcvBreakenPipe;
// }
// static void setBreakenPipe() {
// g_BreakenPipeFlag  = 1;
// }
using namespace multlBox;
TCPSocket::~TCPSocket() {
	s_trc(__func__);
	if(m_sockfd >= 0) {
		close(m_sockfd);
		m_sockfd = -1;
	}
}

void TCPSocket::closeSocket() {
	s_trc(__func__);
	if(m_sockfd >= 0) {
		close(m_sockfd);
		m_sockfd = -1;
	}
}

/**
 * send data via socket
 *
 * @author etsai (2018/6/1)
 *
 * @param buf - buffer to transmit
 * @param len - length of transmitted buffer
 *
 * @return int - On success, these calls return the number of
 *         characters sent. On error, return -1 or
 *         ERROR_NOT_WRITEABLE (failed to select)
 */
int TCPSocket::Send(const char *buf, int len) {
	if(mExternStopFlag) {
		return -1;
	}
	FD_ZERO(&m_writefds);
	FD_SET(m_sockfd, &m_writefds);
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;
	int fds = select(m_sockfd + 1, NULL, &m_writefds, NULL, &timeout);
	if(fds <= 0) {
		if(fds == 0) {
			s_dbg("no buf to write!");
			return 0;
		}
		show_errno(0, "select");
		return -1;
	}
	int res = send(m_sockfd, (void *)buf, len, MSG_DONTWAIT);
	if(res < 0) {
		show_errno(0, "send");
		if(errno == ECONNRESET || errno == EPIPE) { //客户端已经关闭连接
			return -2;
		}
		return -1;
	}
	return res;
}
bool TCPSocket::isConnActive() {
	FD_ZERO(&m_readfds);
	FD_SET(m_sockfd, &m_readfds);
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 10000;
	int fds = select(m_sockfd + 1, &m_readfds, NULL, NULL, &timeout);
	if(fds <= 0) {
		if(fds < 0) {
			show_errno(0, "select");
		}
		return true;
	}
	char buf[4];
	int res = recv(m_sockfd, buf, sizeof(buf), 0);
	if(res > 0) {
		return true;
	}
	if(ECONNRESET == errno) {
		show_errno(0, "recv");
		return false;
	}
	if(res == 0) {
		s_err("recv 0: peer has performed an orderly shutdown!");
		return false;
	}
	show_errno(0, "recv");
	return true;
}
int TCPSocket::Recv(char *buf, int len) {
	if(mExternStopFlag) {
		return -1;
	}
	FD_ZERO(&m_readfds);
	FD_SET(m_sockfd, &m_readfds);
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 200000;
	int fds = select(m_sockfd + 1, &m_readfds, NULL, NULL, &timeout);
	if(fds <= 0) {
		if(fds == 0) {
			return 0;
		}
		show_errno(0, "select");
		return -1;
	}
	int res = recv(m_sockfd, (void *)buf, len, 0);
	if(res <= 0) {
		if(0 == res || ECONNRESET == errno) {
			return -2;
		}
		show_errno(0, "recv");
		return -1;
	}
	return res;
}
static int RecvFileWithDelay(int fd, char *buf, int len) {
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 200000;
	int fds = select(fd + 1, &rfds, NULL, NULL, &timeout);
	if(fds <= 0) {
		if(fds == 0) {
			return -3;//暂时没有数据可读
		}
		show_errno(0, "select");
		return -1;
	}
	int res = read(fd, buf, len);
	if(res <= 0) {
		if(0 == res || ECONNRESET == errno) {
			return -2;//读到文件尾
		}
		show_errno(0, "recv");
		return -1;
	}
	return res;
}
bool TCPSocket::isReadable() {
	struct timeval timeout;
	FD_ZERO(&m_readfds);
	FD_SET(m_sockfd, &m_readfds);
	timeout.tv_sec = 0;
	timeout.tv_usec = 20000;
	if (select(m_sockfd + 1, &m_readfds, NULL, NULL, &timeout) == -1) {
		return false;
	}
	return true;
}

bool ReqContex::paraParse(std::string &tail) {
	std::string paraStr;
	bool isParseFinished = false;
	int idx = tail.find_first_of('&');
	if(idx < 0) {
		idx = tail.find_first_of(' ');
		if(idx < 0) {
			idx = tail.find("\r\n\r\n");
		}
		isParseFinished = true;
	}
	paraStr = tail.substr(0, idx);
	ReqParam para;
	int paraIdx = paraStr.find_first_of('=');
	if(paraIdx < 0) {
		s_err("para format err('=' is not find).");
		return false;
	}
	para.key = paraStr.substr(0, paraIdx);
	para.value = paraStr.substr(paraIdx + 1);
	m_params.push_back(para);
	if(isParseFinished) {
		s_inf("mMethod=%s", mMethod.data());
		s_inf("mCmd=%s", mCmd.data());
		return true;
	}
	std::string newStr = tail.substr(idx + 1);
	return paraParse(newStr);
}
bool ReqContex::parse(std::string &req) {
	std::string tail = req;
	int idx = tail.find(" /");
	if(idx < 0) {
		return false;
	}
	mMethod = tail.substr(0, idx);
	tail = tail.substr(idx + 2);
	idx = tail.find_first_of('?');
	if(idx < 0) {
		idx = tail.find_first_of(' ');
		if(idx < 0) {
			idx = tail.find("\r\n\r\n");
			mCmd = tail.substr(0, idx);
			return true;
		}
		mCmd = tail.substr(0, idx);
		return true;
	}
	mCmd = tail.substr(0, idx);
	std::string newStr = tail.substr(idx + 1);
	return paraParse(newStr);
}

int SockClient::HandleResponse(std::string &req) {
	int res = 0;
	ReqContex reqCtx;
	if(!reqCtx.parse(req)) {
		ResponseText("urlParseErr");
		s_err("url_parse_err");
		return -1;
	}
	s_war("method=%s", reqCtx.mMethod.data());
	s_war("cmd=%s", reqCtx.mCmd.data());
	for(auto para : reqCtx.m_params) {
		s_war("key=%s", para.key.data());
		s_war("value=%s", para.value.data());
	}
	if(reqCtx.mMethod.compare("GET") == 0) {
		if(reqCtx.mCmd.compare("getVideo") == 0) {
			size_t pids = get_pids_by_name("req_process.sh", NULL, 1);
			if(pids > 0) {
				ResponseText("serverRecording");
				s_err("serverRecording cancle!");
				return -1;
			}
			std::string url, time;
			for(auto para : reqCtx.m_params) {
				if(para.key.compare("ftp") == 0) {
					url = para.value.data();
				}
				if(para.key.compare("time") == 0) {
					time = para.value.data();
				}
			}
			s_inf("/root/bin/req_process.sh -u %s -t %s -r", url.data(), time.data());
			// getCmdResult("/root/bin/req_process.sh -u %s -t %s -r",url.data(),time.data());
			s_inf("reqProcessDone");
		} else if(reqCtx.mCmd == "send_wifi_info") {
			std::string ssid, psk;
			for(auto para : reqCtx.m_params) {
				if(para.key == "ssid") {
					ssid = para.value;
				} else if(para.key == "psk") {
					psk = para.value;
				}
			}
			ResponseText("send_wifi_info&state=recived");
			int res = cmd_excute("wl_cfg.sh sta '%s' '%s'",ssid.data(),psk.data());
			if(res < 0){
				ResponseText("dev_connect_ap&state=failed");				
			}else{
				ResponseText("dev_connect_ap&state=succeed");
				res = cmd_excute("Broadcast &");
				if(res < 0){
					ResponseText("dev_start_broadcast&state=failed");
				}else{
					ResponseText("dev_start_broadcast&state=succeed");
				}
			}			
		} else if(reqCtx.mCmd == "SendApInf") {
			std::string ssid, psk;
			for(auto para : reqCtx.m_params) {
				if(para.key == "ssid") {
					ssid = para.value;
				} else if(para.key == "psk") {
					psk = para.value;
				}
			}
			ResponseText("send_wifi_info&state=recived");
			my_popen("wl_cfg.sh conn '%s' '%s'",ssid.data(),psk.data());
		} else if(reqCtx.m_params.size() < 1 &&
				  reqCtx.mCmd.find(".fifo") != std::string::npos) {
			res = ResponseMime(reqCtx);
			if(res < 0) {
				return res;
			}
			return SendStreaming(reqCtx);
		} else if(reqCtx.m_params.size() < 1 &&
				  reqCtx.mCmd.find('.') != std::string::npos) {
			res = ResponseMime(reqCtx);
			if(res < 0) {
				return res;
			}
			return SendStreamOnece(reqCtx);
		} else {
			return ResponseText("unsurport command!");
		}
	}
	return 0;
}

int SockClient::ResponseMime(ReqContex &reqCtx) {
	std::stringstream responseString;
	time_t ts = time(NULL);
	static char strDate[128];
	struct tm *t = gmtime(&ts);
	strftime(strDate, sizeof(strDate), "%a, %d %b %Y %H:%M:%S GMT", t);

	responseString << "HTTP/1.1 200 OK\r\n";
	responseString << "Accept-Ranges: bytes\r\n";
	responseString << "Date: " << strDate << "\r\n";
	responseString << "Content-Type: " << "application/octet-stream" << "\r\n";
	size_t bytes = get_size_by_path(reqCtx.mCmd.data());
	if(bytes <= 0) {
		s_war("get_size_by_path failed,set Content-Length to -1 .");
		bytes = -1;
	}
	responseString << "Content-Length: " << bytes << "\r\n\r\n";
	std::string strBuf = responseString.str();

	int totalLen = strBuf.length();
	const char *sendBuf = strBuf.c_str();

	int sendIdx = 0;
	int sendLen = 0;
	while (totalLen > 0) {
		sendLen = Send(&sendBuf[sendIdx], totalLen);
		if (sendLen <= 0) {
			if(sendLen < 0) {
				return sendLen;
			}
			break;
		}
		sendIdx += sendLen;
		totalLen -= sendLen;
	}
	return 0;
}
int SockClient::SendStreaming(ReqContex &reqCtx) {
	int res = 0;
	do {
		s_inf(__func__);
		res = SendStreamOnece(reqCtx);
	} while(!mInnerAbortFlag && (res >= 0 || res == -4));
	return res;
}
int SockClient::SendStreamOnece(ReqContex &reqCtx) {
	s_inf(__func__);
	int fd = un_open(reqCtx.mCmd.data(), O_RDONLY | O_NONBLOCK, 0666);
	if(fd < 0) {
		s_inf(reqCtx.mCmd.data());
		show_errno(0, "fopen");
		return -1;
	}
#define FILE_READ_LEN (16*1024)
	char buf[FILE_READ_LEN];
	ssize_t rret = 0;
	do { //从服务端文件中读数据
		rret = RecvFileWithDelay(fd, buf, FILE_READ_LEN);
		s_dbg("freaded rret:%d", rret);
		if(rret < 0) {
			if(rret == -3) {
				if(isConnActive()) {
					s_dbg("connActive...");
					continue;
				}
				return -2;//客户端已关闭
			}
			if(rret == -2) {
				return -4;
			}
			return rret;
		}
		if(0 == rret) {
			continue;
		}
		//发送数据给客户端
		char *wi = buf;
		size_t rem_size = rret ;
		do {
			ssize_t wret = Send(wi, rem_size);
			s_dbg("Send rem_size:%d,wret:%d", rem_size, wret);
			if(wret < 0) {
				mInnerAbortFlag = true;
				break;
			}
			rem_size -= wret;
			wi += wret;
		} while(rem_size > 0 && !mInnerAbortFlag);
	} while(rret && !mInnerAbortFlag);
	close(fd);
	s_inf("SendStreamOnece done");
	return 0;
}
int SockClient::ResponseText(const char *txt) {
	std::stringstream responseString;
	responseString << "SRPL /";
	responseString << "handleRes";
	responseString << "?state=";
	responseString << txt;
	responseString << "\r\n\r\n";
	std::string strBuf = responseString.str();
	int totalLen = strBuf.length();
	const char *sendBuf = strBuf.c_str();
	int sendIdx = 0;
	int sendLen = 0;
	while (totalLen > 0) {
		sendLen = Send(&sendBuf[sendIdx], totalLen);
		if (sendLen <= 0) {
			if(sendLen < 0) {
				return sendLen;
			}
			break;
		}
		sendIdx += sendLen;
		totalLen -= sendLen;
	}
	return 0;
}
bool SockClient::cmdResultParse(std::string &res) {
	int idx = res.find("recStarted");
	if(idx >= 0) {
		ResponseText("recStarted");
		s_inf("recStarted");
	}
	idx = res.find("cvtFailed");
	if(idx >= 0) {
		ResponseText("cvtFailed");
		s_err("cvtFailed");
	}
	idx = res.find("upSucceed");
	if(idx >= 0) {
		ResponseText("upSucceed");
		s_inf("upSucceed");
	}
	idx = res.find("upFailed");
	if(idx >= 0) {
		ResponseText("upFailed");
		s_err("upFailed");
	}
	return false;
}
std::string SockClient::getCmdResult(const char *fmt, ...) {
	va_list args;
	va_start(args, (char *)fmt);
	char *cmd = NULL;
	vasprintf(&cmd, fmt, args);
	va_end(args);
	FILE *fp = popen(cmd, "r");
	if(!fp) {
		s_err("popen");
		return "";
	}
	fcntl(fileno(fp), F_SETFD, FD_CLOEXEC);
	std::string cmdRes;
	do {
		char buf[1024] = "";
		char *res = fgets(buf, sizeof(buf), fp);
		if(!res) {
			break;
		}
		cmdRes += buf;
		cmdResultParse(cmdRes);
	} while(!feof(fp));
	fclose(fp);
	if(cmd)free(cmd);
	return cmdRes;
}

void SockClient::Stop() {
	mExternStopFlag = true;
	closeSocket();
	if(mHhanleThred.joinable()) {
		mHhanleThred.join();
	}
}

bool SockClient::HandleRequest() {
	m_responseStep = RESPONSE_START;
	mHhanleThred = std::thread([this]() {
		pthread_setname_np(pthread_self(), "reqhandle");
		std::string req;
		int res = 9;
		char buf[1024] = "";
		bool is_find_corect_end = false;
		for(int i = 0; !mExternStopFlag && i < 50; i++) {
			memset(buf, 0, sizeof(buf));
			res = Recv(buf, sizeof(buf));
			if(res < 0) {
				m_responseStep = RESPONSE_FINISH;
				if(res == -2) { //检测到客户端已关闭
					closeSocket();
				}
				return false;
			}
			if(res == 0) {
				continue;
			}
			req += buf;
			s_inf("req:%s", req.data());
			res = req.find("\r\n\r\n");
			if(res >= 0) {
				s_inf("find out the http end!");
				is_find_corect_end = true;
				break;
			}
		}
		if(!is_find_corect_end || req.size() < 16) {
			s_err("can't find http end!");
			ResponseText("getUrlErr");
			m_responseStep = RESPONSE_FINISH;
			return false;
		}
		mInnerAbortFlag = false;
		res = HandleResponse(req);
		if(res < 0) {
			if(res == -2) { //检测到客户端已关闭
				closeSocket();
			}
		}
		m_responseStep = RESPONSE_FINISH;
		return true;
	});
	return true;
}

int SockServ::Bind() {
	int ret = 0;
	int yes = 1;
	char addr_service[8] = {0};
	sprintf(addr_service, "%d", m_bindPort);

	struct addrinfo hints;
	struct addrinfo *result, *rp;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // fill in my IP for me

	ret = getaddrinfo(NULL, addr_service, &hints, &result);

	if(ret != 0) {
		show_errno(0, __func__);
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		m_sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (m_sockfd == -1) {
			continue;
		}

		/* "address already in use" */
		if( setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1 ) {
			show_errno(0, __func__);
			close(m_sockfd);
			break;
		}

		if (bind(m_sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
			s_inf("BindSuccess");
			break;
		}
	}

	if (rp == NULL) {
		s_err("Bind Failed!");
		return -2;
	}
	freeaddrinfo(result);
	return 0;
}

int SockServ::Listen() {
	if (listen(m_sockfd, SOCK_MAX_CONN) != 0) {
		show_errno(0, "listen");
		return -1;
	}
	return 0;
}

int SockServ::Run() {
	pthread_setname_np(pthread_self(), "mainLoop");
	int ret = Bind();
	if (ret < 0) {
		exit(-1);
	}

	ret = Listen();
	if(ret < 0) {
		exit(-2);
	}

	FD_ZERO(&m_readfds);    // http server only care about read
	FD_SET(m_sockfd, &m_readfds);

	int client_sockfd;
	int max_sockfd = 0;
	fd_set tmp_fd;
	struct timeval timeout;

	max_sockfd = std::max(max_sockfd, m_sockfd);
	while (1) {
		// reset timeout
		timeout.tv_sec = 0;
		timeout.tv_usec = 200000;
		tmp_fd = m_readfds; // must using tmp_fd to every time, or select will not work

		int fds = select(max_sockfd + 1, &tmp_fd, NULL, NULL, &timeout);
		if(fds <= 0) {
			if(fds == 0 || errno == EINTR || errno == EAGAIN) {
				continue;
			}
			show_errno(0, "select");
			exit(-3);
		}
		if (FD_ISSET(m_sockfd, &tmp_fd)) {
			// server part
			if ((client_sockfd = accept(m_sockfd, NULL, 0)) != -1) {
				SockClient *newClient = new SockClient(client_sockfd);
				m_clients.push_back(newClient);
				FD_SET(client_sockfd, &m_readfds);
				max_sockfd = std::max(max_sockfd, client_sockfd);
			}
		} else {
			std::list<SockClient *>::iterator it = m_clients.begin();
			for (; it != m_clients.end(); ++it) {
				if ((*it)->IsWaitingRequest() && (*it)->isReadable()) {
					FD_CLR((*it)->GetSocketFD(), &m_readfds);
					(*it)->HandleRequest();
				}
			}
			for (it = m_clients.begin(); it != m_clients.end();) {
				if ((*it)->IsFinishResponse()) {
					(*it)->Stop();
					delete (*it);
					it = m_clients.erase(it);
				} else {
					it++;
				}
			}
		}
	}
	return 0;
}

void SockServ::Stop() {
	mExternStopFlag = true;
	if (m_mainLoopThread.joinable()) {
		m_mainLoopThread.join();
	}

	std::list<SockClient *>::iterator it = m_clients.begin();
	for (; it != m_clients.end(); ++it) {
		(*it)->Stop();
		delete (*it);
	}
	m_clients.clear();

	if(m_sockfd) {
		close(m_sockfd);
		m_sockfd = -1;
	}
}

SockServ::~SockServ() {

}

SockServ::SockServ() {
	s_trc(__func__);
	if (m_bindPort == 0) {
		m_bindPort = 10086;
	}
	m_mainLoopThread = std::thread(&SockServ::Run, this);
}

static int help_info(int argc, char *argv[]) {
	printf("%s help:\n", get_last_name(argv[0]));
	printf("\t-r [share path]\n");
	printf("\t-l [logLvCtrl]\n");
	printf("\t-p [logPath]\n");
	printf("\t-h show help\n");
	return 0;
}
// static  void pipeProcess(int sig) {
// err_nl(__func__);
// setBreakenPipe();
// }
#include <getopt.h>
int SockServ_main(int argc, char *argv[]) {
	int opt = 0;
	while ((opt = getopt_long_only(argc, argv, "r:l:p:dh", NULL, NULL)) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		case 'r':
			// resouce_path=optarg;
			break;
		default: /* '?' */
			return help_info(argc, argv);
		}
	}
	//打印编译时间
	showCompileTmie(argv[0], s_war);
	// signal(SIGPIPE,&pipeProcess);
	signal(SIGPIPE, SIG_IGN);
	WaitOthersInstsExit(argv[0], 20);
	SockServ *webaerv = new SockServ();
	if(!webaerv) {
		return -1;
	}
	while(1) {
		sleep(1);
	}
	delete webaerv;
}