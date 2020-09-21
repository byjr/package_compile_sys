#include <lzUtils/base.h>
#include <lzUtils/alsa_ctrl/alsa_ctrl.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include <memory>
static int help_info(int argc, char *argv[]) {
	printf("%s help:\n", get_last_name(argv[0]));
	printf("\t-l [logLvCtrl]\n");
	printf("\t-p [logPath]\n");
	printf("\t-u pa reinit\n");
	printf("\t-d pa power off\n");
	printf("\t-o pa power on\n");
	printf("\t-h show help\n");
	return 0;
}
static bool PaPowerOff() {
	system("GpioCtrl.sh 504 set out 0");
	return true;
}

static bool PaPowerOn() {
	system("GpioCtrl.sh 504 set out 1");
	return true;
}

static int PaReStart() {
	const char *show_path = "/sys/devices/platform/soc/ffd00000.cbus/ffd1f000.i2c/i2c-0/0-002c/mpu6050_accelerationd";
	int fd = open(show_path, O_WRONLY | O_NONBLOCK);
	if(fd < 0) {
		show_errno(0, "open");
		return -1;
	}
	int res = write(fd, &fd, 1);
	if(res < 0) {
		show_errno(0, "write");
		return -1;
	}
	close(fd);
	system("SceneSwitch.sh stop");//stop players,pa to soundbar
	PaPowerOn();//open pa
	volatile bool IsPaInitDone = false;
	std::thread phyPlayTrd = std::thread([&IsPaInitDone]() {
		alsa_args_t plyPar_st = {
			.device 	 = "plughw:1,1",
			.sample_rate = 48000,
			.channels 	 = 2,
			.action 	 = SND_PCM_STREAM_PLAYBACK,
			.flags		 = 0,
			.fmt		 = SND_PCM_FORMAT_S16_LE,
			.ptime		 = 10000,//10ms
			.btime		 = 40000,//40ms
		};
		alsa_args_t *plyPar = &plyPar_st;
		alsa_ctrl_t *mPly = alsa_ctrl_create(plyPar);
		if(!mPly) {
			s_err("alsa_ctrl_create player failed");
			return -1;
		}
		size_t bufferSize = plyPar->ptime / 1000 * (plyPar->sample_rate * 2 * plyPar->channels / 1000);
		std::unique_ptr<char[]> buffer(new char[bufferSize]);
		if(buffer.get() == NULL) {
			s_err("new char failed");
			exit(-1);
		}
		memset(buffer.get(), 0, bufferSize);
		for(; !IsPaInitDone;) {
			ssize_t wret = alsa_ctrl_write_stream(mPly, buffer.get(), bufferSize);
			if(wret != (int)bufferSize) {
				alsa_ctrl_pause(mPly);
				usleep(plyPar->ptime);
				alsa_ctrl_resume(mPly);
			}
		}
		alsa_ctrl_close(mPly);
		return 0;
	});
	unsigned char initVal = 0;
	for(; !initVal;) {
		int fd = open(show_path, O_RDONLY | O_NONBLOCK);
		if(fd < 0) {
			show_errno(0, "open");
			return -1;
		}
		int res = read(fd, &initVal, sizeof(initVal));
		if(res < (int)sizeof(initVal)) {
			s_err("res = %d", res);
			show_errno(0, "read initVal");
		}
		if(initVal) {
			IsPaInitDone = true;
		}
		close(fd);
	}
	if(phyPlayTrd.joinable()) {
		phyPlayTrd.join();
	}
	return 0;
}

int PaOption_main(int argc, char *argv[]) {
	showCompileTmie(argv[0], s_war);
	WaitOthersInstsExit(argv[0], 20);
	int opt = 0;
	while ((opt = getopt_long_only(argc, argv, "l:p:udoh", NULL, NULL)) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		case 'u':
			return PaReStart();
		// break;
		case 'd':
			PaPowerOff();
			return 0;
		// break;
		case 'o':
			PaPowerOn();
			return 0;
		// break;
		default: /* '?' */
			return help_info(argc, argv);
		}
	}
	return help_info(argc, argv);
}