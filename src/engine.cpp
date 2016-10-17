#include <engine.hpp>

#include <config.hpp>
#include <world.hpp>
#include <ui.hpp>
#include <inventory.hpp>
#include <entities.hpp>
#include <window.hpp>

extern World *currentWorld;

Engine::Engine(void)
    : gameRunning(true), systems(game::entities, game::events)
{
}

void Engine::init(void) {
    game::config::read();
    game::events.subscribe<GameEndEvent>(*this);

    systems.add<WindowSystem>();
    systems.add<InputSystem>();
    systems.add<InventorySystem>();
    systems.add<WorldSystem>();
    systems.add<PlayerSystem>(&player);

    systems.configure();

	game::config::update();
}

void Engine::render(entityx::TimeDelta dt)
{
    systems.update<WindowSystem>(dt);
}

void Engine::update(entityx::TimeDelta dt)
{
    systems.update<InputSystem>(dt);
    systems.update<InventorySystem>(dt);
    systems.update<PlayerSystem>(dt);
	systems.update<WorldSystem>(dt);
}


namespace game {
	entityx::EventManager events;
	entityx::EntityManager entities (events);

    Engine engine;
}
