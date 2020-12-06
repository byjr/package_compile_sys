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

#include "DCSApp/ThreadPoolWrapper.h"

namespace duerOSDcsApp {
namespace application {

ThreadPoolWrapper *ThreadPoolWrapper::s_instance = nullptr;
pthread_once_t ThreadPoolWrapper::s_initOnce = PTHREAD_ONCE_INIT;
pthread_once_t ThreadPoolWrapper::s_destroyOnce = PTHREAD_ONCE_INIT;

ThreadPoolWrapper* ThreadPoolWrapper::getInstance() {
    pthread_once(&s_initOnce, &ThreadPoolWrapper::init);
    return s_instance;
}

void ThreadPoolWrapper::releaseInstance() {
    pthread_once(&s_destroyOnce, ThreadPoolWrapper::release);
}

ThreadPoolWrapper::ThreadPoolWrapper() {
    pthread_mutex_init(&m_mutex, nullptr);
}

ThreadPoolWrapper::~ThreadPoolWrapper() {
    shutDown();
    pthread_mutex_destroy(&m_mutex);
}

void ThreadPoolWrapper::init() {
    if (s_instance == nullptr) {
        s_instance = new ThreadPoolWrapper();
    }
}

void ThreadPoolWrapper::release() {
    delete s_instance;
    s_instance = nullptr;
}

void ThreadPoolWrapper::submit(std::function<void()> function) {
    pthread_mutex_lock(&m_mutex);
    m_executor.submit(function);
    pthread_mutex_unlock(&m_mutex);
}

void ThreadPoolWrapper::shutDown() {
    m_executor.shutdown();
}

} // application
} // duerOSDcsApp
