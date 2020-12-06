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

#ifndef DUEROS_DCS_APP_SAMPLEAPP_THREADPOOLWRAPPER_H_
#define DUEROS_DCS_APP_SAMPLEAPP_THREADPOOLWRAPPER_H_

#include <cstdio>
#include "DeviceTools/Threading/Executor.h"
#include <unistd.h>
#include <cstdlib>
#include <functional>
#include <queue>

namespace duerOSDcsApp {
namespace application {

class ThreadPoolWrapper {
public:
    static ThreadPoolWrapper* getInstance();

    static void releaseInstance();

    void submit(std::function<void()> function);

    void shutDown();

private:
    explicit ThreadPoolWrapper();

    virtual ~ThreadPoolWrapper();

    ThreadPoolWrapper(const ThreadPoolWrapper&);

    ThreadPoolWrapper& operator=(const ThreadPoolWrapper&);

    static void init();

    static void release();

    static ThreadPoolWrapper *s_instance;
    static pthread_once_t s_initOnce;
    static pthread_once_t s_destroyOnce;
    deviceCommonLib::deviceTools::threading::Executor m_executor;
    pthread_mutex_t m_mutex;
};

} // application
} // duerOSDcsApp

#endif // DUEROS_DCS_APP_SAMPLEAPP_THREADPOOLWRAPPER_H_
