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
					vec2 eloc (pos.x + dim.width / 2, pos.y + dim.height / 2);
					if (abs(eloc.x - a.pos.x) <= shortSlashLength) {
						h.health -= a.power;
						game::engine.getSystem<ParticleSystem>()->addMultiple(10, ParticleType::SmallBlast,
							[&](){ return eloc; }, 500, 7);
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

