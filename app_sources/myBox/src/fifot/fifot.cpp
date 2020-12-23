// unordered_map::erase

#include <iostream>
#include <string>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <thread>
#include <atomic>
#include <lzUtils/base.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#define PLY_PCM_DUMP_FIFO "/tmp/ply.fifo"
#define REC_PCM_DUMP_FIFO "/tmp/rec.fifo"
using namespace std::chrono;
enum class SocktState{
	min = -1,
	rAble,
	wAble,
	closed,
	max
};
static int help_info(int argc, char *argv[]) {
	s_err("%s help:", get_last_name(argv[0]));
	s_err("\t-i [input path]");
	s_err("\t-o [output url]");
	s_err("\t-l [logLvCtrl]");
	s_err("\t-p [logPath]");
	s_err("\t-h show help");
	return 0;
}
extern "C"{
static int fd_set_flag(int fd, int flag) {
	int flags = fcntl(fd, F_GETFL, 0);
	if(flags < 0) {
		show_errno(0, "fcntl F_GETFL failed!");
		return -1;
	}
	int ret = fcntl(fd, F_SETFL, flags | flag);
	if(ret < 0) {
		show_errno(0, "fcntl F_SETFL failed!");
		return -2;
	}
	return 0;
}	


static bool writeData(int fd,const char* data,size_t size){
	ssize_t res = 0;
	size_t count  = 0,retryMax = 5;
	for(size_t i = 0;count < size ;){
		res = write(fd,data+count,size-count);
		if(res <= 0){
			if(errno == EINTR){
				usleep(100);
				continue;
			}
			if(errno == EAGAIN){
				if(++i < retryMax){
					usleep(1000);
					continue;
				}
			}
			show_errno(0,"write!!!");
			return false;
		}
		count += res;
	}	
	return true;
}
static bool atomicWrite(int fd,const char* data,size_t size){
	size_t count  = 0;
	int rem = size;
	for(;count < size ;){
		rem = size-count;
		rem = rem < PIPE_BUF ? rem : PIPE_BUF;
		if(!writeData(fd,data+count,rem)){
			return false;
		}
		count += rem;		
	}
	return true;
}

static bool dupPcmData(int fd,const void* buf,size_t bytes){
	if(fd < 0){
		return false;
	}
	return atomicWrite(fd,(const char*)buf,bytes);
}
}
static int mRecBackupFd = -2;
int fifot_main(int argc,char* argv[]){
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
	if(access(REC_PCM_DUMP_FIFO,F_OK)){
		if(mkfifo(REC_PCM_DUMP_FIFO,0666)){
			show_errno(0, REC_PCM_DUMP_FIFO);
			return -1;
		}
	}
	if(mRecBackupFd == -2){
		mRecBackupFd = open(REC_PCM_DUMP_FIFO, O_CREAT|O_RDWR, 0666);
		if(mRecBackupFd < 0){
		 show_errno(0, "open:");
		 return -1;
		}
		if(fd_set_flag(mRecBackupFd,O_NONBLOCK)){
		 return -1;
		}
		s_err("REC_PCM_DUMP_FIFO(%s) open ok!",REC_PCM_DUMP_FIFO);
	}
	for(int i=0;i< 100;i++){
		getchar();
		s_war("start to run %d times ...",i);
		FILE* fp = fopen("111.pcm","rb");
		if(!fp){
			show_errno(0,"open");
			return -1;
		}
		#define BUF_BYTES (4096)
		char buf[BUF_BYTES];
		size_t size = sizeof(buf);
		int res = 0;
		do{
			res = fread(buf,1,size,fp);
			if(res <=0 ){
				if(feof(fp)){
					s_war("got eof!");
					break;
				}
				show_errno(0,"open");
				return -1;
			}
			dupPcmData(mRecBackupFd,buf,res);
		}while(!feof(fp));
		fclose(fp);		
	}
    return 0;
}
