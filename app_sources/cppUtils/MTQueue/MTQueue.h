#ifndef _MT_QUEUE_H
#define _MT_QUEUE_H
#include <mutex>
#include <deque>
#include <condition_variable>
#include <chrono>
namespace cppUtils {
	class MTQueuePar {
	public:
		size_t mMax;
		void (*destroyOne)(void *);
		MTQueuePar(): mMax{3}, destroyOne{NULL} {}
	};
	class MTQueue {
		std::shared_ptr<MTQueuePar> mPar;
		std::deque<void *> q;
		std::mutex mu;
		std::condition_variable cond;
		size_t maxCount;
		volatile bool mIsWaitWasExited;
		void (*destroyOne)(void *);
	public:
		MTQueue(MTQueuePar *par);
		void *read();
		void *read(size_t tdMsec);
		int write(void *one, size_t tdMsec);
		int write(void *one);
		bool cycWrite(void *one);
		void clear();
		size_t getSize();
		void *get();
		bool full();
		bool empty();
		void setWaitExitState();
		~MTQueue();
	};
}
#endif