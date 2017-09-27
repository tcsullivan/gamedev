#include <particle.hpp>

#include <engine.hpp>
#include <error.hpp>
#include <render.hpp>
#include <world.hpp>

#include <mutex>

std::vector<Particle> ParticleSystem::parts;
unsigned int          ParticleSystem::maximum;

ParticleSystem::ParticleSystem(unsigned int max)
{
	maximum = max;
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
	static GLuint particleVBO;
	static const Texture palette ("assets/colorIndex.png");
	// six vertices, 3d coord + 2d tex coord = 5
	constexpr auto entrySize = (6 * 5) * sizeof(GLfloat);

	static std::once_flag init;
	std::call_once(init, [&entrySize](GLuint& vbo) {
		// generate VBO
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, maximum * entrySize, nullptr,	GL_DYNAMIC_DRAW);
	}, particleVBO);

	if (parts.empty())
		return;

	// clear dead particles
	parts.erase(std::remove_if(parts.begin(), parts.end(),
		[](const Particle& p) { return p.timeLeft <= 0; }), parts.end());

	// copy data into VBO
	glBindBuffer(GL_ARRAY_BUFFER, particleVBO);

	auto vbobuf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	UserAssert(vbobuf != nullptr, "Failed to map the particle VBO");

	for (unsigned int i = 0, offset = 0; i < parts.size(); i++, offset += entrySize) {
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

		//glBufferSubData(GL_ARRAY_BUFFER, offset, entrySize, coords);
		std::memcpy(reinterpret_cast<void*>(reinterpret_cast<unsigned long long>(vbobuf) + offset), coords, entrySize);
	}

	UserAssert(glUnmapBuffer(GL_ARRAY_BUFFER) == GL_TRUE, "Failed to unmap the particle VBO");

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

	for (unsigned int i = 0; i < parts.size(); i++) {
		auto& p = parts[i];
		auto& vel = p.velocity;

		// update timers
		if (p.timeLeft > 0)
			p.timeLeft -= dt;
		else
			continue;

		// update movement
		switch (p.type) {
		case ParticleType::Drop:
			if (vel.y > -.6)
				vel.y -= 0.001f;
			break;
		case ParticleType::Confetti:
			if (vel.x > -0.01 && vel.x < 0.01) {
				vel.x = randGet() % 12 / 30.0f - 0.2f;
				vel.y = -0.15f;
			} else {
				vel.x += (vel.x > 0) ? -0.002f : 0.002f;
			}
			break;
		case ParticleType::SmallBlast:
			if (vel.x == 0) {
				int degree = randGet() % 100;
				vel.x = cos(degree) / 4.0f;
				vel.y = sin(degree) / 4.0f;
			} else {
				vel.x += (vel.x > 0) ? -0.001f : 0.001f;
				vel.y += (vel.y > 0) ? -0.001f : 0.001f;
				if ((vel.x > -0.01 && vel.x < 0.01) &&
					(vel.y > -0.01 && vel.y < 0.01)) {
					p.timeLeft = 0;
				}
			}
			break;
		case ParticleType::SmallPoof:
			if (vel.x == 0) {
				vel.x = randGet() % 10 / 20.0f - 0.25f;
				vel.y = 0.1f;
			} else {
				vel.x += (vel.x > 0) ? -0.001f : 0.001f;
				vel.y -= 0.0015f;
			}
			break;
		case ParticleType::DownSlash:
			if (vel.x == 0) {
				vel.x = 0.2f * (randGet() % 16 - 8) / 10.0f;
				vel.y = -vel.x;
			}
			break;
		}

		// really update movement
		p.location.x += vel.x * dt;
		p.location.y += vel.y * dt;

		// world collision
		auto height = WorldSystem::isAboveGround(p.location);
		if (height != 0) {
			if (p.type == ParticleType::Drop || p.type == ParticleType::SmallPoof)
				p.location.y = height + 5, p.velocity.y = randGet() % 10 / 40.0f;
			else 
				p.timeLeft = 0;
		}
	}
}

int ParticleSystem::getCount(void) 
{
	return parts.size();
}
