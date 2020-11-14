#ifndef dst_c_LinkStack_H
#define dst_c_LinkStack_H
#ifdef __cplusplus
extern "C" {
#endif

typedef void (*pLinkStack_showOne)(void*);

typedef struct LinkStackPar_s{
	pLinkStack_showOne showOne;
}LinkStackPar_t;

typedef struct LinkStackNode_s{
	void* data;
	struct LinkStackNode_s* next;
}LinkStackNode_t;

typedef struct LinkStack_s{
	LinkStackPar_t *par;
	LinkStackNode_t* top;
	size_t count;
}LinkStack_t;

LinkStack_t* LinkStack_create(LinkStackPar_t*);
int LinkStack_push(LinkStack_t* ptr,void* data);
void* LinkStack_pop(LinkStack_t* ptr);
void* LinkStack_get(LinkStack_t* ptr,size_t idx);
void LinkStack_show(LinkStack_t* ptr);
void LinkStack_destroy(LinkStack_t* ptr);

#ifdef __cplusplus
}
#endif
#endif