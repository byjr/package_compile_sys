#ifndef __MULTIBOX_RecvBc_H__
#define __MULTIBOX_RecvBc_H__
#define __USE_GNU 1
#include <string>
#include <memory>
#include <atomic>
#include <lzUtils/base.h>
extern "C"{
#include <sys/types.h>
#include <sys/socket.h>	
#include<netinet/in.h>
#include <unistd.h>
}
static void ParseCallback(const char* data){
	s_inf("ParseCallback recive data:%s",data);
}
namespace multlBox {
	class RecvBcPar{
	public:		
		size_t port;
		size_t packMaxBytes;
		void (*parseCallback)(const char* data);
		RecvBcPar(){
			port = 16383;
			packMaxBytes = 1024;
			parseCallback = &ParseCallback;
		}
	};
	class RecvBc {
		RecvBcPar* mPar;
		int mTransfer;
		std::atomic<bool> mIsRunning;
		std::atomic<bool> mIsLoopExited;
		std::string mCtt;
		struct sockaddr_in mSin;
		size_t mSinLen;
	public:
		~RecvBc(){
			Release();
		}
		void Release(){
			mIsRunning = false;
			while(!IsLoopExited());
			if(mTransfer >= 0){
				close(mTransfer);
			}
		}
		int HadDataArrived() {
			fd_set rd;
			FD_ZERO(&rd);
			FD_SET(mTransfer, &rd);
			int maxfd = mTransfer + 1;
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
		bool CttRecv(std::string& ctt){
			static char buf[512] = {0};
			memset(buf,0,sizeof(buf));
			ssize_t res = recvfrom(mTransfer, buf, sizeof(buf), 0, (struct sockaddr *)&mSin, &mSinLen);
			if(res <= 0){
				s_err("CttRecv/recvfrom res:%d",res);
				show_errno(0,"recvfrom");
				return false;
			}
			ctt += buf;
			return true;
		}
		bool Recv(){
			bool res = false;			
			mCtt = "";
			for(;mIsRunning;){
				int ret = HadDataArrived();
				if(ret < 0){
					continue;
				}
				res = CttRecv(mCtt);
				if(!res){
					break;
				}
				if(std::string::npos == mCtt.find("\r\n\r\n")){
					if(mCtt.size() > mPar->packMaxBytes){
						break;
					}
					continue;
				}
				return true;
			}
			return false;
		}
		void Loop(){s_inf(__func__);
			mIsLoopExited = false;
			mIsRunning = true;
			for(;mIsRunning;){
				bool res = Recv();
				if(!res){
					continue;
				}
				mPar->parseCallback(mCtt.data());
			}
			mIsLoopExited = true;
		}		
		bool IsRunning(){
			return mIsRunning;
		}
		bool IsLoopExited(){
			return mIsLoopExited;
		}		
		bool ParseCtt(std::string& ctt){
			s_war(ctt.data());
			return true;
		}
		bool CreateTransfer(){
			mTransfer = socket(AF_INET, SOCK_DGRAM, 0);
			if(mTransfer < 0){
				s_err(__func__);
				show_errno(0,"socket");
				false;
			}
			int bOpt = true;
			int res = setsockopt(mTransfer, SOL_SOCKET, SO_REUSEADDR, (char*)&bOpt, sizeof(bOpt));
			if(res < 0){
				s_err(__func__);
				show_errno(0,"setsockopt");
				return false;
			}
			memset(&mSin,0,sizeof(mSin));
			mSin.sin_family = AF_INET;
			mSin.sin_port = htons(mPar->port);
			mSin.sin_addr.s_addr = INADDR_ANY;	
			mSinLen	= sizeof(mSin);
			res = bind(mTransfer,(struct sockaddr *)&mSin,mSinLen);
			if(res < 0){
				s_err(__func__);
				show_errno(0,"bind");
				return false;
			}
			return true;
		}
		RecvBc(RecvBcPar* par):
				mIsRunning{false},mTransfer{-1},mCtt{""}{
			mPar = par;
			bool res = CreateTransfer();
			if(!res){
				s_err("RecvBc/CreateTransfer failed!");
				Release();
				return ;			
			}
			mIsRunning = true;		
		}	
	};
}
#endif
