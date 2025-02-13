#pragma once
#include <thread>
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
		if (!locked.test_and_set(std::memory_order_acquire)) {
			return true;
		}
		return false;
	}

private:
	std::atomic_flag locked;
};
