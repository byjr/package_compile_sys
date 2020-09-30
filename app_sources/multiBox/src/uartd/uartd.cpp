#include <lzUtils/base.h>
extern "C" {
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>
}
#include "uartd.h"
static int usbDetect_handle(ItemPar &par) {
#define USB_PAN_DEVNODE "/dev/sda"
	auto mPtr = (Uartd *)par.args;
	int res = access(USB_PAN_DEVNODE, F_OK);
	if(res < 0) {
		show_errno(0, USB_PAN_DEVNODE);
		mPtr->sendCmd("reply usbDetect failed");
		return -1;
	}
	mPtr->sendCmd("reply usbDetect pass");
	return 0;
}
//hornDetect sbar
static int hornDetect_handle(ItemPar &par) {
	auto mPtr = (Uartd *)par.args;
	char buf[512] = {0};
	if(!par.argv[1]) {
		mPtr->sendCmd("reply hornDetect failed");
		return -1;
	}
	snprintf(buf, sizeof(buf), "hornDetect.sh start %s", par.argv[1]);
	system(buf);
	mPtr->sendCmd("reply hornDetect started");
	return 0;
}
static int closePa_handle(ItemPar &par) {
	auto mPtr = (Uartd *)par.args;
	char buf[512] = {0};
	snprintf(buf, sizeof(buf), "hornDetect.sh close");
	system(buf);
	mPtr->sendCmd("reply closePa done");
	return 0;
}
static int openPa_handle(ItemPar &par) {
	auto mPtr = (Uartd *)par.args;
	char buf[512] = {0};
	snprintf(buf, sizeof(buf), "hornDetect.sh open");
	system(buf);
	mPtr->sendCmd("reply openPa done");
	return 0;
}
static int reply_handle(ItemPar &par) {
	for(int i = 0; par.argv[i]; i++) {
		s_inf("argv[%i]=%s", i, par.argv[i]);
	}
	return 0;
}
static int uacStart_handle(ItemPar &par) {
	system("SceneSwitch.sh uac");
	return 0;
}
static int lvdsStart_handle(ItemPar &par) {
	system("SceneSwitch.sh lvds");
	return 0;
}
int Uartd::sendCmd(const char *cmd) {
	std::string realCmd = "^";
	realCmd += cmd;
	realCmd += "\n";
	int res = wFifo->writeOne(realCmd);
	if(res < 0) {
		s_err("wFifo->writeOne");
		return -1;
	}
}
Uartd::Uartd(UartdPar *par) {
	mPar = par;
	if(mPar->ttHardPath) {
		std::string fifos = mPar->ttHardPath;
		size_t idx = fifos.find_first_of(':');
		if(idx == std::string::npos) {
			return ;
		}
		std::string hardReadPath = fifos.substr(0, idx);
		if(access(hardReadPath.data(), F_OK)) {
			int res = mkfifo(hardReadPath.data(), 0666);
			if(res < 0) {
				show_errno(0, "mkfifo");
				return;
			}
		}
		std::string hardWritePath = fifos.substr(idx + 1);
		if(access(hardWritePath.data(), F_OK)) {
			int res  = mkfifo(hardWritePath.data(), 0666);
			if(res < 0) {
				show_errno(0, "mkfifo");
				return;
			}
		}
		mHardWriteFd = open(hardReadPath.data(), mPar->openMode);
		if(mHardWriteFd < 0) {
			show_errno(0, "open");
			return ;
		}
		mHardReadFd = open(hardWritePath.data(), mPar->openMode);
		if(mHardReadFd < 0) {
			show_errno(0, "open");
			return ;
		}
	} else {
		mUartdFd = open(mPar->devNode, mPar->openMode);
		if(mUartdFd < 0) {
			show_errno(0, "open");
			return ;
		}
		s_inf("open %s succeed!", mPar->devNode);
		int res = setOpt(mPar->baudRate, 8, 'N', 1);
		if(res < 0) {
			s_err("setOpt failed!");
			close(mUartdFd);
			return ;
		}
		mHardWriteFd = mHardReadFd = mUartdFd;
	}
#define ADD_APP_ITEM(name,args) {#name,&name##_handle,args},
	std::vector<CmdItem> cmdVct = {
		ADD_APP_ITEM(uacStart, this)
		ADD_APP_ITEM(lvdsStart, this)
		ADD_APP_ITEM(usbDetect, this)
		ADD_APP_ITEM(hornDetect, this)
		ADD_APP_ITEM(closePa, this)
		ADD_APP_ITEM(openPa, this)
		ADD_APP_ITEM(reply, this)
	};
	mCmdVct = cmdVct;
	rFifo = new CondFifo(50);
	wFifo = new CondFifo(25);
	rUartThread = std::thread([this]() {
		HardReadProcess();
	});
	wUartThread = std::thread([this]() {
		HardWriteProcess();
	});
	parseThread = std::thread([this]() {
		CmdparseProcess();
	});
	if(mPar->ttMasterFlag) {
		ttMasterThread = std::thread([this]() {
			TtMasterProcess();
		});
	}
	ClientExitFlag = false;
}
int Uartd::TtMasterProcess() {
	char buf[512];
	for(;;) {
		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf), stdin);
		sendCmd(buf);
	}
}
Uartd::~Uartd() {
	if(rUartThread.joinable()) {
		rUartThread.join();
	}
	if(wUartThread.joinable()) {
		wUartThread.join();
	}
	delete wFifo;
	delete rFifo;
	close(mUartdFd);
}
int Uartd::setOpt(int nSpeed, int nBits, char nEvent, int nStop) {
	struct termios newtio, oldtio;
	if ( tcgetattr( mUartdFd, &oldtio) != 0) {
		s_err("SetupSerial 1");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch( nBits ) {
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 8:
		newtio.c_cflag |= CS8;
		break;
	}

	switch( nEvent ) {
	case 'O':
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD;
		newtio.c_iflag |= (INPCK | ISTRIP);
		break;
	case 'E':
		newtio.c_iflag |= (INPCK | ISTRIP);
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'N':
		newtio.c_cflag &= ~PARENB;
		break;
	}
	switch( nSpeed ) {
	case 9600:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	case 38400:
		cfsetispeed(&newtio, B38400);
		cfsetospeed(&newtio, B38400);
		break;
	case 115200:
		cfsetispeed(&newtio, B115200);
		cfsetospeed(&newtio, B115200);
		break;
	case 1500000:
		cfsetispeed(&newtio, B1500000);
		cfsetospeed(&newtio, B1500000);
		break;
	default:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	}
	if ( nStop == 1 ) {
		newtio.c_cflag &= ~CSTOPB;
	} else if ( nStop == 2 ) {
		newtio.c_cflag |= CSTOPB;
	}
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN]  = 1;
	tcflush(mUartdFd, TCIFLUSH);
	if ((tcsetattr(mUartdFd, TCSANOW, &newtio)) != 0) {
		s_err("com set error");
		return -1;
	}
	return 0;
}
int Uartd::isHardWriteable() {
	fd_set wd;
	FD_ZERO(&wd);
	FD_SET(mHardWriteFd, &wd);
	int maxfd = mHardWriteFd + 1;
	struct timeval tv = {0, 1000 * 100};
	int ret = select(maxfd, NULL, &wd, NULL, &tv);
	if(ret < 0) {
		show_errno(0, "select");
		return -1;
	}
	if(!ret) {
		// show_errno(0,"select timer out");
		return -2;
	}
	return 0;
}
int Uartd::HardWriteProcess() {
	for(;;) {
		std::string cmd = wFifo->readOne();
		size_t count = 0;
		s_inf("write cmd:%s", cmd.data());
		show_rt_time("write start...", s_dbg);
		for(; count < cmd.length();) {
			while(isHardWriteable() < 0);
			int res = write(mHardWriteFd, cmd.data() + count, cmd.length() - count);
			if(res <= 0) {
				show_errno(0, "write");
				continue;
			}
			count += res;
		}
		show_rt_time("write done.", s_dbg);
	}
}
int Uartd::CmdparseProcess() {
	for(;;) {
		std::string ori = rFifo->readOne();
		std::string cmd = ori.substr(1, ori.length() - 1);
		ItemPar par;
		par.argv = argl_to_argv((char *)cmd.data(), &par.argc);
		if(!(par.argv && par.argv[0])) {
			s_err("argl_to_argv failde");
			continue;
		}
		int is_match_a_cmd = 0;
		for(auto cmd : mCmdVct) {
			if(0 == strcmp(cmd.name, par.argv[0])) {
				is_match_a_cmd = 1;
				par.args =  cmd.args;
				int res =  cmd.handle(par);
				if(res < 0) {
					s_err("cmd:%s failed!", par.argv[0]);
				}
				break;
			}
		}
		if(!is_match_a_cmd) {
			s_err("cmd:%s not found!", par.argv[0]);
		}
		argv_free(par.argv);
	}
}
int Uartd::isHardReadable() {
	fd_set rd;
	FD_ZERO(&rd);
	FD_SET(mHardReadFd, &rd);
	int maxfd = mHardReadFd + 1;
	struct timeval tv = {0, 1000 * 100};
	int ret = select(maxfd, &rd, NULL, NULL, &tv);
	if(ret < 0) {
		show_errno(0, "select");
		return -1;
	}
	if(!ret) {
		// show_errno(0,"select timer out");
		return -2;
	}
	return 0;
}
int Uartd::HardReadProcess() {
	char buf[128];
	std::string req;
	size_t  hIdx = 0;
	size_t  tIdx = 0;
	bool need_to_get_new_cmd = true;
	for(;;) {
		while(isHardReadable() < 0);
		show_rt_time("read start...", s_dbg);
		memset(buf, 0, sizeof(buf));
		int res = read(mHardReadFd, buf, sizeof(buf));
		if(res <= 0) {
			show_errno(0, "read");
			continue;
		}
		req += buf;
		if(need_to_get_new_cmd) {
			need_to_get_new_cmd = false;
			hIdx = req.find_first_of('^');
			if(hIdx == std::string::npos) {
				req = "";
				need_to_get_new_cmd = true;
				continue;
			} else { //founded
				req = req.substr(hIdx);
			}
		}
		tIdx = req.find_first_of('\n');
		if(tIdx != std::string::npos) { //founded
			std::string cmd = req.substr(0, tIdx);
			show_rt_time("read done.", s_dbg);
			s_inf("read cmd:%s", cmd.data());
			rFifo->writeOne(cmd);
			req = req.substr(tIdx + 1);
			need_to_get_new_cmd = true;
		}
	}
}
static int waitTas5805InitResponse() {
#define Tas5805_NODE_PATH "/sys/devices/platform/soc/ffd00000.cbus/ffd1f000.i2c/i2c-0/0-002e/mpu6050_accelerationd"
#define Tas5805_INITED_NOTE_PATH "/tmp/tas5805_init_done"
	if(access(Tas5805_INITED_NOTE_PATH, F_OK) == 0) {
		return 0;
	}
	system("Tas5805Init.sh start");
	unsigned char initVal = 0;
	do {
		int fd = open(Tas5805_NODE_PATH, O_RDONLY);
		if(fd < 0) {
			show_errno(0, "open");
			usleep(100 * 1000);
			continue;
		}
		int res = read(fd, &initVal, sizeof(initVal));
		if(res < sizeof(initVal)) {
			s_err("res = %d", res);
			show_errno(0, "read initVal");
		}
		close(fd);
		if(initVal) {
			break;
		}
		usleep(100 * 1000);
	} while(!initVal);
Err:
	if(initVal) {
		system("Tas5805Init.sh stop");
		system("echo 1 > " Tas5805_INITED_NOTE_PATH);
		return 0;
	}
	return -1;
}
static int help_info(int argc, char *argv[]) {
	printf("%s help:\n", get_last_name(argv[0]));
	printf("\t-t [rFifo:wFifo]:use fifo to test logic.\n");
	printf("\t-m :Use stdin to simulate the Mastr.\n");
	printf("\t-b [baudRate]:Plz use 9600/38400/115200/1500000.\n");
	printf("\t-l [logLvCtrl]\n");
	printf("\t-p [logPath]\n");
	printf("\t-h show help\n");
	return 0;
}
#include <getopt.h>
int uartd_main(int argc, char *argv[]) {
	int opt = 0;
	UartdPar uartdPar = {
		.devNode = "/dev/ttyS2",
		.baudRate = 9600,
		.openMode = O_RDWR | O_NOCTTY | O_NDELAY,
	};
	while ((opt = getopt_long_only(argc, argv, "b:mt:l:p:th", NULL, NULL)) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		case 't':
			uartdPar.ttHardPath = optarg;
			break;
		case 'm':
			uartdPar.ttMasterFlag = true;
		case 'b':
			uartdPar.baudRate = atoi(optarg);
			break;
		default: /* '?' */
			return help_info(argc, argv);
		}
	}
	//打印编译时间
	showCompileTmie(argv[0], s_war);
	int res = waitTas5805InitResponse();
	if(res < 0) {
		s_err("waitTas5805Init failed!");
		return -1;
	}
	auto uartd = new Uartd(&uartdPar);
	if(!uartd) {
		s_err("new Uartd failed!");
		return -1;
	}
	for(; !uartd->getExitFlag();) {
		pause();
	}
	//-----------------------------
	delete uartd;
	return -1;
}

