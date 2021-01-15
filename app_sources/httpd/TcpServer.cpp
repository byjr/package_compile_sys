#include "TheCommon.h"
#include "HandleTask.h"
#include "TcpServer.h"
#include <bitset>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#define SOCK_MAX_CONN 1024
bool TcpServer::prepare() {
	int ret = 0;
#if 0
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
			show_errno(0, "setsockopt");
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
#else
	mSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(mSocket < 0) {
		show_errno(0, "socket");
		return false;
	}
	struct sockaddr_in addr;
	addr.ai_family = AF_INET;
	addr.sin_port = htons(10080);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(mSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		show_errno(0, "bind");
		close(mSocket);
		return false;
	}
#endif
	if(listen(mSocket, SOCK_MAX_CONN) != 0) {
		show_errno(0, "listen");
		close(mSocket);
		return false;
	}
	return true;
}
bool TcpServer::run() {
	if(set_thread_name(0, "TcpServer") < 0) {
		s_err("set_thread_name:TcpServer failed!");
	}
	mEpFd = epoll_create1(0);
	if(mEpFd == -1) {
		show_errno(0, "epoll_create1");
		return false;
	}
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = mSocket;
	int res = epoll_ctl(mEpFd, EPOLL_CTL_ADD, mSocket, &ev);
	if(res == -1) {
		show_errno(0, "epoll_ctl");
		close(mEpFd);
		return false;
	}
	std::vector<struct epoll_event> events(mPar->maxevents);
	int nReadys = 0;
	for(; !gotExitFlag;) {
		for(int i = 0; i < mPar->retryMax; i++) {
			nReadys = epoll_pwait(mEpFd, events.data(), mPar->maxevents, mPar->timeout_ms, NULL);
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
			if(fd == mSocket) {
				auto pAddr = std::make_unq<sockaddr_t>();
				pAddr->bytes = sizeof(pAddr->addr);
				for(int i = 0; i < mPar->retryMax; i++) {
					conn = accept4(fd, &pAddr->addr, &pAddr->bytes, SOCK_NONBLOCK | SOCK_CLOEXEC);
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
				mTaskHandlerPar = std::make_unq<TaskHandlerPar>();
				mTaskHandlerPar->fd = conn;
				mTaskHandlerPar->epfd = mEpFd;
				mTaskHandlerPar->retryMax = mPar->retryMax;
				mTaskHandlerPar->peerAddr = std::move(pAddr);
				mTaskHandlerPar->plyBuf = mPar->plyBuf;
				mTaskHandlerPar->recBuf = mPar->recBuf;
				auto hd = std::make_shared<TaskHandler>(mTaskHandlerPar);
				if(hd.get() == nullptr) {
					s_err("");
					close(fd);
					break;
				}
				mThMap[conn] = std::move(hd);
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
		s_inf("===mThMap size:%lu", (unsigned long)mThMap.size());
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
	return true;
}

TcpServer::TcpServer(std::unique_ptr<TcpServerPar> &par) {
	gotExitFlag = false;
	isReadyFlag = false;
	mPar = std::move(par);
	if(prepare() == false) {
		s_err("");
		return ;
	}
	mTrd = std::thread([this]()->bool{
		return run();
	});
	isReadyFlag = true;
}

TcpServer::~TcpServer() {
	if(mTrd.joinable()) {
		mTrd.join();
	}
}
// }
