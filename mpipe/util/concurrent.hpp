#pragma once

#include <atomic>

class spin_lock
{
private:

	std::atomic_flag locked_ = ATOMIC_FLAG_INIT;

public:

	bool try_lock()
	{
		return !locked_.test_and_set(std::memory_order_acquire);
	}

	void lock()
	{
		while (locked_.test_and_set(std::memory_order_acquire)) { ; }
	}

	void unlock()
	{
		locked_.clear(std::memory_order_release);
	}
};
