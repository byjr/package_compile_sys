#include <vector>
#include <thread>
#include <lzUtils/base.h>
#include "DataBuffer.h"
namespace cppUtils {
	data_unit::~data_unit() {
		if(mData) {
			delete mData;
		}
	}
	data_unit::data_unit(const void *data, size_t size) {
		mData = new char[size];
		if(!mData) {
			s_err("");
			return;
		}
		mSize = mCapacity = size;
		memcpy(mData, data, size);
	}
	data_unit::data_unit(data_ptr &data) {
		mData = new char[data->size()];
		if(!mData) {
			s_err("");
			return;
		}
		mSize = mCapacity = data->size();
		memcpy(mData, data->data(), data->size());
	}
	data_unit::data_unit(size_t size, char ch) {
		mData = new char[size];
		if(!mData) {
			s_err("");
			return;
		}
		memset(mData, ch, size);
		mSize = mCapacity = size;
	}
	data_unit::data_unit(size_t size) {
		mData = new char[size];
		if(!mData) {
			s_err("");
			return;
		}
		mSize = mCapacity = size;
	}
	char *data_unit::data() {
		return mData;
	}
	size_t data_unit::size() {
		return mSize;
	}
	size_t data_unit::capacity() {
		return mCapacity;
	}
	bool data_unit::resize(size_t size) {
		if(!mData) {
			s_err("");
			return false;
		}
		if(size > mCapacity) {
			auto ptr = new char[size];
			if(!ptr) {
				s_err("oom");
				return false;
			}
			memcpy(ptr, mData, mSize);
			free(mData);
			mData = ptr;
		}
		mSize = size;
		return true;
	}
	void data_unit::repFill(char ch) {
		size_t rem = mCapacity - mSize;
		if(rem > 0) {
			memset(mData + mSize, ch, rem);
		}
	}
	DataBuffer::DataBuffer(std::size_t max):
		gotExitFlag{false} {
		this->max = max;
	}
	void DataBuffer::setExitFlag() {
		gotExitFlag = true;
	}
	bool DataBuffer::write(data_ptr &one) {
		std::unique_lock<std::mutex> lk(mu);
		if(q.size() > max) {
			return false;
		}
		q.push(one);
		lk.unlock();
		cv.notify_one();
		return true;
	}
	bool DataBuffer::wbWrite(data_ptr &one) {
		std::unique_lock<std::mutex> lk(mu);
		while(q.size() >= max) {
			if(gotExitFlag)	return false;
			lk.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			lk.lock();
			if(gotExitFlag)	return false;
		}
		q.push(one);
		lk.unlock();
		cv.notify_one();
		return true;
	}
	bool DataBuffer::crcWrite(data_ptr &one) {
		std::unique_lock<std::mutex> lk(mu);
		if(q.size() >= max) {
			q.pop();
		}
		q.push(one);
		lk.unlock();
		cv.notify_one();
		return true;
	}
	bool DataBuffer::read(data_ptr &one) {
		if(q.empty()) {
			return false;
		}
		one = q.front();
		q.pop();
		return true;
	}
	bool DataBuffer::wbRead(data_ptr &one) {
		std::unique_lock<std::mutex> lk(mu);
		for(; !gotExitFlag;) {
			cv.wait_for(lk, std::chrono::seconds(1),
						[&]()->bool{return (!q.empty() || gotExitFlag);});
			if(!q.empty()) {
				one = q.front();
				q.pop();
				return true;
			}
		}
		return false;
	}
	bool DataBuffer::read(data_ptr &one, size_t _ms) {
		std::unique_lock<std::mutex> lk(mu);
		cv.wait_for(lk, std::chrono::milliseconds(_ms),
					[&]()->bool{return (!q.empty() || gotExitFlag);});
		if(!q.empty()) {
			one = q.front();
			q.pop();
			return true;
		}
		return false;
	}
	std::size_t DataBuffer::size() {
		std::unique_lock<std::mutex> lk(mu);
		return q.size();
	}
	void DataBuffer::stop() {
		gotExitFlag = true;
	}
}