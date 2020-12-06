//
// Created by zhangtuanqing on 18-4-12.
//

#ifndef DUEROS_DCSAPP_INCLUDE_MEDIAPLAYER_DLNAPLAYERINTERFACE_H_
#define DUEROS_DCSAPP_INCLUDE_MEDIAPLAYER_DLNAPLAYERINTERFACE_H_

#include "LocalAudioPlayerListener.h"
// #include "DCSApp/DeviceIoWrapper.h"

namespace duerOSDcsApp {
namespace mediaPlayer {
class DlnaPlayerInterface {
public:
    DlnaPlayerInterface() {}

    ~DlnaPlayerInterface() { }

    void registerListener(LocalAudioPlayerListener* notify) {};

    virtual void play(){
		
	}

    virtual void stop(){
		
	}

    virtual void pause(){
		
	}

    virtual void resume(){
		
	}

    virtual void playNext(){
		
	}

    virtual void playPrevious(){
		
	}

private:
    DlnaPlayerInterface(const DlnaPlayerInterface&);

    DlnaPlayerInterface& operator=(const DlnaPlayerInterface&);
};

}
}
#endif //DUEROS_DCSAPP_INCLUDE_MEDIAPLAYER_DLNAPLAYERINTERFACE_H_
