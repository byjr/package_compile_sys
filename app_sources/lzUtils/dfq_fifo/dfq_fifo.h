#ifndef DF_QUEUE_FIFO_H_INCLUDED
#define DF_QUEUE_FIFO_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif
#include <pthread.h>
#include "../common/dll_fifo.h"
typedef struct dfq_fifo_par_t {
	size_t size;
	void *datPar;
	df_dat_create_t dat_create;
	df_dat_destroy_t dat_destroy;
	df_dat_show_t dat_show;
} dfq_fifo_par_t;

typedef struct dfq_fifo_t {
	dfq_fifo_par_t *mPar;
	dll_fifo_t *mQue;
	pthread_mutex_t *mMtx;
	pthread_cond_t *mCond;
	volatile bool mIsWaitWasExited;
} dfq_fifo_t;

dfq_fifo_t *dfq_fifo_create ( dfq_fifo_par_t *par );
bool dfq_fifo_destroy ( dfq_fifo_t *ptr );
bool dfq_fifo_full ( dfq_fifo_t *ptr );
bool dfq_fifo_empty ( dfq_fifo_t *ptr );
void *dfq_fifo_read ( dfq_fifo_t *ptr );
void *dfq_fifo_tdRead ( dfq_fifo_t *ptr, size_t msec );
void *dfq_fifo_readNb ( dfq_fifo_t *ptr );
bool dfq_fifo_write ( dfq_fifo_t *ptr, void *data );
bool dfq_fifo_tdWrite ( dfq_fifo_t *ptr, void *data, size_t msec );
bool dfq_fifo_cycWrite ( dfq_fifo_t *ptr, void *data );
bool dfq_fifo_clear ( dfq_fifo_t *ptr );
void *dfq_fifo_get ( dfq_fifo_t *ptr, size_t n );
ssize_t dfq_fifo_size ( dfq_fifo_t *ptr );
bool dfq_fifo_setExit ( dfq_fifo_t *ptr );
#ifdef __cplusplus
}
#endif
#endif // DF_QUEUE_FIFO_H_INCLUDED
