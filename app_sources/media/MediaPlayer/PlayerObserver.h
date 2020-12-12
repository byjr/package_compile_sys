#ifndef PlayerObserver_H_
#define PlayerObserver_H_
class PlayerObserver{
	virtual bool onPlaybackStarted() 		= 0 ;
	virtual bool onPlaybackStopped() 		= 0 ;
	virtual bool onPlaybackPaused() 		= 0 ;
	virtual bool onPlaybackFinished() 		= 0 ;
	virtual bool onPlaybackError() 			= 0 ;
	virtual bool onPlaybackResumed() 		= 0 ;
	virtual bool onBufferUnderrun() 		= 0 ;
	virtual bool onBufferRefilled()			= 0 ;
	virtual bool onRecvFirstpacket() 		= 0 ;
	virtual bool onPlaybackNearlyfinished() = 0 ;
};
#endif