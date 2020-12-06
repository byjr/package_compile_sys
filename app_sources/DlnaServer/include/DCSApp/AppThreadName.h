/*
 * Copyright (c) 2018 Baidu, Inc. All Rights Reserved.
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

#ifndef DUEROS_DCS_APP_APPTHREADNAME_H_
#define DUEROS_DCS_APP_APPTHREADNAME_H_

#include <string>

namespace duerOSDcsApp {
namespace application {

// AudioRecorder::recorderOpen
static const std::string APP_THREAD_NAME_RECORDER = "APP_recorder";

// UpnpTransport::upnp_transport_init
static const std::string APP_THREAD_NAME_UPNPTRANSPORT = "APP_UpnpTrans";

// DuerLinkMtkInstance::start_network_monitor
static const std::string APP_THREAD_NAME_DUERLINK_STARTNETWORKMONITOR = "APP_DuerLink_net";

// DuerLinkMtkInstance::start_verify_network
static const std::string APP_THREAD_NAME_DUERLINK_STARTVERIFYNETWORK = "APP_DL_verify";

// ActivityMonitorSingleton::ActivityMonitorSingleton
static const std::string APP_THREAD_NAME_ACTIVITYMONITORSINGLETON = "APP_ActMonitor";

// SystemUpdateRevWrapper::SystemUpdateRevWrapper
static const std::string APP_THREAD_NAME_OTA = "APP_OTA";

}  // namespace application
}  // namespace duerOSDcsApp

#endif  // DUEROS_DCS_APP_APPTHREADNAME_H_

