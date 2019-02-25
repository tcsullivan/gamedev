#ifndef SYSTEM_MOVEMENT_HPP_
#define SYSTEM_MOVEMENT_HPP_

#include <entityx/entityx.h>
#include <attack.hpp>

class MovementSystem : public entityx::System<MovementSystem> {
private:
	constexpr static const char *hitPlayerScript = "\
		effect = function()\n \
			flash(255, 0, 0)\n \
			damage(1)\n \
		end\n \
		hit = function()\n \
			xrange = 5\n \
		end";
	static LuaScript hitPlayer;
	static Attack playerAttack;

public:
	MovementSystem(void) {
		hitPlayer = LuaScript(hitPlayerScript);
		AttackSystem::initLua(hitPlayer);
		playerAttack = { vec2(), vec2(5, 5), vec2(), vec2(),
			hitPlayer, TextureIterator() };
	}

	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;

	static int doAttack(lua_State *);
};

#endif // SYSTEM_MOVEMENT_HPP_
