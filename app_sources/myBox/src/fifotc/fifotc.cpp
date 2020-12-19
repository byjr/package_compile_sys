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
int fifotc_main(int argc,char* argv[]){
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
	FILE* fp = fopen("222.pcm","wb");
	if(!fp){
		show_errno(0,"open");
		return -1;
	}
	#define BUF_BYTES (PIPE_BUF)
	char buf[BUF_BYTES];
	size_t size = sizeof(buf);

	FILE* rfp = fopen(REC_PCM_DUMP_FIFO,"rb");
	if(!rfp){
		show_errno(0,REC_PCM_DUMP_FIFO);
		return -1;		
	}
	int rsize = 0,wsize = 0;
	do{		
		rsize = fread(buf,1,size,rfp);
		if(rsize <=0 ){
			if(feof(rfp)){
				s_war("got eof!");
				break;
			}
			show_errno(0,"open");
			return -1;
		}
		wsize = fwrite(buf,1,rsize,fp);
		if(wsize < rsize){
			show_errno(0,"fwrite");
			return -1;
		}
	}while(!feof(rfp));
	fclose(fp);
	fclose(rfp);
    return 0;
}
