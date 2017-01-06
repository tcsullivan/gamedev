#ifndef PARTICLE_HPP_
#define PARTICLE_HPP_

#include <common.hpp>

#include <list>

#include <entityx/entityx.h>

enum class ParticleType : char {
	Drop,
	Confetti
};

struct Particle {
	vec2 location;
	vec2 velocity;
	ParticleType type;
	int timeLeft;

	Particle(vec2 p, ParticleType t = ParticleType::Drop)
		: location(p), type(t), timeLeft(3000) {} // TODO times
} __attribute__ ((packed));

class ParticleSystem : public entityx::System<ParticleSystem> {
private:
	std::vector<Particle> parts;
	bool max;

public:
	ParticleSystem(int count = 1024, bool m = false);

	void add(const vec2& pos, const ParticleType& type);
	void addMultiple(const int& count, const ParticleType& type, std::function<vec2(void)> f);

	void render(void) const; 
	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;

	int getCount(void) const;
};

#endif // PARTICLE_HPP_
