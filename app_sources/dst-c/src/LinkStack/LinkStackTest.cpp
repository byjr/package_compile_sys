#include <unistd.h>
#include <lzUtils/base.h>
#include "LinkStack.h"
#include <string>
struct Data{
	int n;
	Data(int n){
		this->n = n;
	}
};

static void showOne(Data* data){
	s_inf("n=%d",data->n);
}
int LinkStack_main(int argc,char* argv[]){
	int c = 0;
	while ((c = getopt(argc,argv,"l:ph")) != -1) {
		switch (c) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		default: /* '?' */
			s_err("no para!");
		}
	}
	LinkStackPar_t args;
	args.showOne = pLinkStack_showOne (&showOne);
	LinkStack_t* ptr = LinkStack_create(&args);
	if(!ptr){
		s_err("LinkStack_create failed!");
		return -1;
	}
	for(auto i=0;i<10;i++){
		auto res = LinkStack_push(ptr,new Data(2*i));
		if(res < 0){
			s_err("LinkStack_push failed!");			
		}
	}
	LinkStack_show(ptr);
	s_inf("-------------------------------------");
	Data* dptr=nullptr;
	for(;dptr = (Data*)LinkStack_pop(ptr);){
		showOne(dptr);
		delete dptr;
	}
	LinkStack_destroy(ptr);
	return 0;
}