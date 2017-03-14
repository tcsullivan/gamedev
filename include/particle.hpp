#ifndef PARTICLE_HPP_
#define PARTICLE_HPP_

#include <texture.hpp>
#include <vector2.hpp>

#include <vector>

#include <entityx/entityx.h>

enum class ParticleType : char {
	Drop,
	Confetti,
	SmallBlast,
	SmallPoof,
	DownSlash
};

struct Particle {
	int timeLeft;
	ParticleType type;
	vec2 velocity;
	vec2 location;
	vec2 color; // assets/colorIndex.png

	Particle(vec2 p, ParticleType t, int tl, vec2 c)
		: timeLeft(tl), type(t), location(p), color(c) {}
};

class ParticleSystem : public entityx::System<ParticleSystem> {
private:
	std::vector<Particle> parts;
	unsigned int maximum;

public:
	ParticleSystem(unsigned int max = 2048);

	void add(const vec2& pos, const ParticleType& type, const int& timeleft = 3000,
		const unsigned char& color = 0);
	void addMultiple(const int& count, const ParticleType& type, std::function<vec2(void)> f,
		const int& timeleft = 3000, const unsigned char& color = 0);

	void render(void);
	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;

	int getCount(void) const;
};

#endif // PARTICLE_HPP_
