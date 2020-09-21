#include <lzUtils/base.h>
#include "RePlayer.h"
class chunkPool {
	char *mVct;
	size_t mChunkBytes;
	size_t mMax;
	size_t ri;
	size_t wi;
public:
	chunkPool(size_t chunks, size_t chunkBytes) {
		ri = wi = 0;
		mMax = chunks;
		mChunkBytes = chunkBytes;
		mVct = (char *)calloc(mMax, mChunkBytes);
		if(!mVct) {
			s_err("oom");
			return ;
		}
		s_inf("mMax=%d", mMax);
	}
	char *read() {
		if(wi == ri) {
			return NULL;
		}
		char *chunk = &mVct[ri * mChunkBytes];
		ri = (ri + 1 < mMax) ? (ri + 1) : 0;
		return chunk;
	}
	bool write(char *chunk) {
		s_inf("ri=%d,wi=%d", ri, wi);
		size_t next = (wi + 1 < mMax) ? (wi + 1) : 0;
		if(next == ri) {
			s_inf("AddrPool is full!");
			return true;
		}
		memcpy(&mVct[wi * mChunkBytes], chunk, mChunkBytes);
		wi = next;
		return false;
	}
	size_t getMax() {
		return mMax;
	}
	size_t getChunks() {
		return mMax;
	}
	size_t getChunkSize() {
		return mChunkBytes;
	}
	bool empty() {
		return (wi == ri);
	}
};
#define USER_BUFFER 1
RePlayer::RePlayer(RePlayerPar *par) {
	mPar = par;
	s_inf("idev:%s;odev:%s", mPar->pRecPar->device, mPar->pPlyPar->device);
	mPly = alsa_ctrl_create(mPar->pPlyPar);
	if(!mPly) {
		s_err("alsa_ctrl_create player failed");
		return ;
	}
	mRec = alsa_ctrl_create(mPar->pRecPar);
	if(!mRec) {
		s_err("alsa_ctrl_create recorder failed");
		return ;
	}
	mPauseFlag = false;
	mChunkBytes = mRec->mPar->sample_rate * mRec->bytes_per_frame / 1000 * mPar->mChunkTimeMs;
	s_inf("mChunkBytes:%u", mChunkBytes);
	mTrd = std::thread([this]() {
		auto framesBuf = new char[mChunkBytes];
		auto pool = new chunkPool(mPar->mPoolChunks, mChunkBytes);
		bool playFlag = false;
		bool fullFlag = false;
		char *chunk = NULL;
		for(;;) {
			if(mPauseFlag) {
				usleep(1000 * mPar->mChunkTimeMs);
				continue;
			}
			ssize_t rret = alsa_ctrl_read_stream(mRec, framesBuf, mChunkBytes);
			if(rret != mChunkBytes) {
				s_err("alsa_ctrl_read failed,reset ...");
				// alsa_ctrl_reset(mRec,mPar->pRecPar);
				usleep(1000 * mPar->mChunkTimeMs);
				continue;
			}
#if USER_BUFFER
			fullFlag = pool->write(framesBuf);
			s_inf("fullFlag:%d,playFlag:%d", fullFlag ? 1 : 0, playFlag ? 1 : 0);
			if(pool->empty() && playFlag) {
				alsa_ctrl_pause(mPly);
				playFlag = false;
				s_inf("alsa_ctrl_pause");
				continue;
			} else if(fullFlag && !playFlag) {
				alsa_ctrl_resume(mPly);
				playFlag = true;
				s_inf("alsa_ctrl_resume");
			} else if(!fullFlag && !playFlag) {
				s_inf("alsa_ctrl fill poll");
				continue;
			}
			chunk = pool->read();
			if(!chunk) {
				s_err(__func__);
				exit(-1);
			}
#else
			chunk = framesBuf;
#endif
			ssize_t wret = alsa_ctrl_write_stream(mPly, chunk, mChunkBytes);
			if(wret != rret) {
				s_err("alsa_ctrl_read failed,pause ...");
#if USER_BUFFER
				alsa_ctrl_pause(mPly);
				playFlag = false;
#endif
				continue;
			}
			s_inf("play ...");
		}
		delete framesBuf;
		delete pool;
	});
}
RePlayer::~RePlayer() {
	if(mTrd.joinable()) {
		mTrd.join();
	}
	alsa_ctrl_destroy(mRec);
	alsa_ctrl_destroy(mPly);
}
void RePlayer::pause() {
	mPauseFlag = true;
	alsa_ctrl_pause(mRec);
	usleep(40 * 1000);
	alsa_ctrl_pause(mPly);
}
void RePlayer::resume() {
	mPauseFlag = false;
	alsa_ctrl_resume(mRec);
	usleep(40 * 1000);
	alsa_ctrl_resume(mPly);
}
static int help_info(int argc, char *argv[]) {
	printf("%s help:\n", get_last_name(argv[0]));
	printf("\t-t [rFifo:wFifo]:use fifo to test logic.\n");
	printf("\t-m :Use stdin to simulate the Mastr.\n");
	printf("\t-b [baudRate]:Plz use 9600/38400/115200/1500000.\n");
	printf("\t-l [logLvCtrl]\n");
	printf("\t-p [logPath]\n");
	printf("\t-h show help\n");
	return 0;
}
static RePlayer *g_mRtPlayer = NULL;
static void sig_handle(int sig) {
	static bool isPaused = false;
	switch(sig) {
	case SIGUSR1:
		if(isPaused) {
			isPaused = false;
			g_mRtPlayer->resume();
		} else {
			isPaused = true;
			g_mRtPlayer->pause();
		}
		s_war("isPaused:%d", isPaused ? 1 : 0);
		break;
	default:
		break;
	}
}
#include <getopt.h>
int RePlayer_main(int argc, char *argv[]) {
	alsa_args_t recPar = {
		.device 	 = "plughw:0,1,0",
		.sample_rate = 48000,
		.channels 	 = 2,
		.action 	 = SND_PCM_STREAM_CAPTURE,
		.flags		 = 0,
		.fmt		 = SND_PCM_FORMAT_S16_LE,
	};
	alsa_args_t plyPar = {
		.device 	 = "plughw:1,1",
		.sample_rate = 48000,
		.channels 	 = 2,
		.action 	 = SND_PCM_STREAM_PLAYBACK,
		.flags		 = 0,
		.fmt		 = SND_PCM_FORMAT_S16_LE,
	};
	size_t chunkTimeMs = 10;
	size_t poolChunks = 5;
	int opt = 0;
	while ((opt = getopt_long_only(argc, argv, "t:m:i:o:l:ph", NULL, NULL)) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		case 'i':
			recPar.device = optarg;
			break;
		case 'o':
			plyPar.device = optarg;
			break;
		case 't':
			chunkTimeMs = atoi(optarg);
			break;
		case 'm':
			poolChunks = atoi(optarg);
		default: /* '?' */
			return help_info(argc, argv);
		}
	}
	//打印编译时间
	signal(SIGUSR1, sig_handle);
	showCompileTmie(argv[0], s_war);
	RePlayerPar mPar = {
		.pRecPar = &recPar,
		.pPlyPar = &plyPar,
	};
	mPar.mChunkTimeMs = chunkTimeMs;
	mPar.mPoolChunks = poolChunks;
	RePlayer *mRtPlayer = new RePlayer(&mPar);
	if(!mRtPlayer) {
		s_err("");
		return -1;
	}
	g_mRtPlayer = mRtPlayer;
	delete mRtPlayer;
	return -1;
}