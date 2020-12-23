#ifndef _APP_SOURCES_HTTPD_UTILS_H_
#define _APP_SOURCES_HTTPD_UTILS_H_
#include "android/log.h"
#define HTTPD_TAG "httpd"

#define get_last_name(path) ({\
        size_t i=0;\
        const char* last_lever=path;\
        for(i=0;path[i];i++){\
                if(path[i]=='/'){\
                        last_lever=(char*)((size_t)path+i+1);\
                }\
        }\
        last_lever;\
})

#define s_log(lv,x...) ({\
	char *posInf = NULL, *ctt = NULL,*str = NULL;\
	asprintf(&posInf, "[%s:%d]:%s:", get_last_name(__FILE__),__LINE__,__func__);\
	if(posInf){\
		asprintf(&ctt,x);\
		if(ctt){\
			asprintf(&str,"%s%s",posInf,ctt);\
			if(str){\
				__android_log_print(lv,HTTPD_TAG,"%s",str);\
				free(str);\
			}else{\
				free(posInf);\
			}\
		}else{\
			free(posInf);\
		}\
	}\
})

#define s_slt(x...) s_log(ANDROID_LOG_SILENT,x)
#define s_fat(x...) s_log(ANDROID_LOG_FATAL,x)
#define s_err(x...) s_log(ANDROID_LOG_ERROR,x)
#define s_war(x...) s_log(ANDROID_LOG_WARN,x)
#define s_inf(x...) s_log(ANDROID_LOG_INFO,x)
#define s_dbg(x...) s_log(ANDROID_LOG_DEBUG,x)
#define s_trc(x...) s_log(ANDROID_LOG_VERBOSE,x)

#define show_errno(n,msg) ({\
	int err = n?n:errno;\
	s_err("%s,errno[%d]:%s",#msg,err,strerror(err));\
})

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <fcntl.h>
struct HttpdUtils{
	static ssize_t getBytes(const char* path){
		struct stat lstat;
		int ret = stat ( path, &lstat );
		if ( ret < 0 ) return -1;
		return lstat.st_size;
	}
	static int setTrdName(pthread_t ptid, const char *name) {
		if(!ptid) {
			int res = prctl(PR_SET_NAME, name);
			if(res) {
				s_err("%s/prctl_1", __func__);
				return -1;
			}
		} else {
			int res = pthread_setname_np(ptid, name);
			if(res) {
				s_err("%s/pthread_setname_np", __func__);
				return -1;
			}
		}
		return 0;
	}
	static int setFdFlag(int fd, int flag) {
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
};
	
#endif//_APP_SOURCES_HTTPD_UTILS_H_
