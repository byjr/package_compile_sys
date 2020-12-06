//
// Created by zhangtuanqing on 18-4-12.
//

#ifndef DUEROS_DCSAPP_INCLUDE_MEDIAPLAYER_DLNAPLAYERIMPL_H_
#define DUEROS_DCSAPP_INCLUDE_MEDIAPLAYER_DLNAPLAYERIMPL_H_


#include "DlnaPlayerInterface.h"
#include "LocalAudioPlayerListener.h"
#include <memory>
namespace duerOSDcsApp {
namespace mediaPlayer {
class DlnaPlayerImpl : public DlnaPlayerInterface {
public:
    DlnaPlayerImpl();

    ~DlnaPlayerImpl();

    void registerListener(std::shared_ptr<LocalAudioPlayerListener> notify);

    void play() ;

    void stop() ;

    void pause()  ;

    void resume() ;

    void playNext() ;

    void playPrevious() ;

    virtual void dlnaStartPlay() ;

    virtual void dlnaStopPlay() ;

    virtual void dlnaPausePlay() ;

    static std::shared_ptr<DlnaPlayerImpl> create();

private:
    DlnaPlayerImpl(const DlnaPlayerImpl&);

    DlnaPlayerImpl& operator=(const DlnaPlayerImpl&);

private:
    std::shared_ptr<LocalAudioPlayerListener> m_listener;
};

}
}


#endif //DUEROS_DCSAPP_INCLUDE_MEDIAPLAYER_DLNAPLAYERIMPL_H_
