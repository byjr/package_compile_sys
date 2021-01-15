#ifndef _APP_SOURCES_CPPUTILS_CPPUTILS_H_
#define _APP_SOURCES_CPPUTILS_CPPUTILS_H_
namespace std {
	template<typename T, typename... Ts>
	unique_ptr<T> make_uniq(Ts &&... params) {
		return unique_ptr<T>(new T(forward<Ts>(params)...));
	}
}
#include "Base64Tool/Base64Tool.h"
#include "BinSplit/BinSplit.h"
#include "CrcTool/CrcTool.h"
#include "DataBuffer/DataBuffer.h"
#include "MemPool/MemPool.h"
#include "MTQueue/MTQueue.h"
#include "UrlContex/UrlContex.h"
#endif//_APP_SOURCES_CPPUTILS_CPPUTILS_H_
