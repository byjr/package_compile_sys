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

#ifndef DUEROS_DCSAPP_INCLUDE_MEDIAPLAYER_PCMFILEPLAYERIMPL_H_
#define DUEROS_DCSAPP_INCLUDE_MEDIAPLAYER_PCMFILEPLAYERIMPL_H_

#include "AlsaController.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <libcchip/platform.h>
namespace duerOSDcsApp {
namespace mediaPlayer {

class PcmFilePlayerImpl {
public:
    PcmFilePlayerImpl();

    ~PcmFilePlayerImpl();

	int m_sample_rate;
	int m_out_channal;
	void (*m_start_callback)(void *arg);
	void *m_start_cb_arg;
	void (*m_finish_callback)();

	std::string m_url;
    void play(const std::string &url,
					void (*start_callback)(void *arg),
                	void *start_cb_arg,
                 	void (*finish_callback)());

    void stop();

private:
    void playBuffer();

    static void* playFunc(void *arg);

private:
    AlsaController *m_alsa_ctl;
    volatile bool m_playing_flag;
    bool m_thread_alive;
    pthread_t m_thread_id;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
	autom_t* m_dat;
	size_t m_10ms_bytes;
	trc_time_t *_trcTime;
};

}  // namespace mediaPlayer
}  // namespace duerOSDcsApp

#endif  // DUEROS_DCSAPP_INCLUDE_MEDIAPLAYER_PCMFILEPLAYERIMPL_H_
