#include "One2Multi.h"
#include <vector>
#include <fstream>
#include <mutex>
namespace cppUtils {
	void One2Multi::addObserver(ProcessInterFace *ob) {
		mObservers.push_back(ob);
	}
	void One2Multi::addOutput(int i, buf_ptr &buf) {
		std::unique_lock<std::mutex> lk(*(mPar->mtxPtr));
		mPar->inBufs[i] = buf;
	}
	void One2Multi::delOutput(int i) {
		std::unique_lock<std::mutex> lk(*(mPar->mtxPtr));
		mPar->inBufs.erase(i);
	}
	bool One2Multi::setExit(bool graceful) {
		isGracefulFlag = graceful;
		mGotExitFlag = true;
	}
	bool One2Multi::stop() {
		s_war("stop o2m ...");
		mGotExitFlag = true;
		for(auto i : mPar->inBufs) {
			i.second->stop();
		}
		return true;
	}
	bool One2Multi::wait() {
		return mThread.get();
	}
	bool One2Multi::isReady() {
		return mIsReadyFlag;
	}
	bool One2Multi::notifyExit() {
		for(auto i : mObservers) {
			i->onProcessExit();
		}
		return true;
	}
	One2Multi::One2Multi(std::shared_ptr<One2MultiPar> &par):
		mPar			{	par		},
		mIsReadyFlag	{	false	},
		mGotExitFlag	{	false	},
		isGracefulFlag	{	true	} {
		mThread = std::async(std::launch::async, [this]()->bool{
			data_ptr inData, outData;
			for(; !mGotExitFlag;) {
				std::unique_lock<std::mutex> lk(*(mPar->mtxPtr));
				if(!mPar->inBufs.begin()->second->wbRead(inData)) {
					s_war("One2Multi read exit!");
					setExit(true);
					break;
				}
				if(mGotExitFlag) break;
				lk.unlock();
				for(auto o : mPar->outBufs) {
					if(mGotExitFlag) break;
					if(!o.second) continue;
					if(!o.second->wbWrite(inData)) {
						s_war("One2Multi write exit!");
						setExit(true);
						break;
					}
				}
				lk.lock();
			}
			mIsReadyFlag = false;
			return isGracefulFlag;
		});
		mIsReadyFlag = true;
	}
	One2Multi::~One2Multi() {}
}