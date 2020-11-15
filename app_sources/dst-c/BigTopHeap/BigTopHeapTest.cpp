#include <unistd.h>
#include <lzUtils/base.h>
#include "BigTopHeap.h"
#include <string>
struct Data{
	int n;
	Data(int n){
		this->n = n;
	}
};

static void showOne(Data* data){
	s_raw(",%d",data->n);
}
static int isSorted(Data* A,Data* B){
	if(A->n > B->n){
		return 1;
	}else{
		return 0;
	}
}
int BigTopHeap_main(int argc,char* argv[]){
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
	BigTopHeapPar_t args;
	args.showOne 	= pBigTopHeap_showOne (&showOne);
	args.isSorted 	= pBigTopHeap_isSorted (&isSorted);
	args.max		= 128;
	BigTopHeap_t* ptr = BigTopHeap_create(&args);
	if(!ptr){
		s_err("BigTopHeap_create failed!");
		return -1;
	}
	for(auto i=0;i<128;i++){
		auto res = BigTopHeap_push(ptr,new Data(2*i));
		if(res < 0){
			s_err("BigTopHeap_push failed!");
		}
	}
	BigTopHeap_show(ptr);
	s_inf("-------------------------------------");
	Data* dptr=nullptr;
	for(;dptr = (Data*)BigTopHeap_pop(ptr);){
		showOne(dptr);
		delete dptr;
	}
	BigTopHeap_destroy(ptr);
	return 0;
}