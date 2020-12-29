#include <pthread.h>
#include <lzUtils/base.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <fstream>
#include <vector>
#include "shrb.h"
static char* mBuffer = NULL;
static size_t mSize = 10*1024 + sizeof(ShareMemSync);
#define SHMEME_PATCH "/shm_test"
static char* shrb_create(const char *name, size_t size) {
	int fail = 0;
	int fd = shm_open(name, O_RDWR | O_CREAT, 0777);
	if(fd <= 0) {
		show_errno(0, "shm_open");
		return NULL;
	}
	char *_shrb = NULL;
	size_t bytes = size;
	do {
		int ret = ftruncate(fd, bytes);
		if(ret < 0) {
			show_errno(0, "ftruncate");
			fail = 1;
			break;
		}
		_shrb = (char *)mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		s_inf("_shrb=%p", _shrb);
		if(!_shrb) {
			show_errno(0, "mmap");
			fail = 2;
			break;
		}		
		close(fd);
	} while(0);
	switch(fail) {
	case 0:
		return _shrb;
	case 3:
		munmap(_shrb, bytes);
	case 2:
	case 1:
		close(fd);
		shm_unlink(name);
	default:
		return NULL;
	}
}
char *shrb_get(const char *name, size_t size) {
	int fail = 0;
	int fd = shm_open(name, O_RDWR | O_EXCL, 0777);
	if(fd <= 0) {
		show_errno(0, "shrb_get/shm_open");
		return shrb_create(name, size);
	}	
	char *_shrb = NULL;
	size_t bytes = sizeof(char) + size * 2;
	do {
		_shrb = (char *)mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		s_inf("_shrb=%p", _shrb);
		if(!_shrb) {
			show_errno(0, "mmap");
			fail = 1;
			break;
		}
		close(fd);
	} while(0);
	switch(fail) {
	case 0:
		return _shrb;
	case 1:
		close(fd);
		shm_unlink(name);
	default:
		return NULL;
	}
}
void shrb_detach(char *ptr) {
	if(!ptr) {
		return;
	}
	munmap(ptr, mSize);
}
int shrbd_main(int argc,char* argv[]){
	int opt  =-1;
	while ((opt = getopt(argc, argv, "l:p:h")) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		default: /* '?' */
			return -1;
		}
	}
	char *mBuffer = shrb_get(SHMEME_PATCH,mSize);
	if(mBuffer == NULL){
		s_err("");
		return -1;
	}	
	auto mShm = (ShareMemSync*) mBuffer;
	memset(mBuffer,0,sizeof(ShareMemSync));
	mShm->init(mSize);	
	mShm->checkInfo();
	std::ifstream ifs("in.bin",std::ios::binary);
	if(ifs.is_open() == false){
		s_err("");
		return -1;
	}
	std::vector<char> buf(2345);
	size_t offset = 0;
	sleep(2);
	for(;;){
		ifs.read(buf.data(),buf.size());
		if(ifs.gcount() <= 0){
			if(ifs.eof()){
				s_war("EOF");
				break;
			}
			show_errno(0,read);			
		}		
		while(!mShm->write(buf.data(),ifs.gcount())){
			usleep(10*1000);
		}
		offset += ifs.gcount();
		s_inf("offset=%d",offset);
	}
	ifs.close();
    return 0;
}
int shrbc_main(int argc,char* argv[]){
	int opt  =-1;
	while ((opt = getopt(argc, argv, "l:p:h")) != -1) {
		switch (opt) {
		case 'l':
			lzUtils_logInit(optarg, NULL);
			break;
		case 'p':
			lzUtils_logInit(NULL, optarg);
			break;
		default: /* '?' */
			return -1;
		}
	}
	char *mBuffer = shrb_get(SHMEME_PATCH,mSize);
	if(mBuffer == NULL){
		s_err("");
		return -1;
	}
	auto mShm = (ShareMemSync*) mBuffer;
	mShm->checkInfo();
	std::ofstream ofs("out.bin",std::ios::binary);
	if(ofs.is_open() == false){
		s_err("");
		return -1;
	}
	std::vector<char> buf(1234);
	size_t res = 0;
	size_t offset=0;
	for(;;){		
		res = mShm->remRead(buf.data(),buf.size());
		if(res == 0){
			usleep(1000*2);
			continue;
		}
		offset += res;
		ofs.write(buf.data(),res);
		if(ofs.good() == false){
			show_errno(0,wrute);			
		}
		ofs.flush();
		s_inf("offset=%d",offset);
	}
	ofs.close();
    return 0;	
    return 0;
}