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

#include <DcsSdk/LocalMediaPlayerInterface.h>
#include "DCSApp/DeviceIoWrapper.h"
#include "BluetoothPlayerImpl.h"
#include "LoggerUtils/DcsSdkLogger.h"

namespace duerOSDcsApp {
namespace mediaPlayer {

std::shared_ptr<BluetoothPlayerImpl> BluetoothPlayerImpl::create() {

    std::shared_ptr<BluetoothPlayerImpl> btPlayerClient(new BluetoothPlayerImpl());
    return btPlayerClient;
};


BluetoothPlayerImpl::BluetoothPlayerImpl()  {

}

BluetoothPlayerImpl::~BluetoothPlayerImpl() {

}

void BluetoothPlayerImpl::play() {
	inf(__func__);
    application::DeviceIoWrapper::getInstance()->btResumePlay();
}

void BluetoothPlayerImpl::stop() {
	inf(__func__);
    application::DeviceIoWrapper::getInstance()->btPausePlay();
}

void BluetoothPlayerImpl::pause() {
	inf(__func__);
    application::DeviceIoWrapper::getInstance()->btPausePlay();
}

void BluetoothPlayerImpl::resume() {
	inf(__func__);
    application::DeviceIoWrapper::getInstance()->btResumePlay();
}

void BluetoothPlayerImpl::playNext() {
    inf("BluetoothPlayerImpl::playNext not implemented");
	application::DeviceIoWrapper::getInstance()->btPlayNext();
}

void BluetoothPlayerImpl::playPrevious() {
    inf("BluetoothPlayerImpl::playPrevious not implemented");
	application::DeviceIoWrapper::getInstance()->btPlayPrevious();
}

void BluetoothPlayerImpl::registerListener(
    std::shared_ptr<duerOSDcsApp::mediaPlayer::LocalAudioPlayerListener> notify) {inf(__func__);
    if (nullptr != notify) {
        m_listener = notify;
    }
}

void BluetoothPlayerImpl::btStartPlay() {inf(__func__);
    if (nullptr != m_listener) {
        m_listener->btStartPlay();
    }
}

void BluetoothPlayerImpl::btStopPlay() {inf(__func__);
    if (nullptr != m_listener) {
        m_listener->btStopPlay();
    }
}

void BluetoothPlayerImpl::btDisconnect() {inf(__func__);
    if (nullptr != m_listener) {
        m_listener->btDisconnect();
    }
}

void BluetoothPlayerImpl::btPowerOff() {inf(__func__);
    if (nullptr != m_listener) {
        m_listener->btPowerOff();
    }
}

}
}