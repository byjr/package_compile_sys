#ifdef __cplusplus
extern "C" {
#endif
//--------------------csrb.h start-----------------------------
#ifndef CSRB_H_
#define CSRB_H_ 1
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
typedef struct csrb_t {
    size_t ri;
    size_t wi;
    volatile size_t contex;
    volatile size_t empty;
    char *head;
    size_t capacity;//读方和写方最大方单元字节数的最小公倍数
    char *tail;
    char alloc;//在栈区中取 buffer
    pthread_mutex_t *mMtx;//读写互斥锁
    volatile int isExitSeted;
} csrb_t;

csrb_t *csrb_create(char *head, size_t size);
char *csrb_write_query(csrb_t *ptr, size_t size);
char *csrb_read_query(csrb_t *ptr, size_t size);
void csrb_write_sync(csrb_t *ptr, size_t size);
void csrb_read_sync(csrb_t *ptr, size_t size);
void csrb_clear(csrb_t *ptr, char side);
void csrb_destroy(csrb_t *ptr);
int csrb_write(csrb_t *ptr, char *buf, size_t size);
int csrb_read(csrb_t *ptr, char *buf, size_t size);
void csrb_clearExit(csrb_t *ptr);
void csrb_setExit(csrb_t *ptr);
#endif
//--------------------csrb.h end------------------------------
#ifdef __cplusplus
}
#endif