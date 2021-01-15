#include "HandleTask.h"
#include "TcpServer.h"
#define SOCK_MAX_CONN 1024

static std::unordered_map<int, const char *> epEvtMap = {
	{EPOLLIN, "EPOLLIN"},
	{EPOLLOUT, "EPOLLOUT"},
	{EPOLLRDHUP, "EPOLLRDHUP"},
	{EPOLLPRI, "EPOLLPRI"},
	{EPOLLERR, "EPOLLERR"},
	{EPOLLHUP, "EPOLLHUP"},
	{EPOLLET, "EPOLLET"},
	{EPOLLONESHOT, "EPOLLONESHOT"},
	{EPOLLWAKEUP, "EPOLLWAKEUP"}
};
bool TcpServer::prepare() {
	int ret = 0;
	int yes = 1;
	char addr_service[8] = {0};
	sprintf(addr_service, "%d", mPar->port);

	struct addrinfo hints;
	struct addrinfo *result, *rp;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // fill in my IP for me

	ret = getaddrinfo(NULL, addr_service, &hints, &result);
	if(ret != 0 || result == nullptr) {
		show_errno(0, "getaddrinfo");
		return false;
	}

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
		return false;
	}
	freeaddrinfo(result);
	if(listen(mSocket, SOCK_MAX_CONN) != 0) {
		show_errno(0, "listen");
		close(mSocket);
		return false;
	}
	return true;
}
bool TcpServer::run() {
	s_inf(__func__);
	if(set_thread_name(NULL, "TcpServer") < 0) {
		s_err("set_thread_name:TcpServer failed!");
	}
	mEpFd = epoll_create1(0);
	if(mEpFd == -1) {
		show_errno(0, "epoll_create1");
		return false;
	}
	s_inf(__func__);
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = mSocket;
	int res = epoll_ctl(mEpFd, EPOLL_CTL_ADD, mSocket, &ev);
	if(res == -1) {
		show_errno(0, "epoll_ctl");
		close(mEpFd);
		return false;
	}
	s_inf("mSocket:%d", mSocket);
	std::vector<struct epoll_event> events(mPar->maxevents);
	s_inf("events size:%d", events.size());
	sigset_t sigmask = {0};
	int nReadys = 0;
	s_inf(__func__);
	for(; !gotExitFlag;) {
		s_inf("epoll_wait ...");
		for(int i = 0; i < mPar->retryMax; i++) {
			nReadys = epoll_wait(mEpFd, events.data(), mPar->maxevents, mPar->timeout_ms);
			s_inf("nReadys:%d", nReadys);
			if(nReadys == -1) {
				show_errno(0, "epoll_wait");
				if(errno == EINTR) {
					continue;
				}
			}
			break;
		}
		if(nReadys == 0) {
			continue;
		}
		for(int i = 0; i < nReadys; i++) {
			int fd = events[i].data.fd;
			int conn = -1;
			s_inf("fd:%d", fd);
			if(fd == mSocket) {
				pAddr = std::make_shared<PerrAddr_t>();
				pAddr->second = sizeof(pAddr->first);
				for(int i = 0; i < mPar->retryMax; i++) {
					conn = accept4(fd, &pAddr->first, &pAddr->second, SOCK_NONBLOCK | SOCK_CLOEXEC);
					if( conn == -1) {
						show_errno(0, "epoll_wait");
						if(errno == EAGAIN | errno == EWOULDBLOCK | errno == EINTR) {
							continue;
						}
					}
					break;
				}
				if(conn == -1) {
					continue;
				}
				mTaskHandlerPar = std::make_shared<TaskHandlerPar>();
				mTaskHandlerPar->fd = conn;
				mTaskHandlerPar->epfd = mEpFd;
				mTaskHandlerPar->retryMax = mPar->retryMax;
				mTaskHandlerPar->peerAddr = pAddr;
				s_inf("epfd=%d,fd=%d", mTaskHandlerPar->epfd, mTaskHandlerPar->fd);
				mTaskHandler = std::make_shared<TaskHandler>(mTaskHandlerPar);
				if(mTaskHandler.get() == nullptr) {
					s_err("");
					close(fd);
					break;
				}
				mThMap[conn] = mTaskHandler;
				// s_inf("epfd=%d,fd=%d",mThMap[conn]->mPar->epfd,mThMap[conn]->mPar->fd);
			} else {
				if(0) {
				} else if(events[i].events == EPOLLIN) {
					auto hd = mThMap[fd];
					if(hd) {
						hd->notifyReadAble();
					}
				} else if(events[i].events == EPOLLOUT) {
					auto hd = mThMap[fd];
					if(hd) {
						hd->notifyWriteAble();
					}
				} else if(events[i].events == EPOLLRDHUP) {
					//Stream socket peer closed connection, or shut down writing half of connection.
					auto hd = mThMap[fd];
					if(hd) {
						hd->notifyPeerClosed();
					}
				} else if(events[i].events == EPOLLPRI) {
					s_inf("got %d %s event,un handle!", fd, epEvtMap[EPOLLPRI]);
					// There is urgent data available for read(2) operations.
				} else if(events[i].events == EPOLLERR) {
					// Error condition happened on the associated file descriptor.  epoll_wait(2) will always wait for this event; it is not necessary to set it in events.
					s_inf("got %d %s event,un handle!", fd, epEvtMap[EPOLLPRI]);
				} else if(events[i].events == EPOLLHUP) {
					// Hang  up  happened  on the associated file descriptor.  epoll_wait(2) will always wait for this event; it is not necessary to set it in events.  Note that when reading
					// from a channel such as a pipe or a stream socket, this event merely indicates that the peer closed its end of the channel.  Subsequent  reads  from  the  channel  will
					// return 0 (end of file) only after all outstanding data in the channel has been consumed.
					s_inf("got %d %s event,un handle!", fd, epEvtMap[EPOLLPRI]);
				} else if(events[i].events == EPOLLWAKEUP) {
					// If EPOLLONESHOT and EPOLLET are clear and the process has the CAP_BLOCK_SUSPEND capability, ensure that the system does not enter "suspend" or "hibernate"  while  this event  is pending or being processed.  The event is considered as being "processed" from the time when it is returned by a call to epoll_wait(2) until the next call to epoll_wait(2) on the same epoll(7) file descriptor, the closure of that file descriptor, the removal of the event file descriptor with EPOLL_CTL_DEL, or  the  clearing of EPOLLWAKEUP for the event file descriptor with EPOLL_CTL_MOD.  See also BUGS.
					s_inf("got %d %s event,un handle!", fd, epEvtMap[EPOLLPRI]);
				} else {
					s_inf("got %d %s event, un handle!", fd, "unknow!");
				}
			}
		}
		s_inf("mThMap size:%d", mThMap.size());
		for(auto it = mThMap.begin(); it != mThMap.end(); it++) {
			if(!it->second) {
				continue;
			}
			if(it->second->isFinished()) {
				close(it->first);
				it = mThMap.erase(it);
			}
		}
	}
}

TcpServer::TcpServer(std::weak_ptr<TcpServerPar> par) {
	gotExitFlag = false;
	mPar = par.lock();
	if(prepare() == false) {
		s_err("");
		return ;
	}
	s_inf("mThMap size:%d", mThMap.size());
	mTrd = std::thread([this]()->bool{
		return run();
	});
}
TcpServer::~TcpServer() {
	if(mTrd.joinable()) {
		mTrd.join();
	}
}
// }
