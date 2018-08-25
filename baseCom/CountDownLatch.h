
#ifndef BASECOM_COUNTDOWNLATCH_H
#define BASECOM_COUNTDOWNLATCH_H

#include "Condition.h"
#include "Mutex.h"
#include "noncopyable.h"

class CountDownLatch : public noncopyable
{
	public:
	  explicit CountDownLatch(int count);
		
		void wait();
		
		void countDown();
		
		int getCount() const;
		
	private:
	  mutable MutexLock mutex_;
		Condition condition_;
		int count_;
};

#endif //BASECOM_COUNTDOWNLATCH_H
