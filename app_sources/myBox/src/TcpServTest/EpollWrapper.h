#ifndef _EpollWrapper_H_
#define _EpollWrapper_H_
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <lzUtils/base.h>
struct EpollWrapper{
	static bool add_evt(int epfd,int fd,int flag){
		struct epoll_event ev;
		ev.events = flag;
		ev.data.fd = fd;		
		s_war("%s:epfd=%d,fd=%d,flag=%d",__func__,epfd,fd,flag);		
		int res = epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);
		if(res < 0){
			show_errno(0, "epoll_ctl");
			return false;				
		}
		return true;
	}
	static bool mod_evt(int epfd,int fd,int flag){
		struct epoll_event ev;
		ev.events = flag;
		ev.data.fd = fd;
		s_war("%s:epfd=%d,fd=%d,flag=%d",__func__,epfd,fd,flag);
		int res = epoll_ctl(epfd,EPOLL_CTL_MOD,fd,&ev);
		if(res < 0){
			show_errno(0, "epoll_ctl");
			return false;				
		}
		return true;
	}
	static bool del_evt(int epfd,int fd,int flag=0){
		struct epoll_event ev;
		ev.events = flag;
		ev.data.fd = fd;
		s_war("%s:epfd=%d,fd=%d,flag=%d",__func__,epfd,fd,flag);		
		int res = epoll_ctl(epfd,EPOLL_CTL_DEL,fd,&ev);
		if(res < 0){
			show_errno(0, "epoll_ctl");
			return false;				
		}
		return true;
	}	
};
#endif