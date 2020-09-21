#include "dfq_fifo.h"
#include <stdlib.h>
#include <string.h>
#include "../slog/slog.h"
static void *dll_data_create ( void *par ) {
	dfq_fifo_par_t *dfq_par = ( dfq_fifo_par_t * ) par;
	if ( dfq_par->dat_create ) {
		return dfq_par->dat_create ( dfq_par->datPar );
	}
	return NULL;
}
static bool dll_data_destroy ( void *par, void *data ) {
	dfq_fifo_par_t *dfq_par = ( dfq_fifo_par_t * ) par;
	if ( !dfq_par->dat_destroy ) {
		s_err ( "dat_create is NULL" );
		return NULL;
	}
	return dfq_par->dat_destroy ( dfq_par->datPar, data );
}
static bool dll_data_show ( void *par, void *data ) {
	dfq_fifo_par_t *dfq_par = ( dfq_fifo_par_t * ) par;
	if ( !dfq_par->dat_show ) {
		s_err ( "dat_create is NULL" );
		return NULL;
	}
	return dfq_par->dat_show ( dfq_par->datPar, data );
}
static dll_fifo_par_t g_dll_fifo_par;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

dfq_fifo_t *dfq_fifo_create ( dfq_fifo_par_t *par ) {
	dfq_fifo_t *ptr = ( dfq_fifo_t * ) calloc ( 1, sizeof ( dfq_fifo_t ) );
	if ( !ptr ) {
		s_err ( "(dfq_fifo_t*)calloc failed!!" );
		return NULL;
	}
	ptr->mCond = &cond;
	ptr->mMtx = &mutex;
	ptr->mPar = par;
	dll_fifo_par_t *dll_fifo_par = &g_dll_fifo_par;
	memset ( dll_fifo_par, 0, sizeof ( dll_fifo_par_t ) );
	dll_fifo_par->dat_par = par;
	dll_fifo_par->dat_create = &dll_data_create;
	dll_fifo_par->dat_destroy = &dll_data_destroy;
	dll_fifo_par->dat_show = &dll_data_show;
	ptr->mQue = dll_fifo_create ( dll_fifo_par );
	if ( !ptr->mQue ) {
		s_err ( "dll_fifo_create failed!!" );
		dfq_fifo_destroy ( ptr );
		return NULL;
	}
	ptr->mIsWaitWasExited = false;
	return ptr;
}
bool _dfq_fifo_destroy ( dfq_fifo_t *ptr ) {
	if ( !ptr ) return false;
	if ( ptr->mQue ) {
		if ( !dll_fifo_destroy ( ptr->mQue ) ) {
			return false;
		}
		return true;
	}
	return false;
}
bool dfq_fifo_destroy ( dfq_fifo_t *ptr ) {
	bool res = false;
	pthread_mutex_lock ( ptr->mMtx );
	res = _dfq_fifo_destroy ( ptr );
	pthread_mutex_unlock ( ptr->mMtx );
	return res;
}

bool dfq_fifo_full ( dfq_fifo_t *ptr ) {
	if ( !ptr ) return false;
	bool res = false;
	pthread_mutex_lock ( ptr->mMtx );
	res = ( ptr->mQue->size >= ptr->mPar->size );
	pthread_mutex_unlock ( ptr->mMtx );
	return res;
}

bool dfq_fifo_empty ( dfq_fifo_t *ptr ) {
	if ( !ptr ) return false;
	bool res = false;
	pthread_mutex_lock ( ptr->mMtx );
	res = ( ptr->mQue->size <= 0 );
	pthread_mutex_unlock ( ptr->mMtx );
	return res;
}

void *dfq_fifo_read ( dfq_fifo_t *ptr ) {
	if ( !ptr ) return false;
	pthread_mutex_lock ( ptr->mMtx );
	while ( ptr->mQue->size <= 0 ) {
		pthread_cond_wait ( ptr->mCond, ptr->mMtx );
	}
	void *data = dll_fifo_pop_front ( ptr->mQue );
	pthread_mutex_unlock ( ptr->mMtx );
	return data;
}
void *dfq_fifo_tdRead ( dfq_fifo_t *ptr, size_t msec ) {
	if ( !ptr ) return false;
	pthread_mutex_lock ( ptr->mMtx );
	struct timespec tv = {0};
	tv.tv_nsec = msec * 1000000;
	while ( ptr->mQue->size <= 0 ) {
		int res = pthread_cond_timedwait ( ptr->mCond, ptr->mMtx, &tv );
		if ( res < 0 ) {
			s_err ( "pthread_cond_timedwait res:%d", res );
		}
		if ( !ptr->mIsWaitWasExited ) {
			continue;
		}
		pthread_mutex_unlock ( ptr->mMtx );
		return NULL;
	}
	void *data = dll_fifo_pop_front ( ptr->mQue );
	pthread_mutex_unlock ( ptr->mMtx );
	return data;
}
bool dfq_fifo_write ( dfq_fifo_t *ptr, void *data ) {
	if ( !ptr ) return false;
	pthread_mutex_lock ( ptr->mMtx );
	if ( ptr->mQue->size >= ptr->mPar->size ) {
		pthread_cond_wait ( ptr->mCond, ptr->mMtx );
	}
	bool res = dll_fifo_push_back ( ptr->mQue, data );
	pthread_mutex_unlock ( ptr->mMtx );
	if ( !res ) {
		s_err ( "dll_fifo_push_front failed!!" );
		return false;
	}
	pthread_cond_signal ( ptr->mCond );
	return true;
}
bool dfq_fifo_tdWrite ( dfq_fifo_t *ptr, void *data, size_t msec ) {
	if ( !ptr ) return false;
	struct timespec tv = {0};
	tv.tv_nsec = msec * 1000000;
	pthread_mutex_lock ( ptr->mMtx );
	if ( ptr->mQue->size >= ptr->mPar->size ) {
		pthread_cond_timedwait ( ptr->mCond, ptr->mMtx, &tv );
		if(ptr->mQue->size >= ptr->mPar->size) {
			pthread_mutex_unlock ( ptr->mMtx );
			return false;
		}
	}
	bool res = dll_fifo_push_back ( ptr->mQue, data );
	pthread_mutex_unlock ( ptr->mMtx );
	if ( !res ) {
		s_err ( "dll_fifo_push_front failed!!" );
		return false;
	}
	pthread_cond_signal ( ptr->mCond );
	return true;
}
bool dfq_fifo_cycWrite ( dfq_fifo_t *ptr, void *data ) {
	if ( !ptr ) return false;
	pthread_mutex_lock ( ptr->mMtx );
	if ( ptr->mQue->size >= ptr->mPar->size ) {
		dll_fifo_erase ( ptr->mQue, data );
	}
	bool res = dll_fifo_push_back ( ptr->mQue, data );
	pthread_mutex_unlock ( ptr->mMtx );
	if ( !res ) {
		s_err ( "dll_fifo_push_front failed!!" );
		return false;
	}
	pthread_cond_signal ( ptr->mCond );
	return true;
}
bool dfq_fifo_clear ( dfq_fifo_t *ptr ) {
	if ( !ptr ) return false;
	bool res = false;
	pthread_mutex_lock ( ptr->mMtx );
	res = dll_fifo_clear ( ptr->mQue );
	pthread_mutex_unlock ( ptr->mMtx );
	return res;
}
void *dfq_fifo_get ( dfq_fifo_t *ptr, size_t n ) {
	if ( !ptr ) return NULL;
	void *data = NULL;
	pthread_mutex_lock ( ptr->mMtx );
	data = dll_fifo_get ( ptr->mQue, n );
	pthread_mutex_unlock ( ptr->mMtx );
	return data;
}
void *dfq_fifo_query ( dfq_fifo_t *ptr, ssize_t n ) {
	if ( !ptr ) return NULL;
	void *data = NULL;
	pthread_mutex_lock ( ptr->mMtx );
	data = dll_fifo_query ( ptr->mQue, n );
	pthread_mutex_unlock ( ptr->mMtx );
	return data;
}
ssize_t dfq_fifo_size ( dfq_fifo_t *ptr ) {
	if ( !ptr ) return -1;
	ssize_t size = 0;
	pthread_mutex_lock ( ptr->mMtx );
	size =  ptr->mQue->size;
	pthread_mutex_unlock ( ptr->mMtx );
	return size;
}
bool dfq_fifo_setExit ( dfq_fifo_t *ptr ) {
	if ( !ptr ) return -1;
	ptr->mIsWaitWasExited = true;
	pthread_cond_broadcast ( ptr->mCond );
	return true;
}
