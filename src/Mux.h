#pragma once
#include <atomic>

class Mux {
public:
	void lock() {
		while (!try_lock());
	}
	void unlock() {
		locked.clear(std::memory_order_release);
	}
	bool try_lock() {
		return !locked.test_and_set(std::memory_order_acquire);
	}

private:
	std::atomic_flag locked;
};
