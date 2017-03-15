#include <attack.hpp>
#include <components.hpp>
#include <engine.hpp>
#include <particle.hpp>

constexpr int shortSlashLength = 100;
constexpr int longSlashLength = 200;

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

void AttackSystem::receive(const AttackEvent& ae)
{
	attacks.emplace_front(ae);
}

void AttackSystem::update(entityx::EntityManager& en, entityx::EventManager& ev, entityx::TimeDelta dt)
{
	(void)en;
	(void)ev;
	(void)dt;

	for (const auto& a : attacks) {
		switch (a.type) {
		case AttackType::ShortSlash:
			en.each<Position, Solid, Health>(
				[&a](entityx::Entity e, Position& pos, Solid& dim, Health& h) {
					(void)e;
					if (e.has_component<Player>())
						return;

					if (inrange(a.pos.x, pos.x, pos.x + dim.width, shortSlashLength)) {
						h.health -= a.power;
						game::engine.getSystem<ParticleSystem>()->addMultiple(15, ParticleType::DownSlash,
							[&](){ return vec2(pos.x + dim.width / 2, pos.y + dim.height / 2); }, 300, 7);
					}
				}
			);
			break;
		case AttackType::LongSlash:
			break;
		default:
			break;
		}
	}

	attacks.clear();
}

