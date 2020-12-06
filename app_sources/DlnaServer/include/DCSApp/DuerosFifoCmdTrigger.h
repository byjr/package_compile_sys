#ifndef DUEROSFIFOCMDTRIGGER_H_
#define DUEROSFIFOCMDTRIGGER_H_ 1
#include "DCSApp/DCSApplication.h"
#include "MediaPlayer/Proxy/LocalTtsProxy.h"

#include "MediaPlayer/Proxy/BlueToothPlayerProxy.h"
#include "MediaPlayer/Impl/BluetoothPlayerImpl.h"

extern "C" {
#include <libcchip/platform.h>
}

namespace duerOSDcsApp {
namespace application {
using namespace duerOSDcsApp::mediaPlayer;

class DuerosFifoCmd{	
	private:
	public:
	static DuerosFifoCmd *DuerosFifoCmdPtr;
	static DuerosFifoCmd* getInstance();
	DuerosFifoCmd(void *);
	~DuerosFifoCmd();
//	BlueToothPlayerProxy *blueToothPlayer;
	duerOSDcsApp::mediaPlayer::BluetoothPlayerImpl *BluetoothPlayerImpl;
	int startDuerosFifoCmdProc(void *arg);
	static void *DuerosFifoCmdProc(void *ptr);
};

}
}

#endif
