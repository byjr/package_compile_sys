#include <vector>
#include <thread>
#include "TheCommon.h"
#include "DataBuffer.h"
data_unit::~data_unit(){
	if(data_ptr){
		delete data_ptr;
	}
}

data_unit::data_unit(const void* data,size_t size){
	data_ptr = new char[size];
	if(!data_ptr){
		s_err("");
		return;
	}
	data_size = space_size = size;
	memcpy(data_ptr,data,size);
}
data_unit::data_unit(data_unit* old){
	data_size = space_size = old->size();
	data_ptr = new char[data_size];
	if(!data_ptr){
		s_err("");
		return;
	}
	memcpy(data_ptr,old->data(),data_size);	
}
data_unit::data_unit(size_t size,char ch){
	data_ptr = new char[size];
	if(!data_ptr){
		s_err("");
		return;
	}
	memset(data_ptr,ch,size);
	data_size = space_size = size;
}
data_unit::data_unit(size_t size){
	data_ptr = new char[size];
	if(!data_ptr){
		s_err("");
		return;
	}
	data_size = space_size = size;
}	
char* data_unit::data(){
	return data_ptr;
}
size_t data_unit::size(){
	return data_size;
}
size_t data_unit::capacity(){
	return space_size;
}
bool data_unit::resize(size_t size){
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

DataBuffer::DataBuffer(std::size_t max):
gotExitFlag{false}{
	this->max = max;
	isBusyFlag = false;
}
void DataBuffer::setExitFlag(){
	gotExitFlag = true;
}
bool DataBuffer::push(data_ptr& one){
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
bool DataBuffer::wbPush(data_ptr& one){
	std::unique_lock<std::mutex> lk(mu);
	while(q.size() >= max) {
		if(gotExitFlag)	return false;
		lk.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		lk.lock();
		if(gotExitFlag)	return false;
	}
	q.push(std::move(one));
	lk.unlock();
	cv.notify_one();
	return true;	
}
bool DataBuffer::crcPush(data_ptr& one){
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
bool DataBuffer::pop(data_ptr& one){
	if(q.empty()){
		s_war("wait_for gotExitFlag!");
		return false;
	}
	one = std::move(q.front());
	q.pop();
	return true;		
}
bool DataBuffer::wbPop(data_ptr& one){
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
bool DataBuffer::pop(data_ptr& one,size_t _secs){
	std::unique_lock<std::mutex> lk(mu);
	auto unint_dur = std::chrono::milliseconds(10);
	int count = std::chrono::seconds(_secs) / unint_dur;
	for(int i = 0;i < count;i++){
		cv.wait_for(lk,unint_dur, [&]()->bool{return (!q.empty() || gotExitFlag);});
		if(gotExitFlag){
			return false;
		}
		if(!q.empty()){
			one = std::move(q.front());
			q.pop();
			return true;
		}
	}
	return false;
}
std::size_t DataBuffer::size(){
	std::unique_lock<std::mutex> lk(mu);
	return q.size();
}
void DataBuffer::stop(){
	gotExitFlag = true;
}
