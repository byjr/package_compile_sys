#ifndef _APP_SOURCES_MEDIA_MEDIAPLAYER_DATABUFFER_H_
#define _APP_SOURCES_MEDIA_MEDIAPLAYER_DATABUFFER_H_
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
class data_unit {
	char *data_ptr;
	size_t data_size;
	size_t space_size;
public:
	~data_unit();
	data_unit(const void *data, size_t size);
	data_unit(size_t size, char ch);
	data_unit(size_t size);
	data_unit(data_unit *ptr);
	char *data();
	size_t size();
	size_t capacity();
	bool resize(size_t size);
};
typedef std::unique_ptr<data_unit> data_ptr;
class DataBuffer {
	std::size_t max;
	std::mutex mu;
	std::condition_variable cv;
	std::queue<data_ptr>q;
	std::atomic<bool> gotExitFlag;
	std::atomic<bool> isBusyFlag;
public:
	DataBuffer(std::size_t max);
	void setExitFlag();
	bool push(data_ptr &one);
	bool wbPush(data_ptr &one);
	bool crcPush(data_ptr &one);
	bool pop(data_ptr &one);
	bool wbPop(data_ptr &one);
	bool pop(data_ptr &one, size_t _secs);
	std::size_t size();
	void stop();
	bool isBusying() {
		return isBusyFlag;
	}
	void setBusyFlag(bool busy) {
		isBusyFlag = busy;
	}
};
#endif//_APP_SOURCES_MEDIA_MEDIAPLAYER_DATABUFFER_H_
