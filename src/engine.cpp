#include <engine.hpp>

#include <config.hpp>
#include <font.hpp>
#include <world.hpp>
#include <window.hpp>
#include <ui.hpp>
#include <ui_menu.hpp>
#include <inventory.hpp>
#include <components.hpp>
#include <player.hpp>
#include <quest.hpp>
#include <particle.hpp>
#include <weather.hpp>
#include <attack.hpp>

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
	systems.add<QuestSystem>();

	systems.add<PhysicsSystem>();
	systems.add<MovementSystem>();
	systems.add<DialogSystem>();

	systems.add<ParticleSystem>();
	systems.add<WeatherSystem>();
	systems.add<AttackSystem>();

	systems.add<UISystem>();
	systems.add<SDLReceiver>();

    systems.configure();

	// init ui

	FontSystem::init(game::config::fontFamily);
	FontSystem::setFontSize(16);
	FontSystem::setFontColor(1, 1, 1);
	FontSystem::setFontZ(-6.0f);

    ui::initSounds();
	ui::menu::init();
	game::config::update();
	PlayerSystem::create();
}

void Engine::update(entityx::TimeDelta dt)
{
    systems.update<InputSystem>(dt);
	systems.update<MovementSystem>(dt);
	//systems.update<DialogSystem>(dt); // doesn't do anything
	systems.update<WorldSystem>(dt);
    systems.update<PlayerSystem>(dt);
	//systems.update<QuestSystem>(dt); // doesn't do anything
	systems.update<WeatherSystem>(dt);
	systems.update<ParticleSystem>(dt);
	systems.update<AttackSystem>(dt);
	systems.update<RenderSystem>(dt);
	//systems.update<UISystem>(dt);
}


namespace game {
	entityx::EventManager events;
	entityx::EntityManager entities (events);
	//SpriteLoader sprite_l;

    Engine engine;
}
