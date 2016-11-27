#include <engine.hpp>

#include <config.hpp>
#include <world.hpp>
#include <window.hpp>
#include <ui.hpp>
#include <inventory.hpp>
#include <components.hpp>
#include <player.hpp>

Engine::Engine(void)
    : shouldRun(true), systems(game::entities, game::events)
{
}

void Engine::init(void) {
    game::config::read();
    game::events.subscribe<GameEndEvent>(*this);

    systems.add<WindowSystem>();
    systems.add<RenderSystem>();
	systems.add<InputSystem>();
    systems.add<InventorySystem>();
    systems.add<WorldSystem>();
    systems.add<PlayerSystem>();

	systems.add<PhysicsSystem>();
	systems.add<MovementSystem>();
	systems.add<DialogSystem>();

    systems.configure();

    ui::initSounds();
	game::config::update();
	getSystem<PlayerSystem>()->create();
}

void Engine::render(entityx::TimeDelta dt)
{
    systems.update<RenderSystem>(dt);
	systems.update<WindowSystem>(dt);
    systems.update<InventorySystem>(dt);

	ui::fadeUpdate();
}

void Engine::update(entityx::TimeDelta dt)
{
    systems.update<InputSystem>(dt);
	systems.update<MovementSystem>(dt);
	systems.update<DialogSystem>(dt);
	systems.update<WorldSystem>(dt);
    systems.update<PlayerSystem>(dt);
}


namespace game {
	entityx::EventManager events;
	entityx::EntityManager entities (events);
	SpriteLoader sprite_l;

    Engine engine;
}
