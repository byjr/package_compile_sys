#include <lzUtils/base.h>
#include "main.h"
int MTQueue_main(int argc,char *argv[]);
int RePlayer_main(int argc,char *argv[]);
int RtPlayer_main(int argc,char *argv[]);
int BinSplit_main(int argc,char *argv[]);

app_item_t item_tbl[]={
	// ADD_APP_ITEM(MTQueue,NULL)
	// ADD_APP_ITEM(RePlayer,NULL)
	// ADD_APP_ITEM(RtPlayer,NULL)
	ADD_APP_ITEM(BinSplit,NULL)
	{0}
};
int main(int argc, char *argv[]){
	int i=0;
	for(;item_tbl[i].app_name;i++){
		if(strcmp(item_tbl[i].app_name,get_last_name(argv[0]))==0){
			return item_tbl[i].app_main(argc,argv);
		}
	}
	s_err("can't find app:%s",argv[0]);
	return -1;
}

