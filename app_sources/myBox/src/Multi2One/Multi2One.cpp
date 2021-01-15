#include "Multi2One.h"
#include <vector>
#include <fstream>
#include <mutex>
namespace cppUtils {
	void Multi2One::addObserver(ProcessInterFace *ob) {
		mObservers.push_back(ob);
	}
	void Multi2One::addInput(int i, buf_ptr &buf) {
		std::unique_lock<std::mutex> lk(*(mPar->mtxPtr));
		mPar->inBufs[i] = buf;
	}
	void Multi2One::delInput(int i) {
		std::unique_lock<std::mutex> lk(*(mPar->mtxPtr));
		mPar->inBufs.erase(i);
	}
	bool Multi2One::setExit(bool graceful) {
		isGracefulFlag = graceful;
		mGotExitFlag = true;
	}
	bool Multi2One::stop() {
		s_war("stop m2o ...");
		mGotExitFlag = true;
		for(auto i : mPar->inBufs) {
			i.second->stop();
		}
		return true;
	}
	bool Multi2One::wait() {
		return mThread.get();
	}
	bool Multi2One::isReady() {
		return mIsReadyFlag;
	}
	bool Multi2One::notifyExit() {
		for(auto i : mObservers) {
			i->onProcessExit();
		}
		return true;
	}
	Multi2One::Multi2One(std::shared_ptr<Multi2OnePar> &par):
		mPar			{	par		},
		mIsReadyFlag	{	false	},
		mGotExitFlag	{	false	},
		isGracefulFlag	{	true	} {
		mThread = std::async(std::launch::async, [this]()->bool{
			data_ptr inData, outData;
			bool res = true;
			size_t g_data_count = 0;
			for(; !mGotExitFlag;) {
				std::unique_lock<std::mutex> lk(*(mPar->mtxPtr));
				outData = std::make_shared<data_unit>(mPar->dumBytes * mPar->inBufs.size());
				size_t count =  0;
				for(auto i : mPar->inBufs) {
					if(mGotExitFlag) break;
					if(!i.second) continue;
					if(!i.second->read(inData, 10)) {
						// if(!i.second->wbRead(inData)){
						s_war("Multi2One read exit!");
						setExit(true);
						break;
					}
					memcpy(outData->data() + count, inData->data(), inData->size());
					count += inData->size();
				}
				if(mGotExitFlag) break;
				lk.unlock();
				outData->resize(count);
				res = mPar->outBufs.begin()->second->wbWrite(outData);
				g_data_count += count;
				std::this_thread::yield();
				lk.lock();
				if(!res) {
					s_war("wbWrite exit");
					setExit(true);
					break;
				}
			}
			mIsReadyFlag = false;
			return isGracefulFlag;
		});
		mIsReadyFlag = true;
	}
	Multi2One::~Multi2One() {}
}