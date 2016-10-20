#ifndef ENGINE_HPP_
#define ENGINE_HPP_

#include <entityx/entityx.h>
#include "entityx/deps/Dependencies.h"

#include <texture.hpp>
#include <components.hpp>
#include <events.hpp>

//game::engine::Systems->add<entityx::deps::Dependency<Visible, Sprite>>();

class Engine : public entityx::Receiver<Engine> {
private:
	bool gameRunning;

public:
    entityx::SystemManager systems;

	explicit Engine(void);

	void init(void);
    void render(entityx::TimeDelta dt);
	void update(entityx::TimeDelta dt);

	template<typename T>
	inline T* getSystem(void) {
		return dynamic_cast<T*>(systems.system<T>().get());
	}

	/*void configure(entityx::EventManager &ev) {
		(void)ev;
	}*/

	inline void receive(const GameEndEvent &gee) {
		gameRunning = !(gee.really);
	}

	inline bool shouldRun(void) const {
		return gameRunning;
	}
};


namespace game {
	extern entityx::EventManager events;
    extern entityx::EntityManager entities;

    extern Engine engine;

    inline void endGame(void) {
        events.emit<GameEndEvent>();
    }
	
	extern SpriteLoader sprite_l;
}



#endif // ENGINE_HPP_
