#ifdef __cplusplus
extern "C" {
#endif
//--------------------autom.h start-----------------------------
#ifndef AI_MEM_H_
#define AI_MEM_H_ 1
#include <stdio.h>
#include <stdlib.h>

typedef enum autom_mode_s {
	AUTOM_MODE_MIN,
	AUTOM_MODE_ALLOC,
	AUTOM_MODE_MALLOC,
	AUTOM_MODE_MAX,
} autom_mode_t;

typedef int ( *autom_aftc_t ) ( void *ptr );
typedef int ( *autom_aftw_t ) ( void *ptr );
typedef struct autom_par_s {
	autom_mode_t mode;
	char *ex_tack;
	size_t capacity;
	autom_aftc_t aftc;
	autom_aftw_t aftw;
} autom_par_t;
typedef struct autom_t {
	autom_par_t *mPar;
	size_t content;
	char *head;
	char *tail;
} autom_t;

autom_t *autom_create ( autom_par_t *par );
char *autom_write_query ( autom_t *ptr, size_t bytes );
int autom_write_sync ( autom_t *ptr, size_t bytes );
int autom_write ( autom_t *ptr, const char *buf, size_t bytes ) ;
int autom_destroy ( autom_t *ptr );
char *autom_read_get ( autom_t *ptr, size_t *pBytes ) ;
char *autom_write_get ( autom_t *ptr, size_t *pBytes ) ;
int autom_write_syncn ( autom_t *ptr, size_t bytes );
int autom_writen ( autom_t *ptr, const char *buf, size_t bytes ) ;
char *autom_read_query ( autom_t *ptr, size_t bytes );
void autom_read_sync ( autom_t *ptr, size_t bytes );
ssize_t autom_read ( autom_t *ptr, char *buf, size_t bytes );
int autom_clear ( autom_t *ptr );
int autom_reset ( autom_t *ptr );
ssize_t autom_find_first_ch ( autom_t *ptr, size_t pos, char ch );
ssize_t autom_find_first_sub ( autom_t *ptr, size_t pos, const char *sub, size_t subb ) ;
int autom_subctt ( autom_t *ptr, size_t pos, size_t bytes ) ;
#endif
//--------------------autom.h end------------------------------
#ifdef __cplusplus
}
#endif
