#include <attack.hpp>

#include <components.hpp>
#include <engine.hpp>
#include <particle.hpp>
#include <player.hpp>
#include <world.hpp>

constexpr int shortSlashLength = 20;
constexpr int longSlashLength = 40;

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

	// handle attacking entities
	en.each<Hit, Position>([&](entityx::Entity p, Hit& hit, Position& ppos) {
		bool die = false;
		en.each<Health, Position, Solid>([&](entityx::Entity e, Health& health, Position& pos, Solid& dim) {
			if (!e.has_component<Player>() && inrange(ppos.x, pos.x, pos.x + dim.width) && inrange(ppos.y, pos.y - 2, pos.y + dim.height)) {
				health.health -= hit.damage;
				ParticleSystem::addMultiple(15, ParticleType::SmallBlast,
					[&](){ return vec2(pos.x + dim.width / 2, pos.y + dim.height / 2); }, 300, 7);
				die = !hit.pierce;
			} else if (WorldSystem::isAboveGround(vec2(ppos.x, ppos.y - 5)))
				die = true;
		});

		if (die)
			p.destroy();
	}); 

	// handle emitted attacks
	for (const auto& a : attacks) {
		switch (a.type) {
		case AttackType::ShortSlash:
		case AttackType::LongSlash:
			en.each<Position, Solid, Health>(
				[&a](entityx::Entity e, Position& pos, Solid& dim, Health& h) {
					if (!(e.has_component<Player>() ^ a.fromplayer))
						return;

					if (inrange(a.pos.x, pos.x, pos.x + dim.width, HLINES(shortSlashLength)) &&
						inrange(a.pos.y, pos.y, pos.y + dim.height)) {
						h.health -= a.power;
						ParticleSystem::addMultiple(15, ParticleType::DownSlash,
							[&](){ return vec2(pos.x + dim.width / 2, pos.y + dim.height / 2); }, 300, 7);
					}
				}
			);
			break;
		default:
			break;
		}
	}

	attacks.clear();
}

