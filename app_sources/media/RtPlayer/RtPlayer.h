#ifndef __RT_PLAYER_H_
#define __RT_PLAYER_H_
#include <alsa_wrapper/alsa_ctrl.h>
#include <cppUtils/MTQueue/MTQueue.h>
#include <thread>
#include <atomic>
using namespace cppUtils;
class RtPlayerPar {
public:
	alsa_args_t *pRecPar;				//录音机参数
	alsa_args_t *pPlyPar;				//播放器参数
	MTQueuePar *pMTQPar;				//帧缓冲队列参数
	size_t mChunkTimeMs;				//每个音频chunk可播放的时间
};
class RtPlayer {
	RtPlayerPar *mPar;					//启动参数
	alsa_ctrl_t *mRec;					//录音句柄
	alsa_ctrl_t *mPly;					//播放句柄
	std::thread mPlyTrd;				//播放器线程句柄
	std::thread mRecTrd;				//录音机线程句柄
	MTQueue *mMTQ;						//音频帧缓冲队列
	std::atomic<bool> mPauseFlag;		//RtPlayer 暂停标志
	std::atomic<bool> mFullFlag;		//帧缓冲队列写满标志
	size_t mChunkBytes;					//每个音频chunk包含的字节数
public:
	RtPlayer(RtPlayerPar *par);
	~RtPlayer();
	void pause();						//RtPlayer 暂停方法
	void resume();						//RtPlayer 恢复方法
};
#endif