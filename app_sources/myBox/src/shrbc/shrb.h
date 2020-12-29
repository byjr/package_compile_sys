#ifndef _EXTERNAL_AUDIODUMP_SHAREMEMSYNC_H_
#define _EXTERNAL_AUDIODUMP_SHAREMEMSYNC_H_
#include <pthread.h>
#include <lzUtils/base.h>
typedef unsigned long asize_t;
class AutoMutexLock{
	pthread_mutex_t* mMtxPtr;
public:	
	AutoMutexLock(pthread_mutex_t& pMtx){
		mMtxPtr = &pMtx;
		if(pthread_mutex_lock(mMtxPtr)){
			s_err("pthread_mutex_lock");
		}
	}
	~AutoMutexLock(){
		if(pthread_mutex_unlock(mMtxPtr)){
			s_err("pthread_mutex_unlock");
		}
	}
};
class ShareMemSync{
	volatile asize_t rpos;
	volatile asize_t wpos;
	volatile asize_t rable;
	volatile asize_t wable;
	asize_t capacity;
	pthread_mutexattr_t mtxAttr;
	pthread_mutex_t mtx;
	char head[0];
public:	
	void init(asize_t bytes){
		pthread_mutexattr_init(&mtxAttr);
		pthread_mutexattr_setpshared(&mtxAttr, PTHREAD_PROCESS_SHARED);
		pthread_mutex_init(&mtx, &mtxAttr);
		{
			wpos = rpos = rable = 0;
			wable = capacity = ((bytes - sizeof(ShareMemSync))/2);
		}
	}
	void checkInfo(){
		s_inf("sizeof(ShareMemSync)=%d",sizeof(ShareMemSync));
		s_inf("this=%p",this);
		s_inf("head=%p",head);
		s_inf("capacity=%zu",capacity);
	}
	char *write_query(asize_t size) {
		if(size > wable) {
			return NULL;
		}
		return (char *)head + wpos;
	}
	char *read_query(asize_t size) {
		if(size > rable) {
			return NULL;
		}
		return (char *)head + rpos;
	}
	void write_sync(asize_t size) {
		asize_t i = wpos;
		char *tail = head + capacity;
		if(i + size < capacity) {
			for(; i < size; i++) {
				tail[i] = head[i];
			}
			wpos += size;
		} else {
			for(; i < (capacity - wpos); i++) {
				tail[i] = head[i];
			}
			for(i = 0; i < size - (capacity - wpos); i++) {
				head[i] = tail[i];
			}
			wpos = size - (capacity - wpos);
		}
		{
			AutoMutexLock lk(mtx);
			wable -= size;
			rable += size;			
		}
	}
	void read_sync(asize_t size) {
		if(rpos + size < capacity) {
			rpos += size;
		} else {
			rpos = size - (capacity - rpos);
		}
		{
			AutoMutexLock lk(mtx);
			rable -= size;
			wable += size;
		}
	}
	bool write(char* buf,asize_t size){
		char *addr = write_query(size);
		if(!addr){
			return false;
		}
		memcpy(addr,buf,size);
		write_sync(size);
		return true;
	}
	asize_t remWrite(char* buf,asize_t size){
		asize_t bytes = wable;
		if(size <  bytes){
			bytes = size;
		}
		if(bytes == 0){
			return 0;
		}
		memcpy((char*)head+wpos,buf,bytes);
		write_sync(bytes);
		return bytes;
	}

	bool write(char* buf,asize_t size,asize_t retry){
		asize_t count = 0;
		asize_t res = 0;
		for(int i=0;count < size && i < retry;i++){
			res = remWrite(buf+count,size-count);
			if(res == 0){
				usleep(5*1000);
				continue;
			}
			count += res;
		}
		if(count < size){
			s_err("write failed,loss %zu bytes",size-count);
			return false;
		}
		return true;
	}
	asize_t remRead(char* buf,asize_t size){
		asize_t bytes = rable;
		if(size <  bytes){
			bytes = size;
		}
		if(bytes == 0){
			return 0;
		}
		memcpy(buf,(char*)head+rpos,bytes);
		read_sync(bytes);
		return bytes;
	}
	bool read(char* buf,asize_t size){
		char *addr = read_query(size);
		if(!addr){
			return false;
		}
		memcpy(buf,addr,size);
		read_sync(size);
		return true;
	}	
	bool read(char* buf,asize_t size,asize_t retry){
		asize_t count = 0;
		asize_t res = 0;
		for(size_t i=0;count < size && i < retry;i++){
			res = remRead(buf+count,size-count);
			if(res == 0){
				usleep(5*1000);
				continue;
			}
			count += res;
		}
		if(count < size){
			return false;
		}
		return true;
	}
};

#endif//_EXTERNAL_AUDIODUMP_SHAREMEMSYNC_H_
