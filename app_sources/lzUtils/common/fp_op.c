#include "fp_op.h"

ssize_t get_size_by_path ( const char *path ) {
	struct stat lstat = {0};
	int ret = stat ( path, &lstat );
	if ( ret < 0 ) return -1;
	return lstat.st_size;
}
ssize_t fp_write_file ( const char *path, const char *mode, char *data, size_t bytes ) {
	FILE *fp = fopen ( path, mode );
	if ( !fp ) {
		s_err ( "%s", path );
		show_errno ( 0, "fopen" );
		goto Err;
	}
	ssize_t res = fwrite ( data, sizeof ( char ), bytes, fp );
	if ( res < bytes ) {
		show_errno ( 0, "fwrite" );
		goto Err;
	}
	fclose ( fp );
	return 0;
Err:
	fclose ( fp );
	return -1;
}
char *fp_read_file ( const char *path, const char *mode, size_t *pBytes ) {
	FILE *fp = fopen ( path, mode );
	if ( !fp ) {
		s_err ( "%s", path );
		show_errno ( 0, "fopen" );
		goto Err;
	}
	struct stat f_stat = {0};
	int ret = fstat ( fileno ( fp ), &f_stat );
	if ( ret < 0 ) {
		show_errno ( 0, "stat" );
		goto Err;
	}
	char *data = ( char * ) malloc ( f_stat.st_size + 1 );
	if ( !data ) {
		s_err ( "oom" );
		goto Err;
	}
	ssize_t res = fread ( data, sizeof ( char ), f_stat.st_size, fp );
	if ( res < f_stat.st_size ) {
		show_errno ( 0, "fread" );
		goto Err;
	}
	fclose ( fp );
	*pBytes = res;
	return data;
Err:
	fclose ( fp );
	return NULL;
}
ssize_t fp_copy_file ( const char *src_path, const char *dst_path ) {
	size_t bytes = 0;
	char *data = fp_read_file ( src_path, "rb", &bytes );
	if ( !data ) {
		s_err ( "fp_copy_file/fp_read_file" );
		return -1;
	}
	ssize_t res = fp_write_file ( dst_path, "wb", data, bytes );
	free ( data );
	if ( res < 0 ) {
		s_err ( "fp_copy_file/fp_write_file" );
		return -1;
	}
	return 0;
}
