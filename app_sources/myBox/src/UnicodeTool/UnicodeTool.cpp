// unordered_map::erase
#include <iostream>
#include <string>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <thread>
#include <atomic>
#include <lzUtils/base.h>
#include <unistd.h>
#include <wchar.h>
int UnicodeTool_main(int argc, char *argv[]) {
	int opt  = -1;
	while ((opt = getopt(argc, argv, "l:p:h")) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		default: /* '?' */
			return 0;//help_info(argc, argv);
		}
	}

	FILE *fp = fopen("ori.txt", "rb");
	if(!fp) {
		show_errno(0, "fopen");
		return -1;
	}
#define wchar_max (256)
	char buf[wchar_max * 2];
	int res = 0;
	char *ch;
	wchar_t *wch;
	do {
		res = fread(buf, 2, wchar_max, fp);
		s_inf("res=%d", res);
		if(res <= 0) {
			if(feof(fp)) {
				break;
			}
			exit(-1);
		}
		for(int i = 0; i < res ; i++) {
			ch = buf + (i * 2);
			wch = (wchar_t *)ch ;
			s_inf("0x%02hhX%02hhX", ch[0], ch[1]);
		}
	} while(!feof(fp));
	return 0;
}
