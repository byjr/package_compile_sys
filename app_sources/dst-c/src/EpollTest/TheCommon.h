#ifndef TheCommon_H_
#define TheCommon_H_
#include <memory>
#include <string>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <map>
#include <atomic>
#include <vector>
#include <thread>
#include <sstream>
#include <fstream>
#include <lzUtils/common/fp_op.h>
#include <lzUtils/base.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
typedef std::pair<struct sockaddr, socklen_t> PerrAddr_t;
#endif