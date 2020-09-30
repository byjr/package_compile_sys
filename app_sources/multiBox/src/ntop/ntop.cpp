#include <lzUtils/base.h>
int ntop_main(int argc, char *argv[]) {
	pid_t *pidTbl = NULL;
	ssize_t nPids = 1;
	int ret = -1;
	char *nArgl = NULL;
	const char *name = argv[1];
	if(!(name && name[0])) {
		s_err("Plz input process name");
		return -1;
	}
	char *argl = argv_to_argl(argv);
	if(!argl) {
		s_err("%s/argv_to_argl %s", name);
		s_err("Plz input crecet process name");
		return -1;
	}
	do {
		if(argv[2] && argv[2][0]) {
			nArgl = strstr(argl, argv[2]);
			if(!argl) {
				s_err("");
				goto exit;
			}
		}
		nPids = get_pids_by_name(argv[1], &pidTbl, nPids);
		if(nPids < 1) {
			s_err("%s/get_pids_by_name %s", argv[1]);
			goto exit;
		}
		int res = 0;
		if(nArgl) {
			res = cmd_excute("top -p %d %s", pidTbl[0], nArgl);
		} else {
			res = cmd_excute("top -p %d -Hd1", pidTbl[0]);
		}
		if(res < 0) {
			s_err("");
			goto exit;
		}
		ret = 0;
	} while(0);
exit:
	if(argl) {
		free(argl);
	}
	return 0;
}