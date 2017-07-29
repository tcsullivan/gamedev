#ifndef ATTACK_HPP_
#define ATTACK_HPP_

#include <entityx/entityx.h>

#include <forward_list>
#include <vector>

#include <texture.hpp>
#include <vector2.hpp>

struct Attack {
	int power;
	vec2 offset;
	vec2 range;
	vec2 vel; // TODO use
	vec2 accel; // TODO use

	TextureIterator effect;
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
	struct AttackAnimation {
		vec2 pos;
		TextureIterator effect;
		unsigned int counter;

		AttackAnimation(vec2 p, TextureIterator ti)
			: pos(p), effect(ti), counter(0) {}
	};
	
	std::forward_list<AttackEvent> attacks;
	static std::vector<AttackAnimation> effects;
public:
	void configure(entityx::EventManager& ev) {
		ev.subscribe<AttackEvent>(*this);
	}

	void receive(const AttackEvent& ae);
	void update(entityx::EntityManager& en, entityx::EventManager& ev, entityx::TimeDelta dt) override;
	static void render(void);
};

#endif // ATTACK_HPP_
