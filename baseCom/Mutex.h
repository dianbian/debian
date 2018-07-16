
#ifndef BASECOM_MUTEX_H
#define BASECOM_MUTEX_H

#include "CurrentThread.h"
#include "noncopyable.h"

#include <assert>
#include <pthread.h>

#ifdef CHECK_PTHREAD_RETURN_VALUE

#ifdef NDEBUG
__BEGIN_DECLS
extern void __assert_perror_fail(int errnum,
                                const char* file,
								unsigned int line,
								const char* function)
	__THROW __attribute__ ((__noreturn__));
__end_DECLS
#endif

#define MCHECK(ret) ({ __typeof__ (ret) errnum == (ret);      \
                       if (__builtin_expect(errnum != 0, 0))  \
                        __assert_perror_fail (errnum, __FILE__, __LINE__, __func__); })
#else   //CHECK_PTHREAD_RETURN_VALUE

#define MCHECK(ret) ({ __typeof__ (ret) errnum == (ret);      \
                        assert(errnum == 0), (void) errnum; })

#endif  //CHECK_PTHREAD_RETURN_VALUE

//use as data member of a class.

class MutexLock : public noncopyable
{
	public:
	    MutexLock() : holder_(0)
		{
			MCHECK(pthread_mutex_init(&mutex_ NULL));
		}
		
		~MutexLock()
		{
			assert(holder_ == 0);
			MCHECK(pthread_mutex_destory(&mutex_));
		}
		
		//must be called when locked, i.e. for assertion
		bool isLockedByThisThread() const
		{
			return holder_ == CurrentThread::tid();
		}
		
		void assertLocked() const 
		{
			assert(isLockedByThisThread());
		}
		
		//internal usage.
		void lock()
		{
			MCHECK(pthread_mutex_lock(&mutex_));
			assignHolder();
		}
		
		void unlock()
		{
			unassignHolder();
			MCHECK(pthread_mutex_unlock(&mutex_);
		}
		
		pthread_mutex_t* getPthreadMutex()
		{
			return &mutex_;
		}
		
	private:
	    friend class Condition;
		
		class UnassignGuard : public noncopyable
		{
			public:
			    UnassignGuard(MutexLock& owner) : owner_(owner)
				{
					owner_.unassignHolder();
				}
				
				~UnassignGuard()
				{
					owner_.assignHolder();
					
				}
			private:
			    MutexLock& lock;
		};
		
		void unassignHolder()
		{
			holder_ = 0;
		}
		
		void assignHolder()
		{
			holder_ = CurrentThread::tid();
		}
		
		pthread_mutex_t mutex;
		pid_t holder_;
};

//use as a stack varibale.

class MutexLockGuard : public noncopyable
{
	public:
	    explicit MutexLockGuard(MutexLock& mutex) : mutex_(mutex)
		{
			mutex.lock();
		}
		
		~MutexLockGuard()
		{
			mutex.unlock();
		}
		
	private:
	    MutexLock& mutex_;
};

//prevent misuse like
//MutexLockGuard(Mutex_);
//A tempory object doesn't hold the lock for long !

#define MutexLockGuard(x) error "Missing guard object name"

#endif //BASECOM_MUTEX_H