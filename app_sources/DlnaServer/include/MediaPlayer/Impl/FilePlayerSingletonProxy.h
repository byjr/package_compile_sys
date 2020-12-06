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

#ifndef DUEROS_DCSAPP_INCLUDE_MEDIAPLAYER_FILEPLAYERSINGLETONPROXY_H_
#define DUEROS_DCSAPP_INCLUDE_MEDIAPLAYER_FILEPLAYERSINGLETONPROXY_H_

#include "Mp3FilePlayerInterface.h"
#include "Mp3FilePlayerImpl.h"
#include<string>

namespace duerOSDcsApp {
namespace mediaPlayer {

class FilePlayerSingletonProxy  : public Mp3FilePlayerInterface {
public:
    static FilePlayerSingletonProxy *getInstance();

    void release();

    ~FilePlayerSingletonProxy();

    void initialiaze(const std::string &alertsAudioDev, const std::string &promptAudioDev);

    void registerListener(std::shared_ptr<AudioPlayerListener> notify);

    void registerListener(std::shared_ptr<AudioPlayerListener> notify, FilePlayType type);

    void setFilePlayType(FilePlayType type);

    /**
     * set play mode of the player.
     *
     * @param mode the mode : PLAYMODE_NORMAL PLAYMODE_LOOP
     * @param val loop times, useful when the mode is PLAYMODE_LOOP; loop forever when 0.
     * @return void
     */
    void setPlayMode(PlayMode mode, unsigned int val);

    void audioPlay(const std::string& url,
                   unsigned int offset,
                   unsigned int report_interval);

    void audioPause();

    void audioResume();

    void audioStop();

    void setMute();

    void setUnmute();

    unsigned long getProgress();

    unsigned long getDuration();

    void setFadeIn(int timeSec);

private:
    FilePlayerSingletonProxy();

    FilePlayerSingletonProxy(const FilePlayerSingletonProxy &ths);

    FilePlayerSingletonProxy &operator=(const FilePlayerSingletonProxy &ths);

    static void init();

    static void destroy();

private:
    static FilePlayerSingletonProxy *s_instance;

    static pthread_once_t s_initOnce;

    static pthread_once_t s_destroyOnce;

    Mp3FilePlayerImpl *m_player;
};

}
}

#endif //DUEROS_DCSAPP_INCLUDE_MEDIAPLAYER_FILEPLAYERSINGLETONPROXY_H_
