#ifndef ATTACK_HPP_
#define ATTACK_HPP_

#include <entityx/entityx.h>

#include <forward_list>

#include <vector2.hpp>

struct Attack {
	int power;
	vec2 offset;
	vec2 range;
	vec2 vel; // TODO use
	vec2 accel; // TODO use
};

struct AttackEvent {
	AttackEvent(vec2 p, Attack at, bool fp)
		: pos(p), attack(at), fromplayer(fp) {}

	vec2 pos;
	Attack attack;

	bool fromplayer;
};

class AttackSystem : public entityx::System<AttackSystem>, public entityx::Receiver<AttackSystem> {
private:
	std::forward_list<AttackEvent> attacks;

public:
	void configure(entityx::EventManager& ev) {
		ev.subscribe<AttackEvent>(*this);
	}

	void receive(const AttackEvent& ae);
	void update(entityx::EntityManager& en, entityx::EventManager& ev, entityx::TimeDelta dt) override;
};

#endif // ATTACK_HPP_
