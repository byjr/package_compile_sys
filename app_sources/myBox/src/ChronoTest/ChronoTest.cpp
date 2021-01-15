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

static int help_info(int argc, char *argv[]) {
	s_err("%s help:", get_last_name(argv[0]));
	s_err("\t-i [input path]");
	s_err("\t-o [output url]");
	s_err("\t-l [logLvCtrl]");
	s_err("\t-p [logPath]");
	s_err("\t-h show help");
	return 0;
}

int ChronoTest_main(int argc, char *argv[]) {
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
			return help_info(argc, argv);
		}
	}
	auto tp = system_clock::now();
	std::cout << "纳秒1 :" << tp.time_since_epoch().count() << "\n";
	std::cout << "纳秒2 :" << time_point_cast<nanoseconds>(tp).time_since_epoch().count() << "\n";
	std::cout << "微秒级:" << time_point_cast<microseconds>(tp).time_since_epoch().count() << "\n";
	std::cout << "毫秒级:" << time_point_cast<milliseconds>(tp).time_since_epoch().count() << "\n";
	std::cout << "秒级1 :" << time_point_cast<seconds>(tp).time_since_epoch().count() << "\n";
	std::cout << "秒级2 :" << system_clock::to_time_t(system_clock::now()) << "\n";
	return 0;
}
