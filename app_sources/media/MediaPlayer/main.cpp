#include <getopt.h>
#include <lzUtils/base.h>
#include "main.h"
int main(int argc, char *argv[]) {
	int opt = 0;
	while ((opt = getopt(argc, argv, "l:p:h")) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
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
	auto playerPar = std::make_shared<MediaPlayerPar>();
	if(!playerPar.get()) {
		s_err("");
		return -1;
	}
	auto player = std::make_shared<MediaPlayer>(playerPar);
	if(!(player.get() && player->isInitDone())) {
		s_err("");
		return -1;
	}
	return 0;
}