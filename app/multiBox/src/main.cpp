#include <lzUtils/base.h>
#include <unistd.h>
#include "main.h"

extern int multiBox_main(int argc, char *argv[]);
extern int allPlayer_main(int argc, char *argv[]);
extern int hotplug_main(int argc, char *argv[]);
extern int uartd_main(int argc, char *argv[]);
extern int SockServ_main(int argc, char *argv[]);
extern int mixer_main(int argc, char *argv[]);
extern int RtPlayer_main(int argc, char *argv[]);
extern int Base64Tool_main(int argc, char *argv[]);
extern int CrcTool_main(int argc, char *argv[]);
extern int BurnFw_main(int argc, char *argv[]);
extern int PaOption_main(int argc, char *argv[]);
extern int Misc_main(int argc, char *argv[]);
extern int AppManager_main(int argc, char *argv[]);
extern int ntop_main(int argc, char *argv[]);
extern int RtPlayer_main(int argc, char *argv[]);
extern int Broadcast_main(int argc, char *argv[]);
extern int RecvBc_main(int argc, char *argv[]);
extern int HttpCli_main(int argc, char *argv[]);
app_item_t item_tbl[] = {
	ADD_APP_ITEM(multiBox, NULL)
#ifdef Misc_ENABLE
	ADD_APP_ITEM(Misc, NULL)
#endif
#ifdef allPlay_ENABLE
	ADD_APP_ITEM(allPlayer, NULL)
#endif
#ifdef hotplug_ENABLE
	ADD_APP_ITEM(hotplug, NULL)
#endif
#ifdef uartd_ENABLE
	ADD_APP_ITEM(uartd, NULL)
	ADD_APP_ITEM(RtPlayer, NULL)
	{"I2sToUac", RtPlayer_main},
	{"UacToPhy", RtPlayer_main},
#endif
#ifdef SockServ_ENABLE
	ADD_APP_ITEM(SockServ, NULL)
#endif
#ifdef Base64Tool_ENABLE
	ADD_APP_ITEM(Base64Tool, NULL)
#endif
#ifdef BurnFw_ENABLE
	ADD_APP_ITEM(BurnFw, NULL)
#endif
#ifdef PaOption_ENABLE
	ADD_APP_ITEM(PaOption, NULL)
#endif
#ifdef CrcTool_ENABLE
	ADD_APP_ITEM(CrcTool, NULL)
#endif
#ifdef mixer_ENABLE
	ADD_APP_ITEM(mixer, NULL)
#endif
#ifdef AppManager_ENABLE
	ADD_APP_ITEM(AppManager, NULL)
#endif
#ifdef ntop_ENABLE
	ADD_APP_ITEM(ntop, NULL)
#endif
#ifdef RtPlayer_ENABLE
	ADD_APP_ITEM(RtPlayer, NULL)
#endif
#ifdef Broadcast_ENABLE
	ADD_APP_ITEM(Broadcast, NULL)
#endif
#ifdef RecvBc_ENABLE
	ADD_APP_ITEM(RecvBc, NULL)
#endif
#ifdef HttpCli_ENABLE
	ADD_APP_ITEM(HttpCli, NULL)
#endif
	{
		0
	}
};
int multiBox_main(int argc, char *argv[]) {
	const char *app_dir = "/usr/bin";
	char *app_path = NULL;
	int res = 0;
	int i = 0;
	for(; item_tbl[i].app_name; i++) {
		asprintf(&app_path, "%s/%s", app_dir, item_tbl[i].app_name);
		if(!app_path) {
			s_err("%s/asprintf oom");
			return -1;
		}
		if(access(app_path, F_OK)) {
			res = cmd_excute("ln -sf %s/multiBox %s", app_dir, app_path);
			if(res < 0) {
				s_err("ln -sf %s/multiBox %s failed", app_dir, app_path);
				return -1;
			}
		}
		s_raw("%s is active\n", app_path);
		free(app_path);
		app_path = NULL;
	}
	return 0;
}
int main(int argc, char *argv[]) {
	int i = 0;
	for(; item_tbl[i].app_name; i++) {
		if(strcmp(item_tbl[i].app_name, get_last_name(argv[0])) == 0) {
			return item_tbl[i].app_main(argc, argv);
		}
	}
	s_err("can't find app:%s", argv[0]);
	return -1;
}
