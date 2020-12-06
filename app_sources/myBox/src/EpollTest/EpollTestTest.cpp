#include <unistd.h>
#include <lzUtils/base.h>
#include <lzUtils/common/fd_op.h>
#include <memory>
#include <string>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <map>
#include <atomic>
#include <vector>
#include <thread>
#include <sstream>
#include <fstream>
#include <lzUtils/common/fp_op.h>	
#include <lzUtils/base.h>	
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#define LINE_BUFFER_SIZE 1024
#define LINE_END_MARK "\r\n"
#define SOCK_MAX_CONN 128
static int help_info(int argc, char *argv[]) {
	s_err("%s help:", get_last_name(argv[0]));
	s_err("\t-i [input path]");
	s_err("\t-o [output url]");
	s_err("\t-l [logLvCtrl]");
	s_err("\t-p [logPath]");
	s_err("\t-h show help");
	return 0;
}
int epollfd = -1;
bool gotExitFlag = false;
size_t retryMax = 10;
int create_listen_fd(){
	int ret = 0;
	int yes = 1;
	char addr_service[8] = {0};
	sprintf(addr_service, "%d", 10080);

	struct addrinfo hints;
	struct addrinfo *result, *rp;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // fill in my IP for me

	ret = getaddrinfo(NULL, addr_service, &hints, &result);
	if(ret != 0 || result == nullptr) {
		show_errno(0, "getaddrinfo");
		return -1;
	}
	int mSocket = -1;
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		mSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (mSocket == -1) {
			continue;
		}

		/* "address alnReadys in use" */
		if( setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1 ) {
			show_errno(0, setsockopt);
			close(mSocket);
			break;
		}

		if (bind(mSocket, rp->ai_addr, rp->ai_addrlen) == 0) {
			s_inf("BindSuccess");
			break;
		}
	}

	if (rp == NULL) {
		s_err("Bind Failed!");
		show_errno(0, "Bind");
		freeaddrinfo(result);
		close(mSocket);
		return -1;
	}
	freeaddrinfo(result);
	if(listen(mSocket, SOCK_MAX_CONN) != 0){
		show_errno(0, "listen");
		close(mSocket);
		return -1;
	}	
	return mSocket;	
}

bool getline(std::string& line,int fd){
	char buf[LINE_BUFFER_SIZE+1];
	std::size_t end = std::string::npos;
	ssize_t res = 0;
	line = "";
	size_t retryCount = 0;s_inf("read fd=%d",fd);
	for(;!gotExitFlag;){
		// res = read(fd,buf,sizeof(buf));
		res = read(fd,buf,LINE_BUFFER_SIZE);
		if(res <= 0){
			show_errno(0,"read failed");
			s_err("res=%d",res);
			if(errno == EAGAIN || errno == EWOULDBLOCK ||errno == EINTR){
				s_err("retry count %d ...",retryCount);
				if(++retryCount < retryMax){
					usleep(100*1000);
					continue;
				}
			}
			return false;
		}
		retryCount = 0;
		buf[res] = 0;
		s_inf("read data:%s",buf);
		line += buf;
		end = line.find(LINE_END_MARK);
		if(end == line.npos){
			continue;
		}
	}
	if(line == LINE_END_MARK){
	}
	line = line.substr(0,end);
	if(gotExitFlag){
		return false;
	}	
	return true;
}
void do_use_fd(int fd){
	std::string line;
	if(getline(line,fd) == false){
		s_err("getline false！");
		return;
	}
	s_inf(line.data());
}
int epoll_run(){
	#define MAX_EVENTS 10
	struct epoll_event ev, events[MAX_EVENTS];
	int listen_sock, conn_sock, nfds;
	listen_sock = create_listen_fd();
	if(listen_sock < 0){
		s_err("");
        exit(EXIT_FAILURE);
	}
	/* Code to set up listening socket, 'listen_sock',
	  (socket(), bind(), listen()) omitted */

	epollfd = epoll_create1(0);
	if (epollfd == -1) {
	   perror("epoll_create1");
	   exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
	   perror("epoll_ctl: listen_sock");
	   exit(EXIT_FAILURE);
	}

	for (;;) {
	   nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
	   if (nfds == -1) {
		   perror("epoll_wait");
		   exit(EXIT_FAILURE);
	   }
		int n = 0;
		struct sockaddr local;
		socklen_t addrlen = sizeof(local);
	   for (n = 0; n < nfds; ++n) {
		   if (events[n].data.fd == listen_sock) {
			   conn_sock = accept(listen_sock,
							   (struct sockaddr *) &local, &addrlen);
			   if (conn_sock == -1) {
				   perror("accept");
				   exit(EXIT_FAILURE);
			   }
			   // setnonblocking(conn_sock);
				if(fd_set_flag(conn_sock,O_NONBLOCK)< 0){
					perror("fd_set_flag");
				   exit(EXIT_FAILURE);
				}			   
			   ev.events = EPOLLIN | EPOLLET;
			   ev.data.fd = conn_sock;
			   if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
						   &ev) == -1) {
				   perror("epoll_ctl: conn_sock");
				   exit(EXIT_FAILURE);
			   }
		   } else {
			   do_use_fd(events[n].data.fd);
		   }
	   }
	}		
}
int EpollTest_main(int argc,char* argv[]){
	int opt  =-1;
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
	return epoll_run();
}