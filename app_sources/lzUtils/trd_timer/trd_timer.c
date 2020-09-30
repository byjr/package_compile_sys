#include <stdlib.h>
#include "trd_timer.h"
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int _trd_timer_destroy(trd_timer_t *ptr) {
	if(ptr) {
		px_timer_del(ptr->id);
		free(ptr);
		ptr = NULL;
	}
	return 0;
}

int trd_timer_destroy(trd_timer_t *ptr) {
	int res = -1;
	pthread_mutex_lock(&mutex);
	res = _trd_timer_destroy(ptr);
	pthread_mutex_unlock(&mutex);
	return res;
}

int _trd_timer_trigger(trd_timer_t *ptr, void *args) {
	s_trc(__func__);
	if(!ptr) {
		return -1;
	}
	struct itimerspec *itPtr = (struct itimerspec *)args;
	if(itPtr) {
		memcpy(&ptr->mPar->its, itPtr, sizeof(struct itimerspec));
	}
	return px_timer_set(ptr->id, 0, &ptr->mPar->its, NULL);
}

int trd_timer_trigger(trd_timer_t *ptr, void *args) {
	int res = -1;
	pthread_mutex_lock(ptr->mMtx);
	res = _trd_timer_trigger(ptr, args);
	pthread_mutex_unlock(ptr->mMtx);
	return res;
}

int _trd_timer_stop(trd_timer_t *ptr) {
	s_trc(__func__);
	if(!ptr) {
		return -1;
	}
	struct itimerspec it = {0};
	return px_timer_set(ptr->id, 0, &it, NULL);
}

int trd_timer_stop(trd_timer_t *ptr) {
	int res = -1;
	pthread_mutex_lock(ptr->mMtx);
	res = _trd_timer_stop(ptr);
	pthread_mutex_unlock(ptr->mMtx);
	return res;
}


trd_timer_t *trd_timer_create(trd_timer_args_t *_args) {
	trd_timer_t *ptr = calloc(1, sizeof(trd_timer_t));
	if(!ptr) {
		s_err("trd_timer_Create failure");
		return NULL;
	}
	ptr->mPar = _args;
	ptr->evp.sigev_notify = SIGEV_THREAD;
	ptr->evp.sigev_notify_attributes = _args->_attr;
	ptr->evp.sigev_notify_function = _args->handle;
	ptr->evp.sigev_value = _args->arg;
	int ret = px_timer_create(CLOCK_MONOTONIC, &ptr->evp, &ptr->id);
	if(ret < 0) return NULL;
	ptr->mMtx = &mutex;
	return ptr;
}

