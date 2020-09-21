#ifndef __MULTIBOX_BROADCAST_H__
#define __MULTIBOX_BROADCAST_H__
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


namespace multlBox {
	class BroadcastPar{
	public:		
		size_t port;
		size_t reSendItv_ms;
		size_t sendItv_sec;
		std::string userCtt; 
		size_t maxRetryCount;
		BroadcastPar(){
			port = 16383;
			reSendItv_ms = 200;
			sendItv_sec = 1;
			maxRetryCount = 50;
		}
	};
	class Broadcast {
		BroadcastPar* mPar;
		int mTransfer;
		std::atomic<bool> mIsRunning;
		std::atomic<bool> mIsLoopExited;
		std::string mCtt;
		struct sockaddr_in mSin;
	public:
		~Broadcast(){
			Release();	
		}
		void Release(){
			mIsRunning = false;
			while(!IsLoopExited());
			if(mTransfer >= 0){
				close(mTransfer);
			}
		}
		ssize_t SendData(const char* data,size_t size){s_inf(__func__);
			int res = sendto(mTransfer,data,size,0,(struct sockaddr*)&mSin, sizeof(struct sockaddr));
			if(res < 0){
				show_errno(0,"sendto");
				return -1;
			}
			return res;
		}
		bool SendPack(std::string ctt){s_inf(__func__);
			size_t retryCount = 0;
			const char* data = ctt.data();
			size_t size = ctt.size();
			for(;size > 0;){s_inf(__func__);
				int res = SendData(data,size);
				if(res <= 0){
					if(++retryCount > mPar->maxRetryCount){
						return false;
					}
					continue;
				}
				size -= res;
				data += res;
			}
			return true;
		}
		void SendLoop(){
			for(;mIsRunning;){s_inf(__func__);
				bool res = SendPack(mCtt);
				if(!res){
					usleep(mPar->reSendItv_ms*100);
					continue;
				}
				sleep(mPar->sendItv_sec);
			}
		}
		void Loop(){
			for(;mIsRunning;){
				bool res = SendPack(mCtt);
				if(!res){
					usleep(mPar->reSendItv_ms*100);
					continue;
				}
				sleep(mPar->sendItv_sec);
			}
		}		
		bool IsRunning(){
			return mIsRunning;
		}
		bool IsLoopExited(){
			return mIsLoopExited;
		}			
		bool PrepareCtt(std::string& ctt){
			ctt = mPar->userCtt;
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
			int res = setsockopt(mTransfer, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, (char*)&bOpt, sizeof(bOpt));
			if(res < 0){
				s_err(__func__);
				show_errno(0,"setsockopt");
				return false;
			}
			mSin.sin_family = AF_INET;
			mSin.sin_port = htons(mPar->port);
			mSin.sin_addr.s_addr = INADDR_BROADCAST;				
			return true;
		}
		Broadcast(BroadcastPar* par):
				mIsRunning{false},mTransfer{-1},mCtt{""}{
			mPar = par;
			bool res = PrepareCtt(mCtt);
			if(!res){
				s_err("Broadcast /%PrepareCtt failed!");
				return ;
			}
			res = CreateTransfer();
			if(!res){
				s_err("Broadcast/CreateTransfer failed!");
				Release();
				return ;			
			}
			mIsRunning = true;		
		}	
	};
}
#endif
