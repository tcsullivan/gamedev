/**
 * @file engine.hpp
 * @brief The main game engine, and functions to assist it.
 */

#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include <atomic>
#include <chrono>
#include <thread>

#include <entityx/entityx.h>

#include <components.hpp>
#include <events.hpp>

//game::engine::Systems->add<entityx::deps::Dependency<Visible, Sprite>>();

/**
 * @class Engine
 * The main game engine class. Only one instance of this should be created, it
 * handles everything game-related.
 */
class Engine : public entityx::Receiver<Engine> {
public:
	/**
	 * A flag to indicate if a thread should continue to run.
	 */
	bool shouldRun;

	/**
	 * Handles game systems.
	 */
    entityx::SystemManager systems;

	explicit Engine(void);

	/**
	 * Initializes the game engine, and all systems used within it.
	 */
	void init(void);

	/**
	 * Updates all logic systems.
	 * @param dt the delta time
	 */
	void update(entityx::TimeDelta dt);

	/**
	 * A shortcut to get a system, for calling system-specific functions.
	 * Takes the type of the desired system.
	 */
	template<typename T>
	inline T* getSystem(void) {
		return dynamic_cast<T*>(systems.system<T>().get());
	}

	/**
	 * A handler for the game ending event.
	 * @param gee game end event data
	 */
	inline void receive(const GameEndEvent &gee) {
		shouldRun = !(gee.really);
	}
};

class LockableEntityManager : public entityx::EntityManager {
private:
	std::atomic_bool locked;
	
public:
	LockableEntityManager(entityx::EventManager& ev)
		: EntityManager(ev) {
		locked.store(false);
	}
	
	void lock(void) {
		while (locked.load())
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		
		locked.store(true);
	}
	
	void unlock(void) {
		locked.store(false);
	}
	
	bool try_lock(void) {
		if (locked.load())
			return false;
		
		locked.store(true);
		return true;
	}
};

namespace game {
	/**
	 * Handles all game events.
	 */
	extern entityx::EventManager events;

	/**
	 * Handles entity data.
	 */
    extern LockableEntityManager entities;
	
	/**
	 * An instance of the main game engine.
	 */
    extern Engine engine;

	/**
	 * Ends the game.
	 */
    inline void endGame(void) {
        events.emit<GameEndEvent>();
    }
}



#endif // ENGINE_HPP_
