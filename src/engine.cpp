#include <engine.hpp>

#include <config.hpp>
#include <world.hpp>
#include <ui.hpp>
//#include <inventory.hpp>
#include <window.hpp>
#include <components.hpp>
#include <player.hpp>

extern World *currentWorld;

Engine::Engine(void)
    : gameRunning(true), systems(game::entities, game::events)
{
}

void Engine::init(void) {
    game::config::read();
    game::events.subscribe<GameEndEvent>(*this);

    systems.add<WindowSystem>();
    systems.add<RenderSystem>();
	systems.add<InputSystem>();
//    systems.add<InventorySystem>();
    systems.add<WorldSystem>();
    systems.add<PlayerSystem>();
	systems.add<MovementSystem>();
//    systems.add<PlayerSystem>(&player);

    systems.configure();

	game::config::update();
}

void Engine::render(entityx::TimeDelta dt)
{
    systems.update<RenderSystem>(dt);
	systems.update<WindowSystem>(dt);

}

void Engine::update(entityx::TimeDelta dt)
{
    systems.update<InputSystem>(dt);
//    systems.update<InventorySystem>(dt);
//    systems.update<PlayerSystem>(dt);
	systems.update<MovementSystem>(dt);
	systems.update<WorldSystem>(dt);
    systems.update<PlayerSystem>(dt);
}


namespace game {
	entityx::EventManager events;
	entityx::EntityManager entities (events);
	SpriteLoader sprite_l;

    Engine engine;
}
