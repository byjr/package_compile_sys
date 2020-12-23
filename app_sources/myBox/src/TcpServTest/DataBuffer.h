#ifndef _APP_SOURCES_MEDIA_MEDIAPLAYER_DATABUFFER_H_
#define _APP_SOURCES_MEDIA_MEDIAPLAYER_DATABUFFER_H_
#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <thread>
class data_unit{
	char* data_ptr;
	size_t data_size;
	size_t space_size;
public:
	~data_unit(){
		if(data_ptr){
			delete data_ptr;
		}
	}
	data_unit(size_t size,char ch){
		data_ptr = new char[size];
		if(!data_ptr){
			s_err("");
			return;
		}
		memset(data_ptr,ch,size);
		data_size = space_size = size;
	}
	data_unit(size_t size){
		data_ptr = new char[size];
		if(!data_ptr){
			s_err("");
			return;
		}
		data_size = space_size = size;
	}	
	char* data(){
		return data_ptr;
	}
	size_t size(){
		return data_size;
	}
	size_t capacity(){
		return space_size;
	}
	bool resize(size_t size){
		if(!data_ptr){
			s_err("");
			return false;
		}
		if(size > space_size){
			auto ptr = new char[size];
			if(!ptr){
				s_err("oom");
				return false;
			}
			memcpy(ptr,data_ptr,data_size);
			free(data_ptr);
			data_ptr = ptr;
		}
		data_size = size;
		return true;
	}
};
// typedef std::vector<char> data_unit;
typedef std::shared_ptr<data_unit> data_ptr;

class DataBuffer{
	std::size_t max;
	std::mutex mu;
	std::condition_variable cv;
	std::queue<data_ptr>q;
	std::atomic<bool> gotExitFlag;
public:
	DataBuffer(std::size_t max):
	gotExitFlag{false}{
		this->max = max;
	}
	void setExitFlag(){
		gotExitFlag = true;
	}
	bool push(data_ptr& one){
		std::unique_lock<std::mutex> lk(mu);
		if(q.size() > max) {
			return false;
		}
		// s_inf("one:%p,front:%p",one.get(),q.front().get());
		q.push(std::move(one));
		lk.unlock();
		cv.notify_one();
		return true;	
	}
	bool wbPush(data_ptr& one){s_inf("111");
		std::unique_lock<std::mutex> lk(mu);
		while(q.size() >= max) {
			if(gotExitFlag)	return false;
			lk.unlock();s_dbg("q.size()=%d",q.size());
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			lk.lock();
			if(gotExitFlag)	return false;
		}
		q.push(std::move(one));
		lk.unlock();
		cv.notify_one();s_inf("333");
		return true;	
	}
	bool crcPush(data_ptr& one){
		std::unique_lock<std::mutex> lk(mu);
		if(q.size() >= max) {
			q.pop();
		}
		q.push(std::move(one));
		// s_inf("one:%p,back:%p",one.get(),q.back().get());
		lk.unlock();
		cv.notify_one();
		return true;	
	}
	bool pop(data_ptr& one){
		if(q.empty()){
			s_war("wait_for gotExitFlag!");
			return false;
		}
		one = std::move(q.front());
		q.pop();
		return true;		
	}
	bool wbPop(data_ptr& one){
		std::unique_lock<std::mutex> lk(mu);
		auto unint_dur = std::chrono::milliseconds(10);
		cv.wait_for(lk,unint_dur, [&]()->bool{return (!q.empty() || gotExitFlag);});
		if(gotExitFlag){
			s_war("wait_for gotExitFlag!");
			return false;
		}
		one = std::move(q.front());
		q.pop();
		return true;
	}
	bool pop(data_ptr& one,size_t _secs){
		std::unique_lock<std::mutex> lk(mu);
		auto unint_dur = std::chrono::milliseconds(10);
		int count = std::chrono::seconds(_secs) / unint_dur;
		for(int i = 0;i < count;i++){s_trc("q.size()=%d",q.size());
			cv.wait_for(lk,unint_dur, [&]()->bool{return (!q.empty() || gotExitFlag);});
			if(gotExitFlag){
				return false;
			}	s_trc("q.size()=%d",q.size());		
			if(!q.empty()){
				one = std::move(q.front());
				q.pop();
				return true;
			}
		}
		return false;
	}
	std::size_t size(){
		std::unique_lock<std::mutex> lk(mu);
		return q.size();
	}
	void stop(){
		gotExitFlag = true;
	}
};
#endif//_APP_SOURCES_MEDIA_MEDIAPLAYER_DATABUFFER_H_
