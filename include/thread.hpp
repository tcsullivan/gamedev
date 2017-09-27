#ifndef THREAD_HPP_
#define THREAD_HPP_

#include <thread>
#include <atomic>
#include <entityx/entityx.h>

class GameThread : public entityx::Receiver<GameThread> {
private:
	static std::atomic_bool pause;

	std::atomic_bool die;
	std::thread thread;

public:
	GameThread(std::function<void(void)> func) {
		die.store(false);
		pause.store(false);
		thread = std::thread([&](std::function<void(void)> f) {
			while (!die.load()) {
				if (!pause.load())
					f();
				else
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}, func);
	}

	~GameThread(void) {
		thread.join();
	}

	inline void stop(void) {
		die.store(true);
	}

	static inline void pauseAll(void) {
		pause.store(true);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	static inline void resumeAll(void)
	{ pause.store(false); }

	static inline bool isPaused(void)
	{ return pause.load(); }
};

#endif // THREAD_HPP_
