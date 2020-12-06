#define _GNU_SOURCE
#include <stdarg.h>
#include <sys/types.h>
#ifndef _WIN_API_
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <syscall.h>
#include <linux/input.h>
#endif
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>

#include "fp_op.h"
#include "fd_op.h"
#include "misc.h"
#include "../slog/slog.h"
#ifndef _WIN_API_
int openInputByName ( const char *inputName ) {
	int fd = -1;
	const char *dirname = "/dev/input";
	char devname[512] = { 0 };
	struct dirent *de;
	DIR *dir = opendir ( dirname );
	if ( dir == NULL ) {
		show_errno ( 0, opendir );
		return -1;
	}
	strcpy ( devname, dirname );
	char *filename = devname + strlen ( devname );
	*filename++ = '/';
	while ( ( de = readdir ( dir ) ) ) {
		if ( de->d_name[0] == '.' &&
				( de->d_name[1] == '\0' ||
				  ( de->d_name[1] == '.' && de->d_name[2] == '\0' ) ) )
			continue;
		strcpy ( filename, de->d_name );
		fd = open ( devname, O_RDONLY );
		if ( fd >= 0 ) {
			char name[128] = {0};
			if ( ioctl ( fd, EVIOCGNAME ( sizeof ( name ) - 1 ), &name ) < 1 ) {
				name[0] = '\0';
			}
			if ( !strcmp ( name, inputName ) ) {
				break;
			} else {
				close ( fd );
				fd = -1;
			}
		}
	}
	closedir ( dir );
	return fd;
}

int get_tids_by_name ( const char *tid_name, pid_t **pp_pid ) {
	char filename[1024];
	char got_name[256];
	if ( ! ( tid_name && tid_name[0] ) ) {
		printf ( "ERR:Plz input correct thread name by argv[1]\n" );
		return -1;
	}
	DIR *dir = opendir ( "/proc" );
	if ( !dir ) {
		perror ( "Cannot open /proc\n" );
		return -1;
	}
	pid_t *p_pid = NULL;
	struct dirent *next = NULL;
	int count = 0, failure = 0;
	for ( ; ( next = readdir ( dir ) ) && !failure ; ) {
		if ( !isdigit ( *next->d_name ) ) continue; //排除非数字
		memset ( filename, 0, sizeof ( filename ) );
		snprintf ( filename, sizeof ( filename ) - 1, "/proc/%s/task", next->d_name );
		DIR *tdir = opendir ( ( const char * ) filename );
		if ( !tdir ) {
			perror ( "opendir\n" );
			break;
		}
		struct dirent *next_t = NULL;
		for ( ; ( next_t = readdir ( tdir ) ); ) {
			if ( !isdigit ( *next_t->d_name ) ) continue; //排除非数字
			memset ( filename, 0, sizeof ( filename ) );
			snprintf ( filename, sizeof ( filename ) - 1, "/proc/%s/task/%s/status", next->d_name, next_t->d_name );
			FILE *fp = fopen ( filename, "r" );
			if ( !fp ) {
				continue;
			}
			do {
				memset ( filename, 0, sizeof ( filename ) );
				char *res = fgets ( filename, sizeof ( filename ) - 1, fp );
				if ( !res ) {
					break;
				}
				memset ( got_name, 0, sizeof ( got_name ) );
				sscanf ( filename, "%*s %s", got_name );
				if ( strcmp ( got_name, tid_name ) ) {
					break;
				}
				if ( pp_pid ) {
					p_pid = realloc ( p_pid, sizeof ( pid_t ) * ( count + 1 ) );
					if ( !p_pid ) {
						printf ( "ERR:realloc\n" );
						failure = 1;
						break;
					}
					p_pid[count] = atoi ( next_t->d_name );
				}
				count ++;
			} while ( 0 );
			fclose ( fp );
		}
		closedir ( tdir );
	}
	if ( pp_pid ) *pp_pid = p_pid;
	closedir ( dir );
	if ( failure ) {
		return -1;
	}
	return count;
}
size_t get_pids_by_name ( const char *pidName, pid_t **pp_pid, size_t max ) {
	char filename[512];
	char name[128];
	struct dirent *next;
	FILE *file;
	DIR *dir;
	int fail = 1;
	pid_t *p_pid = NULL;
	dir = opendir ( "/proc" );
	if ( !dir ) {
		return -1;
		perror ( "Cannot open /proc\n" );
	}
	int count = 0;
	for ( count = 0; ( next = readdir ( dir ) ) != NULL && count < max; ) {
		/* Must skip ".." since that is outside /proc */
		if ( strcmp ( next->d_name, ".." ) == 0 )
			continue;

		/* If it isn't a number, we don't want it */
		if ( !isdigit ( *next->d_name ) )
			continue;

		memset ( filename, 0, sizeof ( filename ) );
		sprintf ( filename, "/proc/%s/status", next->d_name );
		if ( ! ( file = fopen ( filename, "r" ) ) )
			continue;

		memset ( filename, 0, sizeof ( filename ) );
		if ( fgets ( filename, sizeof ( filename ) - 1, file ) != NULL ) {
			/* Buffer should contain a string like "Name:   binary_name" */
			sscanf ( filename, "%*s %s", name );
			if ( !strcmp ( name, pidName ) ) {
				if ( pp_pid ) {
					if ( ( p_pid = realloc ( p_pid, sizeof ( pid_t ) * ( count + 1 ) ) ) == NULL ) {
						s_err ( "realloc fail!" );
						fclose ( file );
						goto exit;
					}
					p_pid[count] = strtol ( next->d_name, NULL, 0 );
				}
				count++;
			}
		}
		fail = 0;
		if ( pp_pid ) *pp_pid = p_pid;
		fclose ( file );
	}
exit:
	closedir ( dir );
	if ( fail && count <= 0 ) return -1;
	return count;
}
int pkill ( const char *name, int sig ) {
	int ret = 0;
	pid_t *p_pid = NULL;
	if ( !name ) {
		s_err ( "name:%s is invalid!", name );
		return -1;
	}
	size_t count = get_pids_by_name ( name, &p_pid, 1 );
	if ( ! ( count > 0 && p_pid ) ) {
		s_err ( "get_pids_by_name %s failure!", name );
		return -2;
	}
	ret = kill ( p_pid[0], sig );
	FREE ( p_pid );
	if ( ret < 0 ) return -3;
	return 0;
}
#endif
static pthread_mutex_t popen_mtx = PTHREAD_MUTEX_INITIALIZER;
int my_popen ( const char *fmt, ... ) {
	char buf[MAX_USER_COMMAND_LEN] = "";
	char cmd[MAX_USER_COMMAND_LEN] = "";
	FILE *pfile;
	__attribute__((unused))int status = -2;
	pthread_mutex_lock ( &popen_mtx );
	va_list args;
	va_start ( args, fmt );
	vsprintf ( cmd, fmt, args );
	va_end ( args );
	//          s_inf("%s",cmd);
	if ( ( pfile = popen ( cmd, "r" ) ) ) {
#ifndef _WIN_API_
		fcntl ( fileno ( pfile ), F_SETFD, FD_CLOEXEC );
#endif
		while ( !feof ( pfile ) ) {
			fgets ( buf, sizeof buf, pfile );
		}
		//              s_inf("%s",buf);
		status = pclose ( pfile );
	}
	pthread_mutex_unlock ( &popen_mtx );
#ifndef _WIN_API_
	if ( WIFEXITED ( status ) ) {
		return WEXITSTATUS ( status );
	}
	return -1;
#else
	return 0;
#endif // _WIN_API_
}
int my_popen_get ( char *rbuf, int rbuflen, const char *cmd, ... ) {
	char buf[MAX_USER_COMMAND_LEN];
	va_list args;
	FILE *pfile;
	__attribute__((unused))int status = -2;
	char *p = rbuf;

	rbuflen = ( !rbuf ) ? 0 : rbuflen;

	va_start ( args, ( char * ) cmd );
	vsnprintf ( buf, sizeof ( buf ), cmd, args );
	va_end ( args );

	pthread_mutex_lock ( &popen_mtx );
	if ( ( pfile = popen ( buf, "r" ) ) ) {
#ifndef _WIN_API_
		fcntl ( fileno ( pfile ), F_SETFD, FD_CLOEXEC );
#endif
		while ( !feof ( pfile ) ) {
			if ( ( rbuflen > 0 ) && fgets ( buf, CCHIP_MIN ( rbuflen, sizeof ( buf ) ), pfile ) ) {
				int len = snprintf ( p, rbuflen, "%s", buf );
				rbuflen -= len;
				p += len;
			} else {
				break;
			}
		}
		if ( ( rbuf ) && ( p != rbuf ) && ( * ( p - 1 ) == '\n' ) ) {
			* ( p - 1 ) = 0;
		}
		status = pclose ( pfile );
	}
	pthread_mutex_unlock ( &popen_mtx );
#ifndef _WIN_API_
	if ( WIFEXITED ( status ) ) {
		return WEXITSTATUS ( status );
	}
	return -1;
#else
	return 0;
#endif // _WIN_API_
}

char *cmd_res_get ( char *buf, size_t bytes, const char *cmd, ... ) {
	//组装命令行参数
	va_list args;
	va_start ( args, ( char * ) cmd );
	char *cmdBuf = NULL;
	vasprintf ( &cmdBuf, cmd, args );
	va_end ( args );
	//打开命令执行句柄
	FILE *fp = popen ( cmdBuf, "r" );
	if ( !fp ) {
		show_errno ( 0, "cmd_res_get/popen" );
		free ( cmdBuf );
		return NULL;
	}
	free ( cmdBuf ); //释放命令行参数buffer
#ifndef _WIN_API_
	//设置命令行句柄属性
	fcntl ( fileno ( fp ), F_SETFD, FD_CLOEXEC );
#endif
#define CMD_RES_GET_RBUF_SIZE 128
	size_t count_bytes = 0;         //累计读到了的字节数
	char *rBuf = NULL;              //结果内容首地址
	size_t rbytes = 0;              //要读的字节数
	char *wIdx = NULL;              //（写索引）每次从这里开始写
	ssize_t res = 0;                //返回值
	do {
		//确定本次buffer中能写的字节数
		rbytes = count_bytes + CMD_RES_GET_RBUF_SIZE < bytes - 1 ?
				 CMD_RES_GET_RBUF_SIZE : bytes - count_bytes;

		if ( !buf ) { //如果外面没有给定buffer,就分配buffer
			rBuf = realloc ( rBuf, count_bytes + rbytes + 1 );
			if ( !rBuf ) {
				show_errno ( 0, "cmd_res_get/malloc" );
				return NULL;
			}
		} else { //外部已经给定内存
			rBuf = buf;
		}
		wIdx = rBuf + count_bytes;//更新写索引
		res = fread ( wIdx, 1, rbytes, fp ); //读结果
		if ( res < rbytes ) {
			if ( !feof ( fp ) ) {
				show_errno ( 0, "cmd_res_get/popen" );
				return NULL;
			}
		}
		if ( count_bytes >= bytes ) {
			break;
		}
		count_bytes += res;
	} while ( !feof ( fp ) );
	fclose ( fp );
	rBuf[count_bytes] = 0;//写入字符串结束符
	return rBuf;
}
int cmd_excute(const char *fmt, ...) {
	int ret = -1;
	char *cmd = NULL;
	va_list args;
	va_start(args, fmt);
	vasprintf(&cmd, fmt, args);
	va_end(args);
	if(!cmd) {
		s_err("%s/vasprintf oom", __func__);
		goto exit;
	}
	int res = system(cmd);
	if(res == -1) {
		s_err("cmd_excute:%s can't be excuted!",cmd);
		goto exit;
	}
#ifndef _WIN_API_
	if(!WIFEXITED(res)) {
		s_err("cmd_excute:%s abort!",cmd);
		goto exit;
	}
	ret = WEXITSTATUS(res);
#endif
	s_dbg("cmd_excute: %s exited:%d",cmd,ret);
exit:
	if(cmd) {
		free(cmd);
	}
	return ret;
}
void argv_free ( char *argv[] ) {
	if ( argv ) {
		free ( argv );
	}
}
char *argv_to_argl ( char *argv[] ) {
	int i = 0, rt_len = 0, rt_idx = 0;
	char *argl = NULL;
	if ( ! ( argv && argv[0] ) ) return NULL;
	for ( i = 0;; i++ ) {
		if ( !argv[i] ) break;
		rt_len = strlen ( argv[i] );
		rt_idx += rt_len;
		argl = ( char * ) realloc ( argl, rt_idx + 2 );
		if ( !argl ) return NULL;
		strcat ( argl, argv[i] );
		argl[rt_idx++] = ' ';
	}
	argl[rt_idx - 1] = '\0';
	return argl;
}
char **argl_to_argv ( char argl[], int *pArgc ) {
	int i = 0, count = 0;
	if ( ! ( argl && argl[0] ) ) return NULL;
	char **argv = ( char ** ) calloc ( 1, sizeof ( char * ) );
	if ( !argv ) return NULL;
	argv[0] = argl;
	for ( i = 0;; i++ ) {
		if ( argl[i] == ' ' || argl[i] == '\n' || argl[i] == '\0' ) {
			while ( argl[i + 1] == ' ' ) {
				i++;
			}
			count++;
			argv = ( char ** ) realloc ( argv, ( count + 1 ) * sizeof ( char * ) );
			if ( !argv ) return NULL;
			argv[count] = argl + i + 1;
			if ( argl[i] == '\0' || argl[i] == '\n' ) {
				if ( argl[i] == '\n' ) {
					argl[i] = 0;
				}
				break;
			}
			argl[i] = 0;
		}
	}
	argv[count] = NULL;
	if ( pArgc ) {
		*pArgc = count;
	}
	return argv;
}
void argv_dup_free(char *argv[]) {
	int i = 0;
	for(i = 0; argv[i]; i++) {
		free(argv[i]);
	}
	free(argv);
	argv = NULL;
}
char **argl_dup2_argv(const char argl[], int *pArgc) {
	int i = 0, count = 0;
	if(!(argl && argl[0]))return NULL;
	char **argv = (char **)calloc(1, sizeof(char *));
	if(!argv)return NULL;
	argv[0] = (char *)argl;
	for(i = 0;; i++) {
		if(argl[i] == ' ' || argl[i] == '\n' || argl[i] == '\0' ) {
			while(argl[i] == ' ' && argl[i + 1] == ' ') {
				i++;
			}
			count++;
			argv = (char **)realloc(argv, (count + 1) * sizeof(char *));
			if(!argv)return NULL;
			argv[count] = (char *)argl + i + 1;
			char *arg = (char *)calloc(1, argv[count] - argv[count - 1]);
			if(!arg) {
				argv_dup_free(argv);
				return NULL;
			}
			strncpy(arg, argv[count - 1], argv[count] - argv[count - 1] - 1);
			argv[count - 1] = arg;
		}
		if(argl[i] == '\0' || argl[i] == '\n')break;
	}
	argv[count] = NULL;
	if(pArgc) {
		*pArgc = count;
	}
	return argv;
}
char *dup_argl_cmd(const char *argl) {
	int i = 0;
	for(i = 0; argl[i]; i++) {
		if(argl[i] == ' ' || argl[i] == '\n' || argl[i] == '\0' ) {
			break;
		}
	}
	char *cmd = strndup(argl, i + 2);
	if(!cmd) {
		s_err("%s/strndup", __func__);
		return NULL;
	}
	return cmd;
}
void argv_shift(char* argv[],size_t n){
	int i = 0;
	for(;argv[i+n];i++){
		argv[i]=argv[i+n];
	}
}
#ifndef _WIN_API_
int vfexec ( char *argl, char isBlock ) {
	int ret = -1;
	char **argv = argl_to_argv ( argl, NULL );
	if ( ! ( argv && argv[0] ) ) goto exit;
	pid_t pid = vfork();
	if ( pid < 0 ) goto exit;
	if ( pid == 0 ) {
		int ret = execvp ( argv[0], argv );
		if ( ret < 0 ) {
			show_errno ( 0, "execv" );
			goto exit;
		}
	} else {

		int ws = 0;
		int ret = isBlock ? pid = waitpid ( pid, &ws, 0 ) : waitpid ( pid, &ws, WNOHANG );
		if ( ret < 0 && errno != ECHILD ) {
			show_errno ( 0, "waitpid" );
			goto exit;
		}
		ret = 0;
	}
exit:
	if ( argv ) argv_free ( argv );
	return ret < 0 ? -1 : 0;
}

int fexec ( char *argl, char isBlock ) {
	int ret = -1;
	char **argv = argl_to_argv ( argl, NULL );
	if ( ! ( argv && argv[0] ) ) goto exit;
	pid_t pid = fork();
	if ( pid < 0 ) goto exit;
	if ( pid == 0 ) {
		int ret = execvp ( argv[0], argv );
		if ( ret < 0 ) {
			show_errno ( 0, "execv" );
			goto exit;
		}
	} else {
		int ws = 0;
		int ret = isBlock ? pid = waitpid ( pid, &ws, 0 ) : waitpid ( pid, &ws, WNOHANG );
		if ( ret < 0 && errno != ECHILD ) {
			show_errno ( 0, "waitpid" );
			goto exit;
		}
		ret = 0;
	}
exit:
	if ( argv ) argv_free ( argv );
	return ret < 0 ? -1 : 0;
}
#endif
//编码目标字符集
static const char *base64Char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
//长度关系 (y：输出长度，x:输入长度):y=(x%3)?(x/3+1)*4:x/3*4 考虑传输效率将每次编解码的二进制字节数暂定为：(1024/4-1)*3=765
size_t base64Encode ( char *bin, size_t binSize, char *base64, size_t baseSize ) {
	int i, j;
	char current;
	for ( i = 0, j = 0 ; i < binSize ; i += 3 ) {
		if ( j + 4 >= baseSize ) {
			s_err ( "out buf is't enough,alredy en %u bytes,output %u bytes !", i, j );
			base64[j] = '\0';
			return -1;
		}
		current = ( bin[i] >> 2 ) ;
		current &= ( char ) 0x3F;
		base64[j++] = base64Char[ ( int ) current];

		current = ( ( char ) ( bin[i] << 4 ) ) & ( ( char ) 0x30 ) ;
		if ( i + 1 >= binSize ) {
			base64[j++] = base64Char[ ( int ) current];
			base64[j++] = '=';
			base64[j++] = '=';
			break;
		}
		current |= ( ( char ) ( bin[i + 1] >> 4 ) ) & ( ( char ) 0x0F );
		base64[j++] = base64Char[ ( int ) current];

		current = ( ( char ) ( bin[i + 1] << 2 ) ) & ( ( char ) 0x3C ) ;
		if ( i + 2 >= binSize ) {
			base64[j++] = base64Char[ ( int ) current];
			base64[j++] = '=';
			break;
		}
		current |= ( ( char ) ( bin[i + 2] >> 6 ) ) & ( ( char ) 0x03 );
		base64[j++] = base64Char[ ( int ) current];

		current = ( ( char ) bin[i + 2] ) & ( ( char ) 0x3F ) ;
		base64[j++] = base64Char[ ( int ) current];
	}
	base64[j] = '\0';
	return j;
}
size_t base64Decode ( char *base64, size_t baseSize, char *bin, size_t binSize ) {
	int i = 0, j = 0, k = 0;
	char temp[4] = {0};
	if ( ( baseSize / 4 - 1 ) * 3 > binSize ) {
		s_err ( "out buf is't enough,alredy encode %u bytes,output %u bytes !", i, j );
		return -1;
	}
	for ( i = 0, j = 0; i < baseSize ; i += 4 ) {
		memset ( temp, 0xFF, sizeof ( temp ) );
		for ( k = 0 ; k < 64 ; k ++ ) {
			if ( base64Char[k] == base64[i] )
				temp[0] = k;
		}
		for ( k = 0 ; k < 64 ; k ++ ) {
			if ( base64Char[k] == base64[i + 1] )
				temp[1] = k;
		}
		for ( k = 0 ; k < 64 ; k ++ ) {
			if ( base64Char[k] == base64[i + 2] )
				temp[2] = k;
		}
		for ( k = 0 ; k < 64 ; k ++ ) {
			if ( base64Char[k] == base64[i + 3] )
				temp[3] = k;
		}

		bin[j++] = ( ( char ) ( ( ( char ) ( temp[0] << 2 ) ) & 0xFC ) ) |
				   ( ( char ) ( ( char ) ( temp[1] >> 4 ) & 0x03 ) );
		if ( base64[i + 2] == '=' )
			break;

		bin[j++] = ( ( char ) ( ( ( char ) ( temp[1] << 4 ) ) & 0xF0 ) ) |
				   ( ( char ) ( ( char ) ( temp[2] >> 2 ) & 0x0F ) );
		if ( base64[i + 3] == '=' )
			break;

		bin[j++] = ( ( char ) ( ( ( char ) ( temp[2] << 6 ) ) & 0xF0 ) ) |
				   ( ( char ) ( temp[3] & 0x3F ) );
	}
	return j;
}
#ifndef _WIN_API_
int unique_process_lock(const char *name) {
	int ret = -1;
	char *path = NULL;
	asprintf(&path, "/var/run/%s.pid", get_last_name(name));
	if(!path) {
		s_err("%s/asprintf oom", __func__);
		exit(-1);
	}
	int fd = open(path, O_WRONLY | O_CREAT, 0666);
	if (fd < 0) {
		disp_syserr(0, fcntl);
		s_err("Fail to open %s", path);
		exit(-1);
	}
	struct flock lock = {0};
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	if (fcntl(fd, F_SETLK, &lock) < 0) {
		disp_syserr(0, fcntl);
		s_err("Fail to fcntl %s F_SETLK", name);
		goto exit;
	}
	char *ctx = NULL;
	asprintf(&ctx, "%d", getpid());
	if(!ctx) {
		s_err("%s/asprintf oom", __func__);
		exit(-1);
	}
	int res = write(fd, ctx, strlen(ctx));
	if(res < strlen(ctx)) {
		s_war("Fail to write pidfile");
	}
	ret = 0;
exit:
	if(ctx) {
		free(ctx);
	}
	if(fd >= 0 && ret) {
		close(fd);
	}
	if(path) {
		free(path);
	}
	return ret;
}
int is_process_has_locked(const char *name) {
	int ret = -1;
	char *path = NULL;
	asprintf(&path, "/var/run/%s.pid", get_last_name(name));
	if(!path) {
		s_err("%s/asprintf oom", __func__);
		goto exit;
	}
	int fd = open(path, O_WRONLY | O_CREAT, 0666);
	if (fd < 0) {
		disp_syserr(0, open);
		s_err("Fail to open %s", path);
		goto exit;
	}
	struct flock lock = {0};
	if (fcntl(fd, F_GETLK, &lock) < 0) {
		disp_syserr(0, fcntl);
		s_err("Fail to fcntl F_GETLK");
		goto exit;
	}
	ret = (lock.l_type == F_WRLCK) ? 1 : 0;
exit:
	if(path) {
		free(path);
	}
	if(fd >= 0) {
		close(fd);
	}
	return ret;
}
int WaitOthersInstsExit ( const char *name, size_t _10ms ) {
	while ( unique_process_lock ( name ) < 0 ) {
		if ( _10ms ) {
			_10ms --;
			usleep ( 1000 * 10 );
			continue;
		}
		exit ( -1 );
	}
	return 0;
}
#include <sys/prctl.h>
int set_thread_name(pthread_t ptid, const char *name) {
	if(!ptid) {
		int res = prctl(PR_SET_NAME, name);
		if(res) {
			s_err("%s/prctl", __func__);
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
#include <net/if.h>
#include <arpa/inet.h>
int get_local_ip_by_name(const char *name, char *ip_buf, size_t buf_bytes) {
	int res = 0;
	size_t i = 0;
	struct ifreq *ifr = NULL;
	if(!name) {
		s_err("Plz a vaild ifr_name by arg1!");
		return -1;
	}
	if(!(ip_buf)) {
		s_err("Plz a vaild ip_buf by arg2!");
		return -1;
	}
	if(buf_bytes < 17) {
		s_err("buf_bytes can't less then 17!");
		return -1;
	}
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0) {
		s_err(__func__);
		show_errno(0, "socket");
		return -1;
	}
	struct ifconf ifc;
	ifc.ifc_len = 1024;
	ifc.ifc_buf = (char *)calloc(1, ifc.ifc_len);
	if(!ifc.ifc_buf) {
		s_err("%s/calloc/oom", __func__);
		goto ErrHandle;
	}
	res = ioctl(fd, SIOCGIFCONF, &ifc);
	if(res < 0) {
		s_err(__func__);
		show_errno(0, "ioctl SIOCGIFCONF");
		goto ErrHandle;
	}
	ifr = (struct ifreq *)ifc.ifc_buf;
	for(; ifr->ifr_name; ifr++) {
		if(strcmp(name, ifr->ifr_name)) {
			continue;
		}
		inet_ntop(AF_INET, &((struct sockaddr_in *)&ifr->ifr_addr)->sin_addr, ip_buf, 17);
		free(ifc.ifc_buf);
		close(fd);
		return 0;
	}
ErrHandle:
	if(ifc.ifc_buf) {
		free(ifc.ifc_buf);
	}
	if(fd >= 0) {
		close(fd);
	}
	return -1;
}
#endif
#ifdef _WIN_API_
void *memmem ( const void *mp, size_t mb, const void *sp, size_t sb ) {
	char *mcb = ( char * ) mp;
	char *scb = ( char * ) sp;
	size_t i = 0, j = 0, k = 0;
	for ( ; i < mb; i++ ) {
		for ( j = 0, k = i; j < sb; j++, k++ ) {
			if ( scb[j] != mcb[k] ) {
				break;
			}
		}
		if ( j == ( sb ) ) {
			return mcb + i;
		}
	}
	return NULL;
}
#endif
