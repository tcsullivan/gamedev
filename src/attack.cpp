#include <attack.hpp>

#include <components.hpp>
#include <engine.hpp>
#include <particle.hpp>
#include <player.hpp>
#include <render.hpp>
#include <world.hpp>

// math helpers because we don't trust stdlib
template<typename T>
inline T abs(const T& n) {
	static_assert(std::is_arithmetic<T>::value, "abs expects numbers");
	return n >= 0 ? n : -n;
}

bool inrange(float point, float left, float right, float range)
{
	return (left < point + range && left > point - range) ||
		(right < point + range && right > point - range) ||
		(point > left && point < right);
}

bool inrange(float point, float left, float right)
{
	return point > left && point < right;
}

std::vector<AttackSystem::AttackAnimation> AttackSystem::effects;

bool AttackSystem::receive(const AttackEvent& ae)
{
	attacks.emplace_front(ae);
	return true;
}

namespace lua {
	static entityx::Entity* entity;

	inline void setEntity(entityx::Entity* e) {
		entity = e;
	}

	int flash(lua_State* state) {
		float r = lua_tonumber(state, 1);
		float g = lua_tonumber(state, 2);
		float b = lua_tonumber(state, 3);
		entity->replace<Flash>(Color(r, g, b));
		return 0;
	}

	int damage(lua_State* state) {
		float d = lua_tonumber(state, 1);
		entity->component<Health>()->health -= d;
		return 0;
	}
}

void AttackSystem::initLua(LuaScript& s)
{
	s.addFunction("flash", lua::flash);
	s.addFunction("damage", lua::damage); 
}

void AttackSystem::update(entityx::EntityManager& en, entityx::EventManager& ev, entityx::TimeDelta dt)
{
	(void)en;
	(void)ev;
	(void)dt;

	// handle painful entities (e.g. arrow)
	en.each<Hit, Position>([&](entityx::Entity p, Hit& hit, Position& ppos) {
		bool die = false;
		en.each<Health, Position, Solid>([&](entityx::Entity e, Health& health, Position& pos, Solid& dim) {
			(void)health;
			if (!e.has_component<Player>() && inrange(ppos.x, pos.x, pos.x + dim.width) && inrange(ppos.y, pos.y - 2, pos.y + dim.height)) {
				lua::setEntity(&e);
				hit.attack->script();
				if (hit.effect.size() > 0)
					effects.emplace_back(vec2(ppos.x, ppos.y), hit.effect);
				//ParticleSystem::addMultiple(15, ParticleType::SmallBlast,
				//	[&](){ return vec2(pos.x + dim.width / 2, pos.y + dim.height / 2); }, 300, 7);
				die = true;
			} else if (WorldSystem::isAboveGround(vec2(ppos.x, ppos.y - 5)))
				die = true;
		});

		if (die)
			p.destroy();
	}); 

	// handle emitted attacks (player's)
	for (const auto& a : attacks) {
		vec2 point = a.pos + a.attack.offset;
		vec2 size = a.attack.range;
		point.y -= size.y / 2; // center range height

		en.each<Position, Solid, Health>(
			[&](entityx::Entity e, Position& pos, Solid& dim, Health& h) {
				(void)h;
				if (!(e.has_component<Player>() ^ a.fromplayer)) // no self-harm please
					return;

				if (inrange(point.x, pos.x, pos.x + dim.width, HLINES(size.x)) &&
					inrange(point.y, pos.y, pos.y + dim.height, HLINES(size.y))) {
					lua::setEntity(&e);
					a.attack.script();
					if (a.attack.effect.size() > 0)
						effects.emplace_back(point, a.attack.effect);
					//ParticleSystem::addMultiple(15, ParticleType::DownSlash,
					//	[&](){ return vec2(pos.x + dim.width / 2, pos.y + dim.height / 2); }, 300, 7);
				}
			}
		);
	}

	attacks.clear();
}

#define RATE 3
void AttackSystem::render(void)
{
	float z = -9.9f;
	Render::worldShader.use();
	Render::worldShader.enable();
	for (auto& ae : effects) {
		ae.effect(ae.counter / RATE); // bind current frame
		auto dim = ae.effect.getTextureDim();
		GLfloat verts[] = {
			ae.pos.x,         ae.pos.y,         z, 0, 0,
			ae.pos.x + dim.x, ae.pos.y,         z, 1, 0,
			ae.pos.x + dim.x, ae.pos.y + dim.y, z, 1, 1,
			ae.pos.x + dim.x, ae.pos.y + dim.y, z, 1, 1,
			ae.pos.x,         ae.pos.y + dim.y, z, 0, 1,
			ae.pos.x,         ae.pos.y,         z, 0, 0
		};
		glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), verts);
		glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), verts + 3);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	Render::worldShader.disable();
	Render::worldShader.unuse();

	effects.erase(std::remove_if(effects.begin(), effects.end(), [](auto& e) { return ++e.counter >= e.effect.size() * RATE; }),
		effects.end());
}
