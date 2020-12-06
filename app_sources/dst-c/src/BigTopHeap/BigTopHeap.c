#include <lzUtils/base.h>
#include "BigTopHeap.h"
BigTopHeap_t* BigTopHeap_create(BigTopHeapPar_t* par){
	BigTopHeap_t* ptr = (BigTopHeap_t*)calloc(1,sizeof(BigTopHeap_t));
	if(!ptr){
		s_err("malloc BigTopHeap_t failed");
		return  NULL;		
	}
	ptr->par = par;
	ptr->A = (void*)calloc(par->max+1,sizeof(void*));//use [1 2 3...max]
	if(!ptr->A){
		s_err("BigTopHeap_create/malloc ptr->A failed!");
		goto exit;
	}
	return ptr;
exit:
	if(ptr){
		free(ptr);
	}
	return NULL;
}
int BigTopHeap_push(BigTopHeap_t* ptr,void* data){
	if(ptr->count > ptr->par->max){
		s_err("push failed,the heap is full!");
		return -1;
	}
	ptr->A[ptr->count+1] = data;
	ptr->count ++;
	int i = ptr->count;
	for(;i>1;i/=2){
		if(ptr->par->isSorted(ptr->A[i/2],ptr->A[i])){
			break;
		}
		void* t = ptr->A[i];
		ptr->A[i] = ptr->A[i/2];
		ptr->A[i/2] = t;		
	}
	return 0;
}
void* BigTopHeap_pop(BigTopHeap_t* ptr){
	if(ptr->count == 0){
		return NULL;		
	}
	void* ret = ptr->A[1];
	ptr->A[1] = ptr->A[ptr->count];
	int i = 1,j=2;
	ptr->count --;
	for(;i*2+1 <= ptr->count;i=j){
		if(ptr->par->isSorted(ptr->A[i*2],ptr->A[i*2+1])){
			j=i*2;
		}else{
			j=i*2+1;
		}		
		if(ptr->par->isSorted(ptr->A[i],ptr->A[j])){			
			break;
		}
		void* t = ptr->A[i];
		ptr->A[i] = ptr->A[j];
		ptr->A[j] = t;
	}
	return ret;
}
void* BigTopHeap_get(BigTopHeap_t* ptr,size_t idx){
	if(ptr->count == 0){
		s_err("BigTopHeap_pop failed,the heap is empty!");
		return NULL;		
	}
	return ptr->A[1];
}
void BigTopHeap_show(BigTopHeap_t* ptr){
	int i = 1;
	s_inf("=========:");
	for(;i<=ptr->count;i++){
		if(ptr->par->showOne){
			ptr->par->showOne(ptr->A[i]);
		}	
	}
	s_raw("\n");
}
void BigTopHeap_destroy(BigTopHeap_t* ptr){
	if(!ptr){
		return ;
	}
	if(ptr->A){
		free(ptr->A);	
	}
	free(ptr);
}