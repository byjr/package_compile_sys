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
#include <string.h>
#include <unistd.h>
namespace std{
	template<typename T, typename... Ts>
	std::unique_ptr<T> make_unq(Ts&&... params){
		return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
	}
}
#ifdef USE_LZ_UTILS
#include <lzUtils/base.h>
#include <lzUtils/common/fd_op.h>
#include <lzUtils/common/fp_op.h>
#else
#include "HttpdUtils.h"
#define set_thread_name HttpdUtils::setTrdName
#define get_size_by_path HttpdUtils::getBytes
#define fd_set_flag HttpdUtils::setFdFlag
#endif
#include "DataBuffer.h"

#endif