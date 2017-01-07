#ifndef PARTICLE_HPP_
#define PARTICLE_HPP_

#include <common.hpp>

#include <list>

#include <entityx/entityx.h>

enum class ParticleType : char {
	Drop,
	Confetti,
	SmallBlast
};

struct Particle {
	vec2 location;
	vec2 velocity;
	ParticleType type;
	int timeLeft;

	Particle(vec2 p, ParticleType t = ParticleType::Drop, int tl = 3000)
		: location(p), type(t), timeLeft(tl) {} // TODO times
} __attribute__ ((packed));

class ParticleSystem : public entityx::System<ParticleSystem> {
private:
	std::vector<Particle> parts;
	bool max;

public:
	ParticleSystem(int count = 2048, bool m = false);

	void add(const vec2& pos, const ParticleType& type, const int& timeleft = 3000);
	void addMultiple(const int& count, const ParticleType& type, std::function<vec2(void)> f, const int& timeleft = 3000);

	void render(void);
	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;

	int getCount(void) const;
};

#endif // PARTICLE_HPP_
