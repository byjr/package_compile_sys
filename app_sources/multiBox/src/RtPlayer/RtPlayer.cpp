#include <lzUtils/base.h>
#include <alloca.h>
#include "RtPlayer.h"
class chunkData {
	char *data;
	size_t size;
public:
	chunkData(char *dat, size_t bytes) {
		data = new char[bytes];
		memcpy(data, dat, bytes);
		size = bytes;
	}
	~chunkData() {
		delete []data;
	}
	char *getData() {
		return data;
	}
	size_t getSize() {
		return size;
	}
	static void destroy(void *one);
};
void chunkData::destroy(void *one) {
	auto chunk = (chunkData *)one;
	if(!chunk) {
		return ;
	}
	delete chunk;
}
RtPlayer::RtPlayer(RtPlayerPar *par) {
	mPar = par;
	mMTQ = new MTQueue(mPar->pMTQPar);
	if(!mMTQ) {
		s_err("new MTQueue failed");
		return ;
	}
	s_inf("idev:%s;odev:%s", mPar->pRecPar->device, mPar->pPlyPar->device);
	mRec = alsa_ctrl_create(mPar->pRecPar);
	if(!mRec) {
		s_err("alsa_ctrl_create recorder failed");
		return ;
	}
	mPly = alsa_ctrl_create(mPar->pPlyPar);
	if(!mPly) {
		s_err("alsa_ctrl_create player failed");
		return ;
	}
	mPauseFlag = false;
	mFullFlag = false;
	mChunkBytes = mRec->mPar->sample_rate * mRec->bytes_per_frame / 1000 * mPar->mChunkTimeMs;
	s_inf("mChunkBytes:%u", mChunkBytes);
	mRecTrd = std::thread([this]() {
		set_thread_name(0, "RecorderThread");
		auto framesBuf = new char[mChunkBytes];
		chunkData *chunk;
		for(;;) {
			if(mPauseFlag) {
				usleep(1000 * 100);
				continue;
			}
			ssize_t rret = alsa_ctrl_read_stream(mRec, framesBuf, mChunkBytes);
			if(rret != mChunkBytes) {
				s_err("alsa_ctrl_read failed");
				continue;
			}
			chunk = new chunkData(framesBuf, rret);
			if(!chunk) {
				s_err("new chunkData failed");
				continue;
			}
			mFullFlag = mMTQ->cycWrite(chunk);
		}
		delete framesBuf;
	});
	while(!mFullFlag) {
		usleep(1000 * 10);
	}
	mPlyTrd = std::thread([this]() {
		chunkData *chunk;
		set_thread_name(0, "PlayerThread");
		for(;;) {
			if(mPauseFlag) {
				usleep(1000 * 100);
				continue;
			}
			chunk = (chunkData *)mMTQ->read();
			if(!chunk) {
				s_err("exit play thread");
				continue;
			}
			ssize_t wret = alsa_ctrl_write_stream(mPly, chunk->getData(), chunk->getSize());
			if(wret != chunk->getSize()) {
				alsa_ctrl_pause(mPly);
				while(!mFullFlag) {
					usleep(1000 * mPar->mChunkTimeMs);
				}
				alsa_ctrl_resume(mPly);
			}
			delete chunk;
		}
	});
}
RtPlayer::~RtPlayer() {
	if(mRecTrd.joinable()) {
		mRecTrd.join();
	}
	alsa_ctrl_destroy(mRec);
	if(mPlyTrd.joinable()) {
		mPlyTrd.join();
	}
	alsa_ctrl_destroy(mPly);
	delete mMTQ;
}
void RtPlayer::pause() {
	mPauseFlag = true;
	alsa_ctrl_pause(mRec);
	alsa_ctrl_pause(mPly);
	mMTQ->clear();
}
void RtPlayer::resume() {
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
RtPlayer *g_mRtPlayer = NULL;
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
		break;
	default:
		break;
	}
}
#include <getopt.h>
int RtPlayer_main(int argc, char *argv[]) {
	alsa_args_t recPar = {
		.device 	 = "bg_card",
		.sample_rate = 48000,
		.channels 	 = 2,
		.action 	 = SND_PCM_STREAM_CAPTURE,
		.flags		 = 0,
		.fmt		 = SND_PCM_FORMAT_S16_LE,
		.ptime		 = 10000,//10ms
		.btime		 = 40000,//40ms
	};
	alsa_args_t plyPar = {
		.device 	 = "bg_card",
		.sample_rate = 48000,
		.channels 	 = 2,
		.action 	 = SND_PCM_STREAM_PLAYBACK,
		.flags		 = 0,
		.fmt		 = SND_PCM_FORMAT_S16_LE,
		.ptime		 = 10000,//10ms
		.btime		 = 40000,//40ms
	};
	MTQueuePar MTQPar;
	MTQPar.mMax = 4;
	MTQPar.destroyOne = &chunkData::destroy;
	size_t chunkTimeMs = 10;
	int opt = 0;
	while ((opt = getopt_long_only(argc, argv, "nt:m:i:o:l:ph", NULL, NULL)) != -1) {
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
		case 'm':
			MTQPar.mMax = atoi(optarg);
			break;
		case 't':
			chunkTimeMs = atoi(optarg);
			break;
		case 'n':
			plyPar.flags |= SND_PCM_NONBLOCK;
			break;
		default: /* '?' */
			return help_info(argc, argv);
		}
	}
	//打印编译时间
	signal(SIGUSR1, sig_handle);
	showCompileTmie(argv[0], s_war);
	RtPlayerPar mPar = {
		.pRecPar = &recPar,
		.pPlyPar = &plyPar,
		.pMTQPar = &MTQPar,
	};
	mPar.mChunkTimeMs = chunkTimeMs;
	RtPlayer *mRtPlayer = new RtPlayer(&mPar);
	if(!mRtPlayer) {
		s_err("");
		return -1;
	}
	g_mRtPlayer = mRtPlayer;
	delete mRtPlayer;
	return -1;
}