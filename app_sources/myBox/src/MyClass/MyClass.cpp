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
enum class SocktState{
	min = -1,
	rAble,
	wAble,
	closed,
	max
};
static int help_info(int argc, char *argv[]) {
	s_err("%s help:", get_last_name(argv[0]));
	s_err("\t-i [input path]");
	s_err("\t-o [output url]");
	s_err("\t-l [logLvCtrl]");
	s_err("\t-p [logPath]");
	s_err("\t-h show help");
	return 0;
}
class MyClass{
	std::thread aaa	;
	std::thread bbb	;
	std::mutex mu;
	std::condition_variable cv;
	SocktState mSktSt;
	std::atomic<bool> gotExitFlag;
public:	
	bool waitState(SocktState state){
		std::unique_lock<std::mutex> lk(mu);
		auto unint_dur = std::chrono::milliseconds(100);
		int count = std::chrono::seconds(10) / unint_dur;
		mSktSt = SocktState::min;	
		for(int i = 0;i < count;i++){
			if(cv.wait_for(lk,unint_dur,[this,state]()->bool{
					return !(!gotExitFlag && mSktSt != state);})){
				break;
			}
		}
		return (!gotExitFlag);
	}
	void notifyState(SocktState state){
		std::unique_lock<std::mutex> lk(mu);
		mSktSt = SocktState::rAble;
		lk.unlock();
		cv.notify_one();		
	}
	MyClass(){
		mSktSt = SocktState::min;
		gotExitFlag	= false;
		aaa = std::thread([this](){
			while(1){
				inf_t("wait:%d",waitState(SocktState::rAble)?1:0);			
			}
			
		});
		bbb = std::thread([this](){
			do{
				std::this_thread::sleep_for(std::chrono::seconds(3));
				notifyState(SocktState::rAble);
			}while(1);
		});			
	}
	~MyClass(){
		if(aaa.joinable()){
			aaa.join();
		}
		if(aaa.joinable()){
			bbb.join();
		}	
	}
};
int MyClass_main(int argc,char* argv[]){
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
			return help_info(argc, argv);
		}
	}		
	std::unique_ptr<MyClass> myClass = 
		std::unique_ptr<MyClass>(new MyClass);	
    return 0;
}
