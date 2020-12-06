#ifndef __RT_PLAYER_H_
#define __RT_PLAYER_H_
#include <alsa_wrapper/alsa_ctrl.h>
#include <cppUtils/MTQueue/MTQueue.h>
#include <thread>
#include <atomic>
using namespace cppUtils;
class RtPlayerPar {
public:
	alsa_args_t *pRecPar;				//¼��������
	alsa_args_t *pPlyPar;				//����������
	MTQueuePar *pMTQPar;				//֡������в���
	size_t mChunkTimeMs;				//ÿ����Ƶchunk�ɲ��ŵ�ʱ��
};
class RtPlayer {
	RtPlayerPar *mPar;					//��������
	alsa_ctrl_t *mRec;					//¼�����
	alsa_ctrl_t *mPly;					//���ž��
	std::thread mPlyTrd;				//�������߳̾��
	std::thread mRecTrd;				//¼�����߳̾��
	MTQueue *mMTQ;						//��Ƶ֡�������
	std::atomic<bool> mPauseFlag;		//RtPlayer ��ͣ��־
	std::atomic<bool> mFullFlag;		//֡�������д����־
	size_t mChunkBytes;					//ÿ����Ƶchunk�������ֽ���
public:
	RtPlayer(RtPlayerPar *par);
	~RtPlayer();
	void pause();						//RtPlayer ��ͣ����
	void resume();						//RtPlayer �ָ�����
};
#endif