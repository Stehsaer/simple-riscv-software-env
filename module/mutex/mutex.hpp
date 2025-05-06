#pragma once

#ifdef RVISA_A

#include <atomic>

class Mutex
{
	std::atomic<bool> atomic_lock;

  public:

	Mutex();
	~Mutex() = default;

	void unlock();
	void lock();
	bool try_lock();
};

#else
#error "Mutex is not supported without Atomic Extension"
#endif