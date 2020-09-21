#ifndef _MemPool_H
#define _MemPool_H
#include <mutex>
#include <vector>
#include <lzUtils/base.h>
namespace cppUtils {
	class MemUnit {
	public:
		bool isAct;
		void *data;
		size_t size;
		MemUnit(): isAct{true} {}
		// MemUnit(size_t len):
		// isAct{true}	{
		// s_trc(__func__);
		// data = new char[len];
		// if(!data){
		// s_err("new char[%d]",len);
		// return ;
		// }
		// size = len;
		// }
		// ~MemUnit(){
		// s_trc(__func__);
		// delete [](char *)data;
		// }
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