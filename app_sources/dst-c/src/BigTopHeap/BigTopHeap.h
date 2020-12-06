#ifndef dst_c_BigTopHeap_H
#define dst_c_BigTopHeap_H
#ifdef __cplusplus
extern "C" {
#endif

typedef void (*pBigTopHeap_showOne)(void*);
typedef int (*pBigTopHeap_isSorted)(void*,void*);

typedef struct BigTopHeapPar_s{
	pBigTopHeap_showOne showOne;
	pBigTopHeap_isSorted isSorted;
	size_t max;//容量
}BigTopHeapPar_t;

typedef struct BigTopHeap_s{
	BigTopHeapPar_t *par;
	void** A;
	size_t count;
}BigTopHeap_t;

BigTopHeap_t* BigTopHeap_create(BigTopHeapPar_t*);
int BigTopHeap_push(BigTopHeap_t* ptr,void* data);
void* BigTopHeap_pop(BigTopHeap_t* ptr);
void* BigTopHeap_get(BigTopHeap_t* ptr,size_t idx);
void BigTopHeap_show(BigTopHeap_t* ptr);
void BigTopHeap_destroy(BigTopHeap_t* ptr);

#ifdef __cplusplus
}
#endif
#endif