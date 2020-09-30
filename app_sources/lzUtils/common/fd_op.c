#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifndef _WIN_API_
#include <sys/mman.h>
#endif
#include "fd_op.h"
size_t fd_get_size(int fd) {
    struct stat stat = {0};
    int ret = un_fstat(&stat, fd);
    if(ret < 0)return -1;
    return stat.st_size;
}
void show_every_byte(char *data, size_t size) {
    int i = 0;
    s_inf("data show begin:------\\");
    for(i = 0; i < size; i++) {
        s_raw("%02d:0x%02x\n", i, data[i]);
    }
    s_inf("data show end:--------/");
}
int path_get_size(size_t *p_size, char *path) {
    struct stat lstat = {0};
    int ret = stat(path, &lstat);
    if(ret < 0)return -1;
    *p_size = lstat.st_size;
    return 0;
}
char *fd_read_file(const char *path, size_t *pBytes) {
    int fd = open(path, O_RDONLY);
    if(fd < 0) {
        show_errno(0, "open");
        return NULL;
    }
    char *data = NULL;
    struct stat fd_stat = {0};
    ssize_t res = fstat(fd, &fd_stat);
    if(res < 0) {
        show_errno(0, "fstat");
        goto Err;
    }
    data = (char *)malloc(fd_stat.st_size);
    if(!data) {
        s_err("oom");
        goto Err;
    }
    ssize_t count = fd_stat.st_size;
    int retry_times = 10;
    do {
        res = read(fd, data, count);
        if(res < 0) {
            if(errno == EAGAIN || errno == EINTR) {
                retry_times --;
                if(retry_times < 0) {
                    goto Err;
                }
                continue;
            }
            show_errno(0, "read");
            goto Err;
        }
        if(res == 0) {
            s_inf("resd done");
            break;
        }
        if(res > 0) {
            retry_times = 10;
            count -= res;
            if(count > 0) {
                data += res;
            }
        }
    } while(count > 0);
    *pBytes = fd_stat.st_size;
    return data;
Err:
    if(data) {
        free(data);
    }
    if(fd > 0) {
        close(fd);
    }
    return NULL;
}
int fd_write_file(const char *path, const char *op, char *data, size_t size) {
    ssize_t ret_size = 0;
    int fd = 0, fail = 1, ret = 0;
    int flags = O_CREAT | O_RDWR;
    if('a' == op[0])flags |= O_APPEND;
    if('w' == op[0])flags |= O_TRUNC;
    fd = un_open(path, flags, 0666);
    if(-1 == fd)return -1;
    ret_size = un_write(fd, data, size);
    if(ret_size < size)goto exit1;
    fail = 0;
exit1:
    ret = un_close(fd);
    if(-1 == ret)return -1;
    if(fail)return -1;
    return ret_size;
}
int fd_cpoy_file(const char *sPath, const char *dPath) {
    size_t bytes = 0;
    char *data =  fd_read_file(sPath, &bytes);
    if(!data) {
        s_err("fd_cpoy_file/fd_read_file");
        return -1;
    }
    ssize_t res = fd_write_file(dPath, "w", data, bytes);
    free(data);
    if(res < 0) {
        s_err("fd_cpoy_file/fd_write_file");
        return -2;
    }
    return 0;
}
#ifndef _WIN_API_
int fd_set_flag(int fd, int flag) {
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
int fd_clear_flag(int fd, int flag) {
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags < 0) {
		show_errno ( 0, "fcntl F_GETFL failed!" );
        return -1;
    }
    int ret = fcntl(fd, F_SETFL, flags &= ~(flag));
    if(ret < 0) {
        show_errno(0, "fcntl F_SETFL failed!");
        return -2;
    }
    return 0;
}

char *fd_mmap_for_read(const char *path, size_t *pBytes) {
    int fd = un_open(path, O_CREAT | O_RDONLY, 0666);
    if(!fd) {
        return NULL;
    }
    do {
        ssize_t nBytes = fd_get_size(fd);
        if(nBytes < 0) {
            s_err("fd_get_size");
            break;
        }
        char *datPtr = (char *)mmap(NULL, nBytes, PROT_READ, MAP_PRIVATE, fd, 0);
        if(!datPtr) {
            show_errno(0, "mmap");
            break;
        }
        close(fd);
        *pBytes = nBytes;
        return datPtr;
    } while(0);
    close(fd);
    return NULL;
}
#endif



