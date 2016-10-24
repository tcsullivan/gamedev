#include <entities.hpp>

#include <engine.hpp>
#include <player.hpp>
#include <components.hpp>

void entityxTest(void)
{
	entityx::Entity e = game::entities.create();
	e.assign<Position>(100.0f, 100.0f);
	e.assign<Direction>(0.0f, 0.0f);

	e = game::entities.create();
	e.assign<Position>(0.0f, 100.0f);
	e.assign<Direction>(-0.01f, 0.0f);
	e.assign<Physics>(-0.001f);
	e.assign<Visible>(-.2f);
	auto sprite_h = e.assign<Sprite>();
	sprite_h->addSpriteSegment(SpriteData("assets/cat.png",
										  vec2(0, 0)),
										  vec2(0, 0));

	game::engine.getSystem<PlayerSystem>()->setPlayer(e);
}
