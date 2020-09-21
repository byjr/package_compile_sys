#include "MemPool.h"
#include <thread>
#include <unistd.h>
#include <lzUtils/base.h>
using namespace cppUtils;
bool MemPool::add(MemUnit *mu) { //添加一个活动单元
	std::unique_lock<std::mutex> locker(mtx);
	s_dbg("data=%u", mu->data);
	mVct.push_back(mu);
	return true;
}
bool MemPool::del(void *addr) { //删除一个单元
	std::unique_lock<std::mutex> locker(mtx);
	if(addr) { //删除指定单元
		auto it = mVct.begin();
		for(; it != mVct.end(); it++) {
			if(addr == (*it)->data) {
				delete (*it);
				mVct.erase(it);
			}
		}
		return false;
	}
	//删除一个活动单元
	auto it = mVct.begin();
	for(; it != mVct.end(); it++) {
		if((*it)->isAct) {
			mVct.erase(it);
			delete (*it);
		}
	}
	return false;
}

void *MemPool::get(size_t *pLen) { //获取一个活动单元地址
	std::unique_lock<std::mutex> locker(mtx);
	for(auto mu : mVct) {
		if(mu->isAct) {
			mu->isAct = false;
			if(pLen) {
				*pLen = mu->size;
			}
			return mu->data;
		}
	}
	return NULL;
}

bool MemPool::put(void *addr) { //归还一个单元地址
	std::unique_lock<std::mutex> locker(mtx);
	for(auto mu : mVct) {
		if(mu->data == addr) {
			mu->isAct = true;
			return true;
		}
	}
	s_err("put a err data  !!!");
	return false;
}

void MemPool::clear() { //使所有单元都变成活动态
	std::unique_lock<std::mutex> locker(mtx);
	for(auto mu : mVct) {
		if(!mu->isAct) {
			mu->isAct = true;
		}
	}
}

size_t MemPool::getSize() {
	std::unique_lock<std::mutex> locker(mtx);
	return mVct.size();
}

void *MemPool::query(int i) {
	s_inf("i=%d", i);
	if(i < 0) {
		for(auto mu : mVct) {
			s_inf("isAct=%d", mu->isAct ? 1 : 0);
			s_inf("data=%u", mu->data);
		}
		return NULL;
	}
	if(i < mVct.size()) {
		s_inf("mVct[%d].isAct=%d", i, mVct[i]->isAct ? 1 : 0);
		s_inf("mVct[%d].data=%u", i, mVct[i]->data);
		return mVct[i]->data;
	}
	s_err("i >= mVct.size");
	return NULL;
}
MemPool::~MemPool() {
	std::unique_lock<std::mutex> locker(mtx);
	for(auto mu : mVct) {
		delete mu;
	}
	s_trc(__func__);
}