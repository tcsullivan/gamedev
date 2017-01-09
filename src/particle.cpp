#include <particle.hpp>

#include <engine.hpp>
#include <render.hpp>
#include <world.hpp>

ParticleSystem::ParticleSystem(int count, bool m)
	: max(m)
{
	parts.reserve(count);
}

void ParticleSystem::add(const vec2& pos, const ParticleType& type, const int& timeleft, const unsigned char& color)
{
	// TODO not enforce max
	if (/*max &&*/ parts.size() + 1 >= parts.capacity())
		return;

	parts.emplace_back(pos, type, timeleft, vec2(color / 8 * 0.25f, color % 8 * 0.125f + 0.0625f));
}

void ParticleSystem::addMultiple(const int& count, const ParticleType& type, std::function<vec2(void)> f,
	const int& timeleft, const unsigned char& color)
{
	int togo = count;
	while (togo-- > 0)
		parts.emplace_back(f(), type, timeleft, vec2(color / 8 * 0.25f, color % 8 * 0.125f + 0.0625f));
}

void ParticleSystem::render(void)
{
	static GLuint particleVBO = 9999;
	// six vertices, 3d coord + 2d tex coord = 5
	constexpr auto entrySize = (6 * 5) * sizeof(GLfloat);
	static const Texture palette ("assets/colorIndex.png");

	if (particleVBO == 9999) {
		// generate VBO
		glGenBuffers(1, &particleVBO);
		glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
		glBufferData(GL_ARRAY_BUFFER, parts.capacity() * entrySize, nullptr,
			GL_STREAM_DRAW);
	}

	// clear dead particles
	parts.erase(std::remove_if(parts.begin(), parts.end(),
		[](const Particle& p) { return p.timeLeft <= 0; }), parts.end());

	// copy data into VBO
	glBindBuffer(GL_ARRAY_BUFFER, particleVBO);

	int offset = 0;
	for (const auto& p : parts) {
		static const auto hl = game::HLINE;
		GLfloat coords[30] = {
			p.location.x,      p.location.y,      -1, p.color.x, p.color.y,
			p.location.x,      p.location.y + hl, -1, p.color.x, p.color.y,
			p.location.x + hl, p.location.y + hl, -1, p.color.x, p.color.y,
			p.location.x + hl, p.location.y + hl, -1, p.color.x, p.color.y,
			p.location.x + hl, p.location.y,      -1, p.color.x, p.color.y,
			p.location.x,      p.location.y,      -1, p.color.x, p.color.y
		};

		glBufferSubData(GL_ARRAY_BUFFER, offset, entrySize, coords);
		offset += entrySize;
	}

	// begin actual rendering
	Render::worldShader.use();
	Render::worldShader.enable();

	// set coords
	glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
	glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE,
		5 * sizeof(GLfloat), 0);
	// set tex coords
	glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE,
		5 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

	palette.use();
	glDrawArrays(GL_TRIANGLES, 0, parts.size() * 6);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
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

		// update movement
		switch (p.type) {
		case ParticleType::Drop:
			if (p.velocity.y > -.6)
				p.velocity.y -= 0.001f;
			break;
		case ParticleType::Confetti:
			if (p.velocity.x > -0.01 && p.velocity.x < 0.01) {
				p.velocity.x = randGet() % 12 / 30.0f - 0.2f;
			} else {
				p.velocity.x += (p.velocity.x > 0) ? -0.002f : 0.002f;
			}
			p.velocity.y = -0.15f;
			break;
		case ParticleType::SmallBlast:
			if (p.velocity.x == 0) {
				int degree = randGet() % 100;
				p.velocity.x = cos(degree) / 4.0f;
				p.velocity.y = sin(degree) / 4.0f;
			} else {
				p.velocity.x += (p.velocity.x > 0) ? -0.001f : 0.001f;
				p.velocity.x += (p.velocity.y > 0) ? -0.001f : 0.001f;
				if ((p.velocity.x > -0.01 && p.velocity.x < 0.01) &&
					(p.velocity.y > -0.01 && p.velocity.y < 0.01)) {
					p.timeLeft = 0;
				}
			}

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
