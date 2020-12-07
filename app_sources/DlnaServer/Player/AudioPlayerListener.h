/*
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef DUEROS_DCS_APP_MEDIAPLAYER_AUDIOPLAYERLISTENER_H_
#define DUEROS_DCS_APP_MEDIAPLAYER_AUDIOPLAYERLISTENER_H_

namespace duerOSDcsApp {
	namespace mediaPlayer {

		enum MediaPlayerState {
			AUDIOPLAYER_IDLE,
			MEDIAPLAYER_PLAYING,
			MEDIAPLAYER_STOPPED,
			MEDIAPLAYER_PAUSED,
			MEDIAPLAYER_FINISHED		
		};

		enum AudioPlayerFinishStatus {
			AUDIOPLAYER_FINISHSTATUS_STOP,    //主动调stop
			AUDIOPLAYER_FINISHSTATUS_END,     //音乐自然播放完成
		};

		enum PlayProgressInfo {
			BEFORE_START,
			DURING_PLAY
		};

		class AudioPlayerListener {
		public:
			virtual void onPlaybackStarted() = 0;
			virtual void onPlaybackStopped() = 0 ;				 
			virtual void onPlaybackPaused() = 0;				 
			virtual void onPlaybackFinished() = 0;				 
			virtual void onPlaybackNearlyfinished() = 0 ;				 
			virtual void onPlaybackError() = 0;				 
			virtual void onBufferUnderrun() = 0;	 
			virtual void onBufferRefilled() = 0; 
			virtual void onRecvFirstpacket() = 0;
			virtual void onPlaybackResumed() = 0;		
		};

	}  // mediaPlayer
}  // duerOSDcsApp

#endif  // DUEROS_DCS_APP_MEDIAPLAYER_AUDIOPLAYERLISTENER_H_
