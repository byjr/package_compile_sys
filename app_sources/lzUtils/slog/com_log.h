#ifdef __cplusplus
extern "C" {
#endif

#ifndef _COM_LOG_H_
#define _COM_LOG_H_ 1

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <sys/time.h>

#define _STR(s)     #s
#define STR(s)      _STR(s)

#define Hgray   "\033[1;30m"
#define Lgray   "\033[0;30m"

#define Hred    "\033[1;31m"
#define Lred    "\033[0;31m"

#define Hgreen  "\033[1;32m"
#define Lgreen  "\033[0;32m"

#define Hyellow "\033[1;33m"
#define Lyellow "\033[0;33m"

#define Hblue   "\033[1;34m"
#define Lblue   "\033[0;34m"

#define Hpurple "\033[1;35m"
#define Lpurple "\033[0;35m"

#define Hindigo "\033[1;36m"
#define Lindigo "\033[0;36m"

#define Hwhite  "\033[1;37m"
#define Lwhite  "\033[0;37m"

#define ColEnd  "\033[0m"

#define TELNET_PATH      "/dev/pts/0"
#define CONSOLE_PATH "/dev/console"
#define LOG_OUT_PATH NULL//CONSOLE_PATH

#ifdef _WIN_API_
#define get_last_name(path) ({\
        int i=0;\
        const char* last_lever=path;\
        for(i=0;path[i];i++){\
                if(path[i]=='\\'){\
                        last_lever=path+i+1;\
                }\
        }\
        last_lever;\
})
#else
#define get_last_name(path) ({\
        int i=0;\
        const char* last_lever=path;\
        for(i=0;path[i];i++){\
                if(path[i]=='/'){\
                        last_lever=path+i+1;\
                }\
        }\
        last_lever;\
})
#endif

#define clog(color,fmt,args...) ({ \
    FILE *fp = LOG_OUT_PATH?fopen(LOG_OUT_PATH, "a"):NULL; \
        if(!fp)fp=stdout;\
        fprintf(fp,"%s[%s %d]:\033[0m",color,get_last_name(__FILE__),__LINE__); \
        fprintf(fp,fmt,##args); \
        fprintf(fp,"\n"); \
        if(fp!=stdout)fclose(fp); \
})

#define plog(path,fmt, args...) ({ \
    FILE *fp = fopen(path,"a"); \
    if (fp) { \
                fprintf(fp,"[%s %d]",get_last_name(__FILE__),__LINE__); \
        fprintf(fp,fmt,##args); \
                fprintf(fp,"\n"); \
        fclose(fp); \
    } \
})

#define show_str_nbytes(addr,bytes,func) ({ \
        size_t len = strlen(addr);\
        len = bytes < len ? bytes : len;\
        char* buf = (char*)malloc(len+1);\
        if(!buf){\
                func("oom");\
        }else{\
                memcpy(buf,addr,len);\
                buf[len]=0;\
                func(buf);\
                free(buf);\
        }\
})

#define cslog(x...) clog(Hindigo,x)
#endif

#ifdef __cplusplus
}
#endif
