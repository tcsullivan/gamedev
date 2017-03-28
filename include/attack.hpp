#ifndef FIGHT_HPP_
#define FIGHT_HPP_

#include <entityx/entityx.h>

#include <forward_list>

#include <vector2.hpp>

enum class AttackType : char {
	ShortSlash,
	LongSlash,
};

struct AttackEvent {
	AttackEvent(vec2 p, AttackType at, bool fp, int pow = 10)
		: pos(p), type(at), fromplayer(fp), power(pow) {}

	vec2 pos;
	AttackType type;
	bool fromplayer;
	int power;
};

class AttackSystem : public entityx::System<AttackSystem>, public entityx::Receiver<AttackSystem> {
private:
	std::forward_list<AttackEvent> attacks;

public:
	explicit AttackSystem() = default;

	void configure(entityx::EventManager& ev) {
		ev.subscribe<AttackEvent>(*this);
	}

	void receive(const AttackEvent& ae);
	void update(entityx::EntityManager& en, entityx::EventManager& ev, entityx::TimeDelta dt) override;
};

#endif // FIGHT_HPP_
