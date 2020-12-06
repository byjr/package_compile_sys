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
#include <sys/mman.h>
#include "PcmFilePlayerImpl.h"
#include "DCSApp/Configuration.h"
#include "DeviceTools/Utils.h"
#include <libcchip/platform.h>
namespace duerOSDcsApp {
namespace mediaPlayer {

PcmFilePlayerImpl::PcmFilePlayerImpl(): m_alsa_ctl(nullptr),
								   m_sample_rate(44100),
								   m_out_channal(1),
                                   m_playing_flag(false),
                                   m_thread_alive(true),
                                   m_thread_id(0) {
    std::string audio_dev = application::Configuration::getInstance()->getRapidPlaybackDevice();

    pthread_mutex_init(&m_mutex, nullptr);
    pthread_cond_init(&m_cond, nullptr);
    m_alsa_ctl = new AlsaController(audio_dev);
    m_alsa_ctl->init(m_sample_rate,m_out_channal);
	m_10ms_bytes = m_sample_rate * 2 / 100;
	inf("m_10ms_bytes:%d",m_10ms_bytes);

	size_t nBytes = 0;
	char *datPtr = fd_mmap_for_read("./appresources/du.pcm",&nBytes);
	if(!datPtr){
		show_errno(0,"fd_mmap_for_read");
	}

	m_dat = autom_create(datPtr,nBytes);
	if(!m_dat){
		err("autom_create failed!");
	}

	struct timespec tv={0,9*1000000};
	_trcTime = trc_time_create(CLOCK_MONOTONIC,&tv);
	if(!_trcTime){
		err("");
	}

    pthread_create(&m_thread_id,nullptr, playFunc, this);
    pthread_setname_np(m_thread_id, "PLY_btnplay");
}

PcmFilePlayerImpl::~PcmFilePlayerImpl() {
	m_thread_alive = false;
	autom_destroy(m_dat);
	 delete m_alsa_ctl;
	 m_alsa_ctl = nullptr;
	
	 pthread_mutex_destroy(&m_mutex);
	 pthread_cond_destroy(&m_cond);
}

void PcmFilePlayerImpl::play(const std::string &url,
				void (*start_callback)(void *arg),
				void *start_cb_arg,
				void (*finish_callback)()){
	stop();
    pthread_mutex_lock(&m_mutex);	
	m_url=url;
	m_start_callback=start_callback;
	m_start_cb_arg=start_cb_arg;
	m_finish_callback=finish_callback;
    m_playing_flag = true;
    pthread_cond_signal(&m_cond);
	pthread_mutex_unlock(&m_mutex);
}

void PcmFilePlayerImpl::stop() {
	m_alsa_ctl->aslaAbort();
    m_alsa_ctl->alsaClear();
    m_playing_flag = false;
}

void* PcmFilePlayerImpl::playFunc(void *arg) {
    auto instance = (PcmFilePlayerImpl*)arg;
	if(posix_thread_set_nice(-20)){
		err("set nice failed!");
	};	
    while (instance->m_thread_alive) {
        pthread_mutex_lock(&instance->m_mutex);
        while (!instance->m_playing_flag) {
            pthread_cond_wait(&instance->m_cond, &instance->m_mutex);
        }
		instance->playBuffer();
        instance->m_playing_flag = false;
        pthread_mutex_unlock(&instance->m_mutex);
    }
    return nullptr;
}

void PcmFilePlayerImpl::playBuffer() {
	if(m_start_callback){
		m_start_callback(m_start_cb_arg);
	}
	m_alsa_ctl->alsaPrepare();

	char *chunk = NULL;
	do{
		chunk = autom_read(m_dat,m_10ms_bytes);
		if(chunk){
			m_alsa_ctl->writeStream(m_out_channal,chunk, m_10ms_bytes);
			trc_time_proc_delay(_trcTime);
			trc_time_proc_sync(_trcTime);
		}else{
			trc_time_proc_delay(_trcTime);
			struct timespec tv={0,3*1000000};
			trc_time_proc_msync(_trcTime,&tv);
			char *remFill = (char *)calloc(1,m_10ms_bytes); 
			if(remFill){
				if(m_dat->cur_bytes){
					size_t remBytes = m_dat->cur_bytes;
					chunk = autom_read(m_dat,remBytes);
					if(chunk){
						memcpy(remFill,chunk,remBytes);
					}
				}
				m_alsa_ctl->writeStream(m_out_channal,remFill, m_10ms_bytes);
				free(remFill);
				trc_time_proc_delay(_trcTime);
			}
		}		
	}while(chunk);	
	autom_reset(m_dat);

	m_alsa_ctl->alsaClear();
	if(m_finish_callback){
		m_finish_callback();
	}
}

}  // mediaPlayer
}  // duerOSDcsApp
