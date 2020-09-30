#include <lzUtils/base.h>
#include <lzUtils/alsa_ctrl/mixer_ctrl.h>
#include <lzUtils/alsa_ctrl/alsa_ctrl.h>
#include <iostream>
#include <memory>
static int help_info(int argc, char *argv[]) {
	printf("%s help:\n", get_last_name(argv[0]));
	printf("\t-l [logLvCtrl]\n");
	printf("\t-p [logPath]\n");
	printf("\t-h show help\n");
	return 0;
}
#include <getopt.h>
int mixer_main(int argc, char *argv[]) {
	std::unique_ptr<mixer_ctrl_par_t> par(new mixer_ctrl_par_t());
	if(!par.get()) {
		s_err("unique_ptr<mixer_ctrl_par_t> ");
		return -1;
	}
	memset(par.get(), 0, sizeof(mixer_ctrl_par_t));
	par->device = "default";
	par->name = "All";
	par->volMax = 100;

	alsa_args_t plyPar_st = {
		.device 	 = "default",
		.sample_rate = 44100,
		.channels 	 = 2,
		.action 	 = SND_PCM_STREAM_PLAYBACK,
		.flags		 = 0,
		.fmt		 = SND_PCM_FORMAT_S16_LE,
		.ptime		 = 20000,//20ms
		.btime		 = 100000,//100ms
	};
	alsa_args_t *plyPar = &plyPar_st;
	const char *g_paly_file_path = NULL;
	char LoopTest = 0;
	char *optstr = "n:i:c:m:M:Ll:p:h";
	int opt = -1;
	while((opt = getopt_long_only(argc, argv, optstr, NULL, NULL)) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		case 'n':
			par->name = optarg;
			break;
		case 'i':
			par->idx = atoi(optarg);
			break;
		case 'm':
			par->volMin = atoi(optarg);
			break;
		case 'M':
			par->volMax = atoi(optarg);
			break;
		case 'L':
			LoopTest = 1;
		case 'F':
			plyPar->ptime = atoi(optarg);
			break;
		case 'B':
			plyPar->btime = atoi(optarg);
			break;
		case 'D':
			plyPar->device = optarg;
			break;
		case 'f':
			g_paly_file_path = optarg;
			break;
		default: /* '?' */
			return help_info(argc, argv);
		}
	}
	alsa_ctrl_t *mPly = alsa_ctrl_create(plyPar);
	if(!mPly) {
		s_err("alsa_ctrl_create player failed");
	}
	FILE *fp = fopen(g_paly_file_path, "rb");
	if(!fp) {
		show_errno(0, "fopen");
		return -1;
	}
	// do{

	// }while(!feof(fp));
	alsa_ctrl_destroy(mPly);
	fclose(fp);
	for(;;) {
		std::unique_ptr<mixer_ctrl_t, int (*)(mixer_ctrl_t *)> ptr(mixer_ctrl_create(par.get()),
				[](mixer_ctrl_t *ptr)->int{return mixer_ctrl_destroy(ptr);});
		if(!ptr.get()) {
			s_err("unique_ptr<mixer_ctrl_t> ");
			return -1;
		}

		if(LoopTest) {
			usleep(100 * 1000);
			continue;
		}
		long int vol = 0;
		size_t channel;
		for(;;) {
			// clog(Hred,"input vol:");
			// std::cin >> vol;
			// if(vol >= par->volMin && vol <= par->volMax){
			// mixer_ctrl_setVol(ptr.get(),vol);
			// }else{
			// s_err("unvailed vol");
			// }
			s_war("input channel:");
			std::cin >> channel;
			if(channel >= 0 && channel <= 2) {
				mixer_ctrl_setChannel(ptr.get(), channel);
			} else {
				s_err("unvailed channel");
			}
		}
	}
	return 0;
}