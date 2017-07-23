#include <attack.hpp>

#include <components.hpp>
#include <engine.hpp>
#include <particle.hpp>
#include <player.hpp>
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

void AttackSystem::receive(const AttackEvent& ae)
{
	attacks.emplace_front(ae);
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
			if (!e.has_component<Player>() && inrange(ppos.x, pos.x, pos.x + dim.width) && inrange(ppos.y, pos.y - 2, pos.y + dim.height)) {
				health.health -= hit.damage;
				e.replace<Flash>(Color(255, 0, 0));
				ParticleSystem::addMultiple(15, ParticleType::SmallBlast,
					[&](){ return vec2(pos.x + dim.width / 2, pos.y + dim.height / 2); }, 300, 7);
				die = !hit.pierce;
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
				if (!(e.has_component<Player>() ^ a.fromplayer)) // no self-harm please
					return;

				if (inrange(point.x, pos.x, pos.x + dim.width, HLINES(size.x)) &&
					inrange(point.y, pos.y, pos.y + dim.height, HLINES(size.y))) {
					h.health -= a.attack.power;
					e.replace<Flash>(Color(255, 0, 0));
					ParticleSystem::addMultiple(15, ParticleType::DownSlash,
						[&](){ return vec2(pos.x + dim.width / 2, pos.y + dim.height / 2); }, 300, 7);
				}
			}
		);
	}

	attacks.clear();
}

