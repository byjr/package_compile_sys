#ifndef __MISC_H__
#define __MISC_H__ 1
#ifndef _WIN_API_
#include <sys/syscall.h>
#endif
#define MAX_USER_COMMAND_LEN     2048

#define FREE(x) if(x){free(x);x=NULL;}

#define CCHIP_MIN(x,y) ((x)<(y)?(x):(y))

#define getCount(array) (sizeof(array)/sizeof(array[0]))

#define ADD_HANDLE_ITEM(name,args) {#name,name##_handle,args},

#define matchProc_startCase(cmd,key) ({char *_cmd = (cmd);if(0==strcmp(_cmd,#key))
#define matchProc_case(key)  else if(0==strcmp(_cmd,#key))
#define matchProc_end() })

#define assert_arg(count,errCode){\
        int err=0;\
        if(!argv[count] || !argv[count][0]){\
                s_err("arg is can't less %d",count);\
                return errCode;\
        }\
}
extern int vfexec ( char *argl, char block );
extern int fexec ( char *argl, char block );
extern int my_popen ( const char *fmt, ... );
extern int my_popen_get ( char *rbuf, int rbuflen, const char *cmd, ... );
int cmd_excute(const char *fmt, ...);
char *cmd_res_get(char *buf, size_t bytes, const char *cmd, ...);
extern int pkill ( const char *name, int sig );
extern size_t get_pids_by_name ( const char *pidName, pid_t **pp_pid, size_t max );
char **argl_to_argv ( char argl[], int *pArgc );
char **argl_dup2_argv(const char argl[], int *pArgc);
char *argv_to_argl ( char *argv[] );
void argv_free ( char *argv[] );
void argv_dup_free(char *argv[]);
char *get_name_cmdline ( char *name, size_t size );
char *get_pid_cmdline ( pid_t pid, size_t size );
int get_tids_by_name ( const char *tid_name, pid_t **pp_pid );
char *getPathtItem ( const char *path, const char *delim, size_t n );
char *cmd_res_get ( char *rBuf, size_t bytes, const char *cmd, ... ) ;

#define base64GetEncodeSafeOutBytes(x) (((x)%3)?((x)/3+1)*4:(x)/3*4)
#define base64GetDecodeSafeOutBytes(x) ((x)*3/4)
size_t base64Encode ( char *bin, size_t binSize, char *base64, size_t baseSize );
size_t base64Decode ( char *base64, size_t baseSize, char *bin, size_t binSize );
int unique_process_lock ( const char *name );
int is_process_has_locked(const char *name);
int WaitOthersInstsExit(const char *name, size_t _10ms);
int openInputByName ( const char *inputNmae );
int set_thread_name(pthread_t ptid, const char *name);
int WaitOthersInstsExit ( const char *name, size_t _10ms );
void argv_shift(char* argv[],size_t n);
#ifdef _WIN_API_
void *memmem ( const void *mp, size_t mb, const void *sp, size_t sb );
#endif
int get_local_ip_by_name(const char *name, char *ip_buf, size_t buf_bytes);
#define showProcessThreadId(msg) ({\
        plog("/tmp/tid.rec","%s:%s,pid=%u,tid=%u",\
                msg,__func__,\
                getpid(),\
                syscall(SYS_gettid)\
        );\
})

#define showCmdResult(showFunc,cmd...) ({\
        char rBuf[1024]="";\
        my_popen_get(rBuf,sizeof(rBuf),cmd);\
        showFunc(rBuf);\
})

#define showMemFree(showFunc) showCmdResult(showFunc,"cat /proc/meminfo | grep MemFree")

#define showHexBuf(date,size) ({\
        int i=0;\
        s_inf("%s hex buf date is:",__func__);\
        for(;i<size;i++){\
                printf("%02hhx ",date[i]);\
        }\
        printf("\n");\
})


#endif /* __PROCUTILS_H__ */
