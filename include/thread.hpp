#ifndef THREAD_HPP_
#define THREAD_HPP_

#ifndef __WIN32__
#include <thread>
#else
#include <win32thread.hpp>
#endif // __WIN32__

#include <atomic>
#include <entityx/entityx.h>

class GameThread : public entityx::Receiver<GameThread> {
private:
	std::atomic_bool die;
	std::thread thread;

public:
	GameThread(std::function<void(void)> func) {
		die.store(false);
		thread = std::thread([&](std::function<void(void)> f) {
			while (!die.load())
				f();
		}, func);
	}

	~GameThread(void) {
		thread.join();
	}

	inline void stop(void) {
		die.store(true);
	}
};

#endif // THREAD_HPP_