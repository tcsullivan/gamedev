#include <particle.hpp>

#include <engine.hpp>
#include <render.hpp>
#include <world.hpp>

ParticleSystem::ParticleSystem(int count, bool m)
	: max(m)
{
	parts.reserve(count);
}

void ParticleSystem::add(const vec2& pos, const ParticleType& type)
{
	// TODO enforce max
	//if (max && parts.size() >= std::end(parts))
	//	return;

	parts.emplace_back(pos, type);
}

void ParticleSystem::addMultiple(const int& count, const ParticleType& type, std::function<vec2(void)> f)
{
	int togo = count;
	while (togo-- > 0)
		parts.emplace_back(f(), type);
}

void ParticleSystem::render(void) const
{
	static const GLfloat tex[12] = {
		0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0
	};

	Render::worldShader.use();
	Render::worldShader.enable();
	Colors::blue.use();

	for (const auto& p : parts) {
		GLfloat coord[18] = {
			p.location.x, p.location.y, -1,
			p.location.x, p.location.y + 5, -1,
			p.location.x + 5, p.location.y + 5, -1,
			p.location.x + 5, p.location.y + 5, -1,
			p.location.x + 5, p.location.y, -1,
			p.location.x, p.location.y, -1
		};

		glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 0, coord);
		glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 0, tex);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	Render::worldShader.disable();
	Render::worldShader.unuse();
}

void ParticleSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)en;
	(void)ev;
	(void)dt; // TODO use for time to die

	auto& worldSystem = *game::engine.getSystem<WorldSystem>();

	for (auto part = std::begin(parts); part != std::end(parts); part++) {
		auto& p = *part;

		// update timers
		p.timeLeft -= dt;
		if (p.timeLeft <= 0)
			parts.erase(part);

		// update movement
		switch (p.type) {
		case ParticleType::Drop:
			if (p.velocity.y > -.5)
				p.velocity.y -= 0.001f;
			break;
		case ParticleType::Confetti:
			break;
		}

		// really update movement
		p.location.x += p.velocity.x * dt;
		p.location.y += p.velocity.y * dt;

		// world collision
		auto height = worldSystem.isAboveGround(p.location);
		if (height != 0)
			p.location.y = height + 5, p.velocity.y = randGet() % 10 / 40.0f;
	}
}

int ParticleSystem::getCount(void) const
{
	return parts.size();
}

