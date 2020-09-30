#ifndef _FP_OP_H
#define _FP_OP_H 1
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "../slog/slog.h"
#include "misc.h"

ssize_t get_size_by_path ( const char *path );
ssize_t fp_write_file ( const char *path, const char *mode, char *data, size_t bytes ) ;
char *fp_read_file ( const char *path, const char *mode, size_t *pBytes ) ;
ssize_t fp_copy_file ( const char *dst_path, const char *src_path ) ;
#ifdef __cplusplus
}
#endif
#endif
