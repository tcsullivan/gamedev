#include <particle.hpp>

#include <engine.hpp>
#include <render.hpp>
#include <world.hpp>

ParticleSystem::ParticleSystem(unsigned int max)
	: maximum(max)
{
	parts.reserve(maximum);
}

void ParticleSystem::add(const vec2& pos, const ParticleType& type, const int& timeleft,
	const unsigned char& color)
{
	if (parts.size() < maximum)
		parts.emplace_back(pos, type, timeleft, vec2(color / 8 * 0.25f, color % 8 * 0.125f + 0.0625f));
}

void ParticleSystem::addMultiple(const int& count, const ParticleType& type, std::function<vec2(void)> f,
	const int& timeleft, const unsigned char& color)
{
	int togo = count;
	while (togo-- > 0)
		add(f(), type, timeleft, color);
}

void ParticleSystem::render(void)
{
	static GLuint particleVBO = 9999;
	static const Texture palette ("assets/colorIndex.png");
	// six vertices, 3d coord + 2d tex coord = 5
	constexpr auto entrySize = (6 * 5) * sizeof(GLfloat);

	if (particleVBO == 9999) {
		// generate VBO
		glGenBuffers(1, &particleVBO);
		glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
		glBufferData(GL_ARRAY_BUFFER, maximum * entrySize, nullptr,	GL_STREAM_DRAW);
	}

	// clear dead particles
	parts.erase(std::remove_if(parts.begin(), parts.end(),
		[](const Particle& p) { return p.timeLeft <= 0; }), parts.end());

	// copy data into VBO
	glBindBuffer(GL_ARRAY_BUFFER, particleVBO);

	for (unsigned int i = 0, offset = 0; i < parts.size() - 1; i++, offset += entrySize) {
		const auto& p = parts[i];
		static const auto& hl = game::HLINE;
		GLfloat coords[30] = {
			p.location.x,      p.location.y,      -1, p.color.x, p.color.y,
			p.location.x,      p.location.y + hl, -1, p.color.x, p.color.y,
			p.location.x + hl, p.location.y + hl, -1, p.color.x, p.color.y,
			p.location.x + hl, p.location.y + hl, -1, p.color.x, p.color.y,
			p.location.x + hl, p.location.y,      -1, p.color.x, p.color.y,
			p.location.x,      p.location.y,      -1, p.color.x, p.color.y
		};

		glBufferSubData(GL_ARRAY_BUFFER, offset, entrySize, coords);
	}

	// begin actual rendering
	Render::worldShader.use();
	Render::worldShader.enable();

	// set coords
	glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE,
		5 * sizeof(GLfloat), 0);
	// set tex coords
	glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE,
		5 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

	palette.use();
	glDrawArrays(GL_TRIANGLES, 0, parts.size() * 6);

	Render::worldShader.disable();
	Render::worldShader.unuse();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)en;
	(void)ev;

	auto& worldSystem = *game::engine.getSystem<WorldSystem>();

	for (unsigned int i = 0; i < parts.size(); i++) {
		auto& p = parts[i];

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
			p.timeLeft = 1000;
			break;
		case ParticleType::SmallBlast:
			if (p.velocity.x == 0) {
				int degree = randGet() % 100;
				p.velocity.x = cos(degree) / 4.0f;
				p.velocity.y = sin(degree) / 4.0f;
			} else {
				p.velocity.x += (p.velocity.x > 0) ? -0.001f : 0.001f;
				p.velocity.y += (p.velocity.y > 0) ? -0.001f : 0.001f;
				if ((p.velocity.x > -0.01 && p.velocity.x < 0.01) &&
					(p.velocity.y > -0.01 && p.velocity.y < 0.01)) {
					p.timeLeft = 0;
				}
			}

			break;
		case ParticleType::SmallPoof:
			if (p.velocity.x == 0) {
				p.velocity.y = 0.1f;
				p.velocity.x = randGet() % 10 / 20.0f - 0.25f;
			} else {
				p.velocity.x += (p.velocity.x > 0) ? -0.001f : 0.001f;
				p.velocity.y -= 0.0015f;
			}
			break;
		}

		// really update movement
		p.location.x += p.velocity.x * dt;
		p.location.y += p.velocity.y * dt;

		// world collision
		auto height = worldSystem.isAboveGround(p.location);
		if (height != 0) {
			if (p.type == ParticleType::Drop || p.type == ParticleType::SmallPoof)
				p.location.y = height + 5, p.velocity.y = randGet() % 10 / 40.0f;
			else 
				p.timeLeft = 0;
		}
	}
}

int ParticleSystem::getCount(void) const
{
	return parts.size();
}
