#include "HandleTask.h"
#include "TcpServer.h"
#define SOCK_MAX_CONN 1024
#include <bitset>
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
	if(set_thread_name(0, "TcpServer") < 0) {
		s_err("set_thread_name:TcpServer failed!");
	}
	mEpFd = epoll_create1(0);
	if(mEpFd == -1) {
		show_errno(0, "epoll_create1");
		return false;
	}
	s_inf(__func__);
	struct epoll_event ev;
	ev.events = EPOLLIN;
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
			nReadys = epoll_pwait(mEpFd, events.data(), mPar->maxevents, mPar->timeout_ms, NULL);
			s_inf("nReadys:%d", nReadys);
			if(nReadys == -1) {
				show_errno(0, "epoll_wait");
				if(errno == EINTR) {
					continue;
				}
			}
			break;
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
				mTaskHandlerPar->plyBuf = mPar->plyBuf;
				mTaskHandlerPar->recBuf = mPar->recBuf;
				mTaskHandler = std::make_shared<TaskHandler>(mTaskHandlerPar);
				if(mTaskHandler.get() == nullptr) {
					s_err("");
					close(fd);
					break;
				}
				mThMap[conn] = std::move(mTaskHandler);
			} else {
				do {
					if(events[i].events & EPOLLHUP) {
						s_inf("EPOLLHUP is set!!!");
						auto hd = mThMap[fd];
						if(hd) {
							hd->notifyPeerClosed();
						}
						break;
					}
					if(events[i].events & EPOLLERR) {
						s_err("EPOLLERR is set!!!");
						auto hd = mThMap[fd];
						if(hd) {
							hd->notifyPeerClosed();
						}
						break;
					}
					if(events[i].events & EPOLLIN) {
						s_inf("EPOLLIN is set!!!");
						auto hd = mThMap[fd];
						if(hd) {
							hd->notifyReadAble();
						}
					}
					if(events[i].events & EPOLLOUT) {
						s_inf("EPOLLOUT is set!!!");
						auto hd = mThMap[fd];
						if(hd) {
							hd->notifyWriteAble();
						}
					}
				} while(0);
			}
			s_inf("events:%s", std::bitset<32>(events[i].events).to_string().data());
		}
		s_inf("===============================mThMap size:%d", mThMap.size());
		for(auto it = mThMap.begin(); it != mThMap.end();) {
			if(!it->second) {
				continue;
			}
			if(it->second->isFinished()) {
				it = mThMap.erase(it);
			} else {
				it++;
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
