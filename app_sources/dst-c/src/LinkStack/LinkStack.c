#include <lzUtils/base.h>
#include "LinkStack.h"
LinkStack_t *LinkStack_create(LinkStackPar_t *par) {
	LinkStack_t *ptr = (LinkStack_t *)calloc(1, sizeof(LinkStack_t));
	if(!ptr) {
		s_err("malloc LinkStack_t failed");
		return  NULL;
	}
	ptr->par = par;
	return ptr;
}
int LinkStack_push(LinkStack_t *ptr, void *data) {
	LinkStackNode_t *np = (LinkStackNode_t *)malloc(sizeof(LinkStackNode_t));
	if(!np) {
		s_err("malloc LinkStackNode_t failed");
		return  -1;
	}
	np->data = data;
	if(ptr->top) {
		np->next = ptr->top;
	}
	ptr->top = np;
	ptr->count ++;
	return 0;
}
void *LinkStack_pop(LinkStack_t *ptr) {
	if(ptr->top == NULL) {
		return NULL;
	}
	LinkStackNode_t *np = ptr->top;
	void *data = ptr->top->data;
	ptr->top = ptr->top->next;
	free(np);
	ptr->count --;
	return data;
}
void *LinkStack_get(LinkStack_t *ptr, size_t idx) {
	int i = 0;
	LinkStackNode_t *np = ptr->top;
	for(; i <= ptr->count; i++, np = np->next) {
		if(i == idx) {
			return np->data;
		}
	}
	return NULL;
}
void LinkStack_show(LinkStack_t *ptr) {
	LinkStackNode_t *np = ptr->top;
	for(; np; np = np->next) {
		if(ptr->par->showOne) {
			ptr->par->showOne(np->data);
		}
	}
}
void LinkStack_destroy(LinkStack_t *ptr) {
	LinkStackNode_t *np = ptr->top;
	for(; np; np = np->next) {
		free(np);
	}
	ptr->top = NULL;
	free(ptr);
}