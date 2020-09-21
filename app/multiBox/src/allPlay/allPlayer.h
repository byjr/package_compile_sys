#ifndef allPlayer_H_
#define allPlayer_H_
extern "C" {
#include <lzUtils/base.h>
#include <lzUtils/user_fifo/user_fifo.h>
#ifdef USER_LIBCCHIP
#define allPlayerLogIint(ctrl,path) lzUtils_logInit(ctrl,path)
#define allPlayerRaw(x...) s_trc(x)
#define allPlayerWar(x...) s_war(x)
#define allPlayerInf(x...) s_inf(x)
#define allPlayerErr(x...) s_err(x)
#define allPlayerDbg(x...) s_dbg(x)
#define allPlayerTrc(x...) s_trc(x)
#define allPlayerPerr(err,x...) show_errno(err,x)
#else
#define allPlayerLogIint(ctrl,path)
#define allPlayerRaw(x...)
#define allPlayerWar(x...)
#define allPlayerInf(x...)
#define allPlayerErr(x...)
#define allPlayerDbg(x...)
#define allPlayerTrc(x...)
#define traceSig(x...)
#define allPlayerPerr(x...)
#endif


}

typedef struct handle_item_t {
	const char *name;
	int (*handle)(char *argv[]);
	void *args;
} handle_item_t;

#endif
