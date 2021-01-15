#ifndef _APP_SOURCES_MEDIA_MEDIAPLAYER_DATABUFFER_H_
#define _APP_SOURCES_MEDIA_MEDIAPLAYER_DATABUFFER_H_
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
namespace cppUtils {
	class data_unit;
	typedef std::shared_ptr<data_unit> data_ptr;
	class data_unit {
		char *mData;
		size_t mSize;
		size_t mCapacity;
	public:
		~data_unit();
		data_unit(data_ptr &data);
		data_unit(const void *data, size_t size);
		data_unit(size_t size, char ch);
		data_unit(size_t size);
		char *data();
		size_t size();
		size_t capacity();
		bool resize(size_t size);
		void repFill(char ch);
	};
	class DataBuffer {
		size_t max;
		std::mutex mu;
		std::condition_variable cv;
		std::queue<data_ptr>q;
		std::atomic<bool> gotExitFlag;
	public:
		DataBuffer(size_t max);
		void setExitFlag();
		bool write(data_ptr &one);
		bool wbWrite(data_ptr &one);
		bool crcWrite(data_ptr &one);
		bool read(data_ptr &one);
		bool wbRead(data_ptr &one);
		bool read(data_ptr &one, size_t _secs);
		size_t size();
		void stop();
	};
}
#endif//_APP_SOURCES_MEDIA_MEDIAPLAYER_DATABUFFER_H_
