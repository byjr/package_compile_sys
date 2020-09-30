#ifndef __MULTIBOX_BROADCAST_H__
#define __MULTIBOX_BROADCAST_H__
#define __USE_GNU 1
#include <string>
#include <memory>
#include <atomic>
#include <vector>
#include <lzUtils/base.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include<netinet/in.h>
#include <unistd.h>

namespace multlBox {
	static void ParseCallback(const char *data) {
		s_inf("ParseCallback recive data:%s", data);
	}
	class HttpCliPar {
	public:
		std::string addr;
		size_t port;
		size_t reSendItv_ms;
		size_t sendItv_sec;
		std::string userCtt;
		size_t maxRetryCount;
		size_t packMaxBytes;
		void (*parseCallback)(const char *data);
		HttpCliPar() {
			port = 16383;
			reSendItv_ms = 200;
			sendItv_sec = 1;
			maxRetryCount = 50;
			packMaxBytes = 1024;
			parseCallback = &ParseCallback;
			std::string userCtt;
		}
	};
	class HttpCli {
		HttpCliPar *mPar;
		int mFd;
		std::atomic<bool> mKeepRun;
		std::atomic<bool> mIsReady;
		std::string mCtt;
		struct sockaddr_in mSin;
		void Release() {
			if(mKeepRun) {
				mKeepRun = false;
			}
			while(!IsReady());
			if(mFd >= 0) {
				close(mFd);
			}
		}
	public:
		~HttpCli() {
			Release();
		}
		ssize_t SendData(const char *data, size_t size) {
			if(!IsFdWriteable()) {
				return -1;
			}
			int res = write(mFd, data, size);
			if(res < 0) {
				show_errno(0, "write");
				return -1;
			}
			return res;
		}
		bool Send(std::string ctt) {
			size_t retryCount = 0;
			const char *data = ctt.data();
			size_t size = ctt.size();
			for(; size > 0;) {
				s_inf(__func__);
				int res = SendData(data, size);
				if(res <= 0) {
					if(++retryCount > mPar->maxRetryCount) {
						return false;
					}
					continue;
				}
				size -= res;
				data += res;
			}
			return true;
		}
		int IsFdWriteable() {
			fd_set wd;
			FD_ZERO(&wd);
			FD_SET(mFd, &wd);
			int maxfd = mFd + 1;
			struct timeval tv = {0, 1000 * 100};
			int ret = select(maxfd, NULL, &wd, NULL, &tv);
			if(ret < 0) {
				show_errno(0, "select");
				return -1;
			}
			if(!ret) {
				// show_errno(0,"select timer out");
				return -2;
			}
			return 0;
		}
		int IsFdReadable() {
			fd_set rd;
			FD_ZERO(&rd);
			FD_SET(mFd, &rd);
			int maxfd = mFd + 1;
			struct timeval tv = {0, 1000 * 100};
			int ret = select(maxfd, &rd, NULL, NULL, &tv);
			if(ret < 0) {
				show_errno(0, "select");
				return -1;
			}
			if(!ret) {
				// show_errno(0,"select timer out");
				return -2;
			}
			return 0;
		}
		bool CttRecv(std::string &rCtt) {
			if(!IsFdReadable()) {
				return false;
			}
			std::vector<char> buf(256, 0);
			ssize_t res = read(mFd, buf.data(), buf.size());
			if(res <= 0) {
				s_err("CttRecv/read res:%d", res);
				show_errno(0, "read");
				return false;
			}
			rCtt += buf.data();
			return true;
		}
		bool Recv() {
			mCtt = "";
			size_t rertyCount = 0;
			for(; mKeepRun && rertyCount < mPar->maxRetryCount;) {
				bool res = CttRecv(mCtt);
				if(!res) {
					continue;
				}
				if(std::string::npos == mCtt.find("\r\n\r\n")) {
					if(mCtt.size() > mPar->packMaxBytes) {
						break;
					}
					continue;
				}
				return true;
			}
			return false;
		}
		void Loop() {
			mKeepRun = true;
			for(; mKeepRun;) {
				bool res = Send(mCtt);
				if(!res) {
					usleep(mPar->reSendItv_ms * 100);
					continue;
				}
				std::string recvData = "";
				res = Recv();
				if(!res) {
					continue;
				}
				mPar->parseCallback(mCtt.data());
			}
			mIsReady = false;
		}
		void StopRun() {
			mKeepRun = false;
		}
		bool IsReady() {
			return mIsReady;
		}
		bool PrepareCtt(std::string &ctt) {
			ctt = mPar->userCtt;
			return true;
		}
		bool CreateTransfer() {
			mFd = socket(AF_INET, SOCK_STREAM, 0);
			if(mFd < 0) {
				s_err(__func__);
				show_errno(0, "socket");
				return false;
			}
			mSin.sin_family = AF_INET;
			mSin.sin_port = htons(mPar->port);
			mSin.sin_addr.s_addr = inet_addr(mPar->addr.data());
			int res = connect(mFd, (struct sockaddr *)&mSin, sizeof(mSin));
			if(res < 0) {
				s_err(__func__);
				show_errno(0, "connect");
				return false;
			}
			return true;
		}
		HttpCli(HttpCliPar *par):
			mKeepRun{true}, mFd{-1}, mCtt{""} {
			mPar = par;
			bool res = PrepareCtt(mCtt);
			if(!res) {
				s_err("HttpCli /%PrepareCtt failed!");
				return ;
			}
			res = CreateTransfer();
			if(!res) {
				s_err("HttpCli/CreateTransfer failed!");
				Release();
				return ;
			}
			mIsReady = true;
		}
	};
}
#endif
