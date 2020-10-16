#include <getopt.h>
#include "allPlayer.h"
#include "AudioPlayerInterface.h"
#include "AudioPlayerImpl.h"
using namespace duerOSDcsApp::mediaPlayer;
AudioPlayerImpl *playerPtr = NULL;
char thePlayerIsDaemon = 0;
int addUrl_handle(item_arg_t *arg) {
	char **argv = arg->argv;
	assert_arg(1, -1);
	if(!playerPtr) {
		allPlayerErr("playerPtr is NULL");
		return -2;
	}
	playerPtr->audioPlay(argv[1], RES_FORMAT::AUDIO_MP3, 0, 15000);
	return 0;
}
cmd_item_t fifo_cmd_tbl[] = {
	ADD_HANDLE_ITEM(addUrl, NULL) {
		0
	}
};
fifo_cmd_t fifo_cmd_args = {
	.type = FIFO_CMD_UNSPECIFIED_FIFO,
	.path = "/tmp/allPlayer.fifo",
	.tbl = fifo_cmd_tbl
};
int allPlayerProxyInit(AudioPlayerImpl *playerIstPtr) {
	int ret = user_fifo_read_init(&fifo_cmd_args);
	if(ret < 0) {
		allPlayerErr("user_fifo_read_init failed!");
		return -1;
	}
	user_fifo_read_proc(&fifo_cmd_args);
	allPlayerErr("user_fifo_read_proc failed!");
	return 0;
}
int main(int argc, char *argv[]) {
	int opt = 0;
	char *alsaDevice = "default";
	bool loopPlayFlag = false;
	while ((opt = getopt_long_only(argc, argv, "D:l:p:odkh", NULL, NULL)) != -1) {
		switch (opt) {
		case 'D':
			alsaDevice = optarg;
			break;
		case 'l':
			allPlayerLogIint(optarg, NULL);
			break;
		case 'p':
			allPlayerLogIint(NULL, optarg);
			break;
		case 'd':
			thePlayerIsDaemon = 1;
			break;
		case 'k':
			my_popen("killall %s", get_last_name(argv[0]));
			break;
		case 'o':
			loopPlayFlag = true;
			break;
		case 'h':
			printf("%s help:\n", get_last_name(argv[0]));
			printf("\t-D [alsaDevice]\n");
			printf("\t-l [logLvCtrl]\n");
			printf("\t-d as a daemon excute\n");
			printf("\t-k killall %s\n", get_last_name(argv[0]));
			printf("\t-h show help\n");
			return 0;
		default: /* '?' */
			printf("\terr option!\n");
			return -1;
		}
	}
	int ret = 0;
	AudioPlayerImpl playerIst(alsaDevice);
	playerPtr = &playerIst;
	for(; optind < argc; optind++) {
		char *curPlayUrl = argv[optind];
		allPlayerInf("palying %s ...", curPlayUrl);
		do {
			playerIst.audioPlay(curPlayUrl, RES_FORMAT::AUDIO_MP3, 0, 15000);
		} while(loopPlayFlag);

	}
	while(!playerIst.getPlayerState()) {
		usleep(100 * 1000);
	}
	if(thePlayerIsDaemon) {
		ret = allPlayerProxyInit(&playerIst);
		if(ret < 0) {
			allPlayerErr("allPlayerProxyInit failure!");
			return -1;
		}
		while(1)pause();
	}
	return 0;
}