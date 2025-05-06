#ifdef RVISA_A

#include "mutex.hpp"

Mutex::Mutex()
{
	atomic_lock.store(false, std::memory_order_release);
}

void Mutex::unlock()
{
	atomic_lock.store(false, std::memory_order_release);
}

void Mutex::lock()
{
	while (!try_lock());
}

bool Mutex::try_lock()
{
	bool temp = false;
	bool acquired
		= atomic_lock
			  .compare_exchange_strong(temp, true, std::memory_order_acquire, std::memory_order_relaxed);
	return acquired;
}

#endif