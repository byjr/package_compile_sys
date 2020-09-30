#ifndef _MemPool_H
#define _MemPool_H
#include <lzUtils/base.h>
#include <mutex>
#include <vector>
namespace cppUtils {
	class MemUnit {
	public:
		bool isAct;
		void *data;
		size_t size;
		MemUnit(): isAct{true} {}
	};
	class MemPool {
		std::vector<MemUnit *> mVct;
		std::mutex mtx;
	public:
		bool add(MemUnit *one);
		bool del(void *data = NULL);
		void *get(size_t *psize = NULL);
		bool put(void *data);
		size_t getSize();
		void clear();
		void *query(int i = -1);
		~MemPool();
	};
}
#endif