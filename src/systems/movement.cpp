#include <systems/movement.hpp>

#include <components/position.hpp>
#include <components/direction.hpp>
#include <components/solid.hpp>
#include <components/animate.hpp>
#include <components/sprite.hpp>
#include <components/aggro.hpp>
#include <components/dialog.hpp>

#include <thread>

#include <attack.hpp>
#include <events.hpp>
#include <player.hpp>
#include <ui.hpp>

void MovementSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	//bool fight = false;
	entityx::Entity toFight;

	(void)ev;
	en.each<Position, Direction>([&](entityx::Entity entity, Position &position, Direction &direction) {
		position.x += HLINES(direction.x) * dt;
		position.y += HLINES(direction.y) * dt;

		if (entity.has_component<Animate>() && entity.has_component<Sprite>()) {
			auto animate = entity.component<Animate>();
			auto sprite =  entity.component<Sprite>();
		
			if (direction.x)	
				animate->updateAnimation(1, sprite->sprite, dt);
			else
				animate->firstFrame(1, sprite->sprite);
		}
		if (entity.has_component<Dialog>() && entity.component<Dialog>()->talking) {
			direction.x = 0;
		} else {
			if (entity.has_component<Sprite>()) {
				auto& fl = entity.component<Sprite>()->faceLeft;
				if (direction.x != 0)
					fl = (direction.x < 0);
			}

			auto ppos = PlayerSystem::getPosition();
			if (ppos.x > position.x && ppos.x < position.x + entity.component<Solid>()->width) {
				/*if (entity.has_component<Aggro>()) {
					//auto dim = entity.component<Solid>();
					//ev.emit<AttackEvent>(vec2(position.x + dim->width, position.y + dim->height), ATTACKKKKKK, false);
					auto& h = entity.component<Health>()->health;
					if (h > 0) {
						fight = true;
						toFight = entity;
						h = 0;
					}
				} else*/ if (entity.has_component<Trigger>()) {
					static bool triggering = false;
					if (!triggering) {
						triggering = true;
						std::thread([&](entityx::Entity e) {
							UISystem::fadeToggle();
							UISystem::waitForCover();
							UISystem::dialogImportant(e.component<Trigger>()->text);
							UISystem::waitForDialog();
							UISystem::fadeToggle();
							e.destroy();
							triggering = false;
						}, entity).detach();
					}
					return;
				}
			}

			static auto doAttack = [](lua_State* s) -> int {
				vec2 pos (lua_tonumber(s, 1), lua_tonumber(s, 2));
				LuaScript script ("effect = function()\nflash(255,0,0)\ndamage(1)\nend\n\
					hit = function()\nxrange = 5\nend");
				AttackSystem::initLua(script);
				Attack attack = {vec2(), vec2(5, 5), vec2(), vec2(),
					script, TextureIterator()};
				game::events.emit<AttackEvent>(AttackEvent(pos,
					attack, false));
				return 0;
			};

			// make the entity wander
			// TODO initialX and range?
			if (entity.has_component<Wander>()) {
				float aggro = 0;
				LuaList vars = {
					LuaVariable("vely", direction.y),
					LuaVariable("velx", direction.x),
					LuaVariable("playerx", ppos.x),
					LuaVariable("playery", ppos.y),
					LuaVariable("selfx", position.x),
					LuaVariable("selfy", position.y),
					LuaVariable("aggro", aggro)
				};

				bool hasAggro = entity.has_component<Aggro>();
				if (hasAggro)
					aggro = entity.component<Aggro>()->yes ? 1 : 0;
				if (aggro)
					entity.component<Wander>()->script.addFunction("attack", doAttack);
				entity.component<Wander>()->script(aggro ? "hostile" : "update", vars);
				if (hasAggro)
					entity.component<Aggro>()->yes = aggro > 0 ? 1 : 0;
			}
		}
	});

//	if (fight) {
//		UISystem::fadeToggleFast();
//		UISystem::waitForCover();
		//game::engine.getSystem<WorldSystem>()->fight(toFight);
//		UISystem::fadeToggleFast();
//	}
}

