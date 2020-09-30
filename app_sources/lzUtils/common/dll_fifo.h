#ifndef _DLL_FIFO_H_
#define _DLL_FIFO_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdbool.h>

typedef void * ( *df_dat_create_t ) ( void *par );
typedef bool ( *df_dat_destroy_t ) ( void *par, void *data );
typedef bool ( *df_dat_show_t ) ( void *par, void *data );

typedef struct dll_fifo_par_t {
	size_t size;
	void *dat_par;
	df_dat_create_t dat_create;
	df_dat_destroy_t dat_destroy;
	df_dat_show_t dat_show;
} dll_fifo_par_t;

typedef struct dll_node_t {
	struct dll_node_t *prev;
	struct dll_node_t *next;
	void *data;
} dll_node_t;

typedef struct dll_fifo_t {
	dll_fifo_par_t *mPar;
	dll_node_t *front;
	dll_node_t *back;
	volatile size_t size;
} dll_fifo_t;

dll_fifo_t *dll_fifo_create ( dll_fifo_par_t *par );
bool dll_fifo_destroy ( dll_fifo_t *ptr );
bool dll_fifo_push_back ( dll_fifo_t *ptr, void *data );
bool dll_fifo_push_front ( dll_fifo_t *ptr, void *data );
void *dll_fifo_pop_front ( dll_fifo_t *ptr );
void *dll_fifo_pop_back ( dll_fifo_t *ptr );
bool dll_fifo_insert ( dll_fifo_t *ptr, void *data, size_t n );
bool dll_fifo_remove ( dll_fifo_t *ptr, size_t n );
bool dll_fifo_erase ( dll_fifo_t *ptr, void *data );
ssize_t dll_fifo_size ( dll_fifo_t *ptr );
void *dll_fifo_query ( dll_fifo_t *ptr, ssize_t n );
void *dll_fifo_rquery ( dll_fifo_t *ptr, ssize_t n );
void *dll_fifo_get ( dll_fifo_t *ptr, size_t n );
bool dll_fifo_clear ( dll_fifo_t *ptr );
#ifdef __cplusplus
}
#endif
#endif




