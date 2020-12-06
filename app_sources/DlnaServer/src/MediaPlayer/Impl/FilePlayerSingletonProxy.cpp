#include "FilePlayerSingletonProxy.h"

namespace duerOSDcsApp {
namespace mediaPlayer {

FilePlayerSingletonProxy* FilePlayerSingletonProxy::s_instance = NULL;
pthread_once_t  FilePlayerSingletonProxy::s_initOnce = PTHREAD_ONCE_INIT;
pthread_once_t  FilePlayerSingletonProxy::s_destroyOnce = PTHREAD_ONCE_INIT;

FilePlayerSingletonProxy* FilePlayerSingletonProxy::getInstance() {
    pthread_once(&s_initOnce, &FilePlayerSingletonProxy::init);
    return s_instance;
}

void FilePlayerSingletonProxy::release() {
    pthread_once(&s_destroyOnce, FilePlayerSingletonProxy::destroy);
}

void FilePlayerSingletonProxy::initialiaze(const std::string &alertsAudioDev, const std::string &promptAudioDev) {
    m_player = new Mp3FilePlayerImpl(alertsAudioDev, promptAudioDev);
}

void FilePlayerSingletonProxy::init() {
    if (s_instance == NULL) {
        s_instance = new FilePlayerSingletonProxy();
    }
}

void FilePlayerSingletonProxy::destroy() {
    if (s_instance) {
        delete s_instance;
        s_instance = NULL;
    }
}

FilePlayerSingletonProxy::FilePlayerSingletonProxy() : m_player(NULL) {
}

FilePlayerSingletonProxy::~FilePlayerSingletonProxy() {
    if (m_player) {
        delete m_player;
        m_player = NULL;
    }
}

void FilePlayerSingletonProxy::registerListener(std::shared_ptr<AudioPlayerListener> notify) {
}

void FilePlayerSingletonProxy::registerListener(std::shared_ptr<AudioPlayerListener> notify, FilePlayType type) {
    m_player->registerListener(notify, type);
}

void FilePlayerSingletonProxy::setFilePlayType(FilePlayType type) {
    m_player->setFilePlayType(type);
}

void FilePlayerSingletonProxy::setPlayMode(PlayMode mode, unsigned int val) {
    m_player->setPlayMode(mode, val);
}

void FilePlayerSingletonProxy::audioPlay(const std::string& url, unsigned int offset, unsigned int report_interval) {
    m_player->audioPlay(url, offset, report_interval);
}

void FilePlayerSingletonProxy::audioPause() {
    m_player->audioPause();
}

void FilePlayerSingletonProxy::audioResume() {
    m_player->audioResume();
}

void FilePlayerSingletonProxy::audioStop() {
    m_player->audioStop();
}

void FilePlayerSingletonProxy::setMute() {
    m_player->setMute();
}

void FilePlayerSingletonProxy::setUnmute() {
    m_player->setUnmute();
}

unsigned long FilePlayerSingletonProxy::getProgress() {
    return m_player->getProgress();
}

unsigned long FilePlayerSingletonProxy::getDuration() {
    return m_player->getDuration();
}

void FilePlayerSingletonProxy::setFadeIn(int timeSec) {
    m_player->setFadeIn(timeSec);
}
}
}