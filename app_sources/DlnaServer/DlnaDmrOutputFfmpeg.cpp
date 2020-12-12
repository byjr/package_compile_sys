////
// Created by eddy on 18-1-30.
//
#include <assert.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Dlna/DlnaDmrLogging.h"
#include "Dlna/DlnaDmrUpnpConnMgr.h"
#include "DlnaDmrOutputFfmpeg.h"

namespace duerOSDcsApp {
namespace dueros_dlna {
using namespace duerOSDcsApp::mediaPlayer;
static output_transition_cb_t play_trans_callback_ = nullptr;
static output_update_meta_cb_t meta_update_callback_ = nullptr;
static enum MediaPlayerState m_mediaPlayerState = MEDIAPLAYER_STOPPED;

void OutputFfmpeg::set_next_uri(const std::string uri) {
    APP_INFO("OutputFfmpeg::set_next_uri");
	m_mediaPlayer->audioPlay(uri, RES_FORMAT::AUDIO_MP3, 0, 15000);
}

void OutputFfmpeg::set_uri(const std::string uri, output_update_meta_cb_t meta_cb) {
    APP_INFO("OutputFfmpeg::set_uri: %s", uri.c_str());
    //meta_update_callback_ = meta_cb;
	m_mediaPlayer->audioPlay(uri, RES_FORMAT::AUDIO_MP3, 0, 15000);
}

int OutputFfmpeg::play(output_transition_cb_t callback) {
    APP_INFO("OutputFfmpeg::play");
    play_trans_callback_ = callback;

    switch (m_mediaPlayerState) {
    case MEDIAPLAYER_STOPPED:
    case MEDIAPLAYER_FINISHED:
        m_mediaPlayer->audioResume();
        break;

    case MEDIAPLAYER_PAUSED:
        m_mediaPlayer->audioResume();
        break;

    default:
        APP_INFO("OutputFfmpeg::play m_mediaPlayerState:%d", m_mediaPlayerState);
        break;
    }

    return 0;
}

int OutputFfmpeg::stop(void) {
    APP_INFO("OutputFfmpeg::stop");
    m_mediaPlayer->audioStop();
    return 0;
}

int OutputFfmpeg::pause(void) {
    APP_INFO("OutputFfmpeg::pause");
    m_mediaPlayer->audioPause();
    return 0;
}

int OutputFfmpeg::resume(void) {
    APP_INFO("OutputFfmpeg::resume");
    m_mediaPlayer->audioResume();
    return 0;
}

int OutputFfmpeg::seek(long position_nanos) {
    APP_INFO("OutputFfmpeg::seek");
    if (m_mediaPlayerState != MEDIAPLAYER_STOPPED) {
        m_mediaPlayer->seekBy(position_nanos / 1000000);
        return 0;
    } else {
        APP_INFO("OutputFfmpeg::seek m_mediaPlayerState != MEDIAPLAYER_STOPPED return -1");
        return -1;
    }
}

int OutputFfmpeg::get_position(long* track_duration,
                               long* track_pos) {
	long pos = m_mediaPlayer->getProgress();
	long duration = m_mediaPlayer->getDuration();
	*track_pos = pos * 1000000;
	*track_duration = duration * 1000000;
	if(duration){
		s_inf("process:[%02ld%]",(pos * 100)/duration);
	}
    return 0;
}

int OutputFfmpeg::get_volume(float* v) {
    //    *v = m_mediaPlayer->getVolume();
    APP_INFO("OutputFfmpeg::get_volume %f", *v);
    return 0;
}

int OutputFfmpeg::set_volume(float value) {
    APP_INFO("OutputFfmpeg::set_volume %f", value);
    //    m_mediaPlayer->setVolume(value/100);
    return 0;
}

int OutputFfmpeg::get_mute(int* m) {

    APP_INFO("OutputFfmpeg::get_mute %d", *m);
    return 0;
}

int OutputFfmpeg::set_mute(int m) {
    APP_INFO("OutputFfmpeg::set_mute %d", m);

    return 0;
}

int OutputFfmpeg::get_state(void) {
    switch (m_mediaPlayerState) {
    case MEDIAPLAYER_STOPPED:
        APP_INFO("OutputFfmpeg::get_state: MEDIAPLAYER_STOPPED");
        break;

    case MEDIAPLAYER_PLAYING:
        APP_INFO("OutputFfmpeg::get_state: MEDIAPLAYER_PLAYING");
        break;

    case MEDIAPLAYER_PAUSED:
        APP_INFO("OutputFfmpeg::get_state: MEDIAPLAYER_PAUSED");
        break;

    case MEDIAPLAYER_FINISHED:
        APP_INFO("OutputFfmpeg::get_state: MEDIAPLAYER_FINISHED");
        break;

    default:
        APP_INFO("OutputFfmpeg::get_state: %d", m_mediaPlayerState);
        break;
    }
    return m_mediaPlayerState;
}

int OutputFfmpeg::init(void) {
    APP_INFO("OutputFfmpeg::init duer_plug");
	if(m_mediaPlayer.get()){
		return 0;
	}
    m_mediaPlayer = std::make_shared<AudioPlayerImpl>("default");
	if(!m_mediaPlayer.get()){
		s_err("new AudioPlayerImpl failed!");
		return -1;
	}
    m_mediaPlayer->registerListener(this);
    return 0;
}

void OutputFfmpeg::onPlaybackStarted() {
    APP_INFO("OutputFfmpeg::onPlaybackStarted");
    m_mediaPlayerState = MEDIAPLAYER_PLAYING;

    if (play_trans_callback_) {
        play_trans_callback_(PLAY_PLAYING);
    }
}

void OutputFfmpeg::onPlaybackStopped() {
    APP_INFO("OutputFfmpeg::onPlaybackStopped");
    m_mediaPlayerState = MEDIAPLAYER_STOPPED;

    if (play_trans_callback_) {
        play_trans_callback_(PLAY_STOPPED);
    }
}

void OutputFfmpeg::onPlaybackPaused() {
    APP_INFO("OutputFfmpeg::onPlaybackPaused");
    m_mediaPlayerState = MEDIAPLAYER_PAUSED;

    if (play_trans_callback_) {
        play_trans_callback_(PLAY_PAUSED_PLAYBACK);
    }
}

void OutputFfmpeg::onPlaybackFinished() {
    APP_INFO("OutputFfmpeg::onPlaybackFinished");
    m_mediaPlayerState = MEDIAPLAYER_FINISHED;
    /*
        if (play_trans_callback_) {
            play_trans_callback_(PLAY_STOPPED);
        }
    */
}

void OutputFfmpeg::onPlaybackError() {
	 APP_INFO("OutputFfmpeg::onPlaybackError");
    if (play_trans_callback_) {
        play_trans_callback_(PLAY_NO_MEDIA_PRESENT);
    }
}


void OutputFfmpeg::onPlaybackResumed() {
    APP_INFO("OutputFfmpeg::onPlaybackResumed");
    m_mediaPlayerState = MEDIAPLAYER_PLAYING;

    if (play_trans_callback_) {
        play_trans_callback_(PLAY_PLAYING);
    }

}

void OutputFfmpeg::onBufferUnderrun() {
	APP_INFO("OutputFfmpeg::onBufferUnderrun");
}

void OutputFfmpeg::onBufferRefilled() {
	APP_INFO("OutputFfmpeg::onBufferRefilled");	
}

void OutputFfmpeg::onRecvFirstpacket() {
	APP_INFO("OutputFfmpeg::onRecvFirstpacket");	
}
void OutputFfmpeg::onPlaybackNearlyfinished() {
	APP_INFO("OutputFfmpeg::onPlaybackNearlyFinished");	
}
}
}




















