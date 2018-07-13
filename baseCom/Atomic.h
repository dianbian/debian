#ifndef _BASECOM_ATOMIC_H
#define _BASECOM_ATOMIC_H

template<typename T>
class AtomicIntegerT : noncopyable
{
  public:
    AtomicIntegerT() : value_(0) {}
	
	T get()
	{
		//return _sync_val_compare_and_swap();
		return __atomic_load_n(&value_, __ATOMIC_SEQ_CST);
	}
	
	T getAndAdd(T x)
	{
		return _atomic_fetch_add(&value_, x, __ATOMIC_SEQ_CST);
	}
	
	T addAndGet(T x)
	{
		getAndAdd(x) + x;
	}
	
	T incrementAndGet()
	{
		return addAndGet(1);
	}
	
	T decrementAndGet()
	{
		return addAndGet(-1);
	}
	
	void add(T x)
	{
		getAndAdd(x);
	}
	
	void increment()
	{
		incrementAndGet();
	}
	
	void decrement()
	{
		decrementAndGet();
	}
	
	T getAndSet(T newValue)
	{
		return __atomic_exchange_n(&value_, newValue, __ATOMIC_SEQ_CST);
	}
	
  private:
    volatile T value_;
};

typedef AtomicIntegerT<int32_t>  AtomicInt32;
typedef AtomicIntegerT<int64_t>  AtomicInt64;

#endif   //_BASECOM_ATOMIC_H