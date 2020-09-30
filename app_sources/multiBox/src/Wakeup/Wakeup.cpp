// example/C/demo.c

// Copyright 2017  KITT.AI (author: Guoguo Chen)

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <lzUtils/base.h>
#include <lzUtils/alsa_ctrl/alsa_ctrl.h>
#include <Wakeup/snowboy.h>
#include <getopt.h>
static int help_info(int argc, char *argv[]) {
	(void)argc;
	printf("%s help:\n", get_last_name(argv[0]));
	printf("\t-l [logLvCtrl]\n");
	printf("\t-p [logPath]\n");
	printf("\t-h show help\n");
	return 0;
}
int Wakeup_main(int argc, char *argv[]) {
	int opt = 0;
	while ((opt = getopt_long_only(argc, argv, "l:p::h", NULL, NULL)) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		default: /* '?' */
			return help_info(argc, argv);
		}
	}
	showCompileTmie(argv[0], s_war);
	WaitOthersInstsExit(argv[0], 20);
	if(chdir("/root/WakeupEvnRoot")) {
		show_errno(0, "chdir");
		return -1;
	}
	// Parameter section.
	// If you have multiple hotword models (e.g., 2), you should set
	// <model_filename> and <sensitivity_str> as follows:
	//   model_filename =
	//     "resources/models/snowboy.umdl,resources/models/smart_mirror.umdl";
	//   sensitivity_str = "0.5,0.5";
	const char resource_filename[] = "resources/common.res";
	const char model_filename[] = "resources/models/alexa_02092017.umdl";
	const char sensitivity_str[] = "0.5";
	float audio_gain = 1;
	bool apply_frontend = false;

	// Initializes Snowboy detector.
	SnowboyDetect *detector = SnowboyDetectConstructor(resource_filename,
							  model_filename);
	SnowboyDetectSetSensitivity(detector, sensitivity_str);
	SnowboyDetectSetAudioGain(detector, audio_gain);
	SnowboyDetectApplyFrontend(detector, apply_frontend);

	alsa_args_t RecPar = {
		.device 	 = "plughw:0,1",
		.sample_rate = SnowboyDetectSampleRate(detector),
		.channels 	 = (char)SnowboyDetectNumChannels(detector),
		.action 	 = SND_PCM_STREAM_CAPTURE,
		.flags		 = 0,
		.fmt		 = SND_PCM_FORMAT_S16_LE,
	};
	alsa_ctrl_t *mRec = alsa_ctrl_create(&RecPar);
	if(!mRec) {
		s_err("alsa_ctrl_create player failed");
		exit(-1);
	}
	s_inf("detector.SampleRate()=%d", SnowboyDetectSampleRate(detector));
	s_inf("detector.NumChannels()=%d", SnowboyDetectNumChannels(detector));
	s_inf("detector.BitsPerSample()=%d", SnowboyDetectBitsPerSample(detector));

	size_t period_bytes = 55000 * (RecPar.sample_rate * RecPar.channels * SnowboyDetectBitsPerSample(detector) / 8) / 1000000;
	s_inf("period_bytes=%d", period_bytes);
	size_t period_size = period_bytes / sizeof(short);
	char *buffer = (char *)malloc(period_bytes);
	if(!buffer) {
		s_err("oom");
		exit(-1);
	}
	// Runs the detection.
	s_inf("Listening... Press Ctrl+C to exit");
	s_inf("period_size=%d", period_size);
	while (true) {
		ssize_t wret = alsa_ctrl_read_stream(mRec, buffer, period_bytes);
		if(wret != (ssize_t)period_bytes) {
			s_err("alsa_ctrl_read_stream");
			alsa_ctrl_reset(mRec);
			continue;
		}
		int result = SnowboyDetectRunDetection(detector,
											   (int16_t *)buffer, period_size, false);
		if (result > 0) {
			s_war("Hotword %d detected!\n", result);
			system("aplay resources/response.wav &");
		}
	}
	free(buffer);
	alsa_ctrl_destroy(mRec);
	SnowboyDetectDestructor(detector);
	return 0;
}