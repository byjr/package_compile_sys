#ifndef _EXTERNAL_AUDIODUMP_SHAREMEMSYNC_H_
#define _EXTERNAL_AUDIODUMP_SHAREMEMSYNC_H_
#include <pthread.h>
#include <lzUtils/base.h>
typedef unsigned long asize_t;
class ShareMemSync {
	asize_t capacity;
	volatile asize_t wpos;
	volatile asize_t rpos;
	char head[0];
	inline asize_t _max(asize_t a, asize_t b) {
		return a < b ? b : a;
	}
	inline asize_t _min(asize_t a, asize_t b) {
		return a > b ? b : a;
	}
	inline asize_t wAble() {
		return capacity - (wpos - rpos);
	}

	inline asize_t rAble() {
		return wpos - rpos;
	}
public:
	void checkInfo() {

	}
	void init(int size) {
		capacity = size - sizeof(ShareMemSync);
		wpos = rpos = 0;
	}
	void clear() {
		wpos = 0;
		rpos = 0;
	}

	asize_t remWrite(char *buf, asize_t size) {
		asize_t l;
		size = _min(size, capacity - wpos + rpos);
		l = _min(size, capacity - (wpos & (capacity - 1)));
		memcpy(head + (wpos & (capacity - 1)), buf, l);
		memcpy(head, buf + l, size - l);
		wpos += size;
		return size;
	}
	bool write(char *buf, asize_t size, asize_t retry) {
		asize_t count = 0;
		asize_t res = 0;
		for(int i = 0; count < size && i < retry; i++) {
			res = remWrite(buf + count, size - count);
			if(res == 0) {
				usleep(5 * 1000);
				continue;
			}
			count += res;
		}
		if(count < size) {
			s_err("write failed,loss %zu bytes", size - count);
			return false;
		}
		return true;
	}
	asize_t remRead(char *buf, asize_t size) {
		asize_t l;
		size = _min(size, wpos - rpos);
		l = _min(size, capacity - (rpos & (capacity - 1)));
		memcpy(buf, head + (rpos & (capacity - 1)), l);
		memcpy(buf + l, head, size - l);
		rpos += size;
		return size;
	}
	bool read(char *buf, asize_t size, asize_t retry) {
		asize_t count = 0;
		asize_t res = 0;
		for(size_t i = 0; count < size && i < retry; i++) {
			res = remRead(buf + count, size - count);
			if(res == 0) {
				usleep(5 * 1000);
				continue;
			}
			count += res;
		}
		if(count < size) {
			return false;
		}
		return true;
	}
};
#endif//_EXTERNAL_AUDIODUMP_SHAREMEMSYNC_H_
