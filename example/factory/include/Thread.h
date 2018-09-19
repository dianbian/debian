
#ifndef BASECOM_THREAD_H
#define BASECOM_THREAD_H

#include "Atomic.h"
#include "CountDownLatch.h"
#include "noncopyable.h"

#include <functional>

#include <pthread.h>

class Thread : public noncopyable
{
	public:
	  typedef std::function<void ()> ThreadFunc;
		
		explicit Thread(const ThreadFunc&, const std::string& name = std::string());
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    explicit Thread(ThreadFunc&&, const std::string& name = std::string());
#endif
    ~Thread();
		
		void start();
		
		int join();  //return pthread_join()
		
		bool started() const { return started_; }
		
		pid_t tid() const { return tid_; }
		
		const std::string& name() const { return name_; }
		
		static int numCreated() { return numCreated_.get(); }
		
	private:
	  void setDefaultName();
		
		bool started_;
		bool joined_;
		pthread_t pthreadId_;
		pid_t tid_;
		ThreadFunc func_;
		std::string name_;
		CountDownLatch latch_;
		
		static AtomicInt32 numCreated_;
};

#endif //BASECOM_THREAD_H
