#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <lzUtils/base.h>
#include <alsa_wrapper/alsa_ctrl.h>
#include <alsa_wrapper/mixer_ctrl.h>
#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <thread>
#include "PlayerObserver.h"

typedef std::vector<char> data_unit;
typedef std::shared_ptr<data_unit> data_ptr;

struct MediaPlayerPar{
	alsa_args_t playerPar;
	mixer_ctrl_par_t mixerPar;
	MediaPlayerPar(){
		memset(this,0,sizeof(MediaPlayerPar));
		
		playerPar.device 	  = "default"        	    ;
		playerPar.sample_rate = 44100                   ;
		playerPar.channels 	  = 2                       ;
		playerPar.action 	  = SND_PCM_STREAM_PLAYBACK ;
		playerPar.flags		  = 0                       ;
		playerPar.fmt		  = SND_PCM_FORMAT_S16_LE   ;
		playerPar.ptime		  = 40000             		;
		playerPar.btime		  = 2000000             	;
		
		mixerPar.device = "default" ;
		mixerPar.name 	= "Master"  ;
		mixerPar.idx 	= 0         ;
		mixerPar.volMin = 0         ;
		mixerPar.volMax = 100       ;
	}
};
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
		while(q.size() > max) {
			if(cv.wait_for(lk, std::chrono::milliseconds(10),
				 [&]()->bool{return gotExitFlag;})) {
				break;
			}
			return false;
		}
		q.push(std::move(one));
		lk.unlock();
		cv.notify_one();
		return true;	
	}
	bool pop(data_ptr& one){
		std::unique_lock<std::mutex> locker(mu);
		if(q.empty()) {
			return false;
		}
		one = std::move(q.back());
		q.pop();
		return true;		
	}
	std::size_t size(){
		std::unique_lock<std::mutex> locker(mu);
		return q.size();
	}
	void stop(){
		gotExitFlag = true;
	}
};
class MediaPlayer{
	std::shared_ptr<MediaPlayerPar> mPar;
	std::vector<std::shared_ptr<PlayerObserver>> mObs;
	std::shared_ptr<mixer_ctrl_t> mMixer;
	std::shared_ptr<alsa_ctrl_t> mPlayer;
	bool mIsInitDone;
	std::shared_ptr<DataBuffer> mBuffer;
	data_ptr mChunk;
	std::size_t mChunkSize;
	std::thread mPlayTrd;
	std::thread mDecTrd;
	std::atomic<bool>gotExitFlag,hadExitedFlag;
public:
	MediaPlayer(std::shared_ptr<MediaPlayerPar>& par):
		mIsInitDone{false},
		gotExitFlag{false},
		hadExitedFlag{false}{
		mPar = std::move(par);
		mMixer = std::shared_ptr<mixer_ctrl_t>(
			mixer_ctrl_create(&mPar->mixerPar),//构造器
			[](mixer_ctrl_t* ptr){mixer_ctrl_destroy(ptr);});//删除器
		if(!mMixer.get()){
			s_err("mMixer craete failed!");
			return;
		}		
		mPlayer = std::shared_ptr<alsa_ctrl_t>(
			alsa_ctrl_create(&mPar->playerPar),
			[](alsa_ctrl_t* ptr){alsa_ctrl_destroy(ptr);});
		if(!mPlayer.get()){
			s_err("mPlayer craete failed!");
			return;
		}
		
		mBuffer = std::make_shared<DataBuffer>(20);
		mChunkSize = mPar->playerPar.sample_rate * 16 / 8 * mPar->playerPar.btime / 1000;
		mChunk	= std::make_shared<data_unit>(mChunkSize,0);
		
		mPlayTrd = std::thread([this]{
			data_ptr data;
			std::size_t wret =0;
			for(;!gotExitFlag;){				
				if(!mBuffer->pop(data)){
					alsa_ctrl_write_stream(mPlayer.get(),mChunk->data(),mChunk->size());
					continue;
				}
				alsa_ctrl_write_stream(mPlayer.get(),data->data(),data->size());
				data.reset();
			}		
		});
				
		mDecTrd = std::thread([this]{
			data_ptr data;
			std::size_t data_size = 0;
			for(;!gotExitFlag;){
				data = std::make_shared<data_unit>(mChunkSize,0);
				std::this_thread::sleep_for(std::chrono::milliseconds(mPar->playerPar.btime));
				mBuffer->push(data);
			}
		});		
		mIsInitDone = true;
	}
	~MediaPlayer(){
		if(mDecTrd.joinable()){
			mDecTrd.join();
		}
		if(mPlayTrd.joinable()){
			mPlayTrd.join();
		}		
	}	
	bool isInitDone(){
		return mIsInitDone;
	}
	void addObserver(std::weak_ptr<PlayerObserver> ob){
		mObs.push_back(std::move(ob.lock()));
	}
	
};