#ifndef __RT_PLAYER_H_
#define __RT_PLAYER_H_
#include <lzUtils/alsa_ctrl/alsa_ctrl.h>
#include <thread>
#include <atomic>
#if 0
class AddrPoolPar {
public:
	size_t mMax;
	void (*destroyOne)(void *);
};
class AddrPool {
	AddrPoolPar *mPar;
	void **mVct;
	size_t mMax;
	size_t ri;
	size_t wi;
	void (*destroyOne)(void *);
public:
	size_t getMax() {
		return mMax;
	}
	AddrPool(AddrPoolPar *par) {
		mMax = par->mMax;
		destroyOne = par->destroyOne;
		ri = wi = 0;
		char **mVct = (char *)malloc(sizeof(void *));
		if(!mVct) {
			s_err("oom");
		}
	}
	void *getOne() {
		if(wi == ri) {
			return NULL;
		}
		return mVct[ri];
	}
	void getSync() {
		mVct[ri] = NULL;
		ri = (ri + 1 < mMax) ? (ri + 1) : 0;
	}
	void *readOne() {
		auto one = getOne();
		if(!one) {
			return NULL;
		}
		getSync();
		return one;
	}
	bool writeOne(void *one) {
		size_t next = (wi + 1 < mMax) ? (wi + 1) : 0;
		if(next == ri) {
			s_inf("AddrPool is full!");
			return false;
		}
		mVct[wi] = one;
		wi = next;
		return true;
	}
	bool cycWrite(void *one) {
		size_t next = (wi + 1 < mMax) ? (wi + 1) : 0;
		bool isFullFlag = false;
		if(next == ri) {
			destroyOne( mVct[i]);
			mVct[ri] = NULL;
			ri = (ri + 1 < mMax) ? (ri + 1) : 0;
			isFullFlag = true;
		}
		mVct[wi] = one;
		wi = next;
		return isFullFlag;
	}
	void clear() {
		for(int i = 0; i < mMax; i++) {
			if(mVct[i]) {
				destroyOne(mVct[i]);
				mVct[i] = NULL;
			}
		}
		wi = ri = 0;
	}
	~AddrPool() {
		clear();
		delete mVct;
	}
};
#endif
class RePlayerPar {
public:
	alsa_args_t *pRecPar;
	alsa_args_t *pPlyPar;
	size_t mChunkTimeMs;
	size_t mPoolChunks;
};
class RePlayer {
	RePlayerPar *mPar;
	alsa_ctrl_t *mRec;
	alsa_ctrl_t *mPly;
	std::thread mTrd;
	std::atomic<bool> mPauseFlag;
	std::atomic<bool> mFullFlag;
	size_t mChunkBytes;
public:
	RePlayer(RePlayerPar *par);
	~RePlayer();
	void pause();
	void resume();
};
#endif