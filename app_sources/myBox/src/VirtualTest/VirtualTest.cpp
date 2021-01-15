// unordered_map::erase
#include <iostream>
#include <string>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <thread>
#include <atomic>
#include <lzUtils/base.h>
#include <unistd.h>
using namespace std::chrono;
class Father {
	virtual bool getInfomation(void *ptr) = 0;
	char *buffer;
public:
	Father() {
		s_inf(__func__);
		buffer = new char[1024 * 1024];
	}

	~Father() {
		if(buffer)
			delete []buffer;
		s_inf(__func__);
	}
};
class Child: public Father {
	bool getInfomation(void *ptr) {
		return ptr == nullptr;
	}
public:
	Child() {
		s_inf(__func__);
		getInfomation(this);
	}
	~Child() {
		s_inf(__func__);
	}
};
int VirtualTest_main(int argc, char *argv[]) {
	int opt  = -1;
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
	for(;;) {
		s_inf("===========================================");
		std::unique_ptr<Father> child =
			std::unique_ptr<Father>(new Child());
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	return 0;
}
