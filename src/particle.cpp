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
	// TODO not enforce max
	if (/*max &&*/ parts.size() + 1 >= parts.capacity())
		return;

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
	static GLuint particleVBO = 9999, texVBO = 0;

	if (particleVBO == 9999) {
		glGenBuffers(1, &particleVBO);
		glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
		glBufferData(GL_ARRAY_BUFFER, parts.capacity() * 18 * sizeof(GLfloat), nullptr,
			GL_STREAM_DRAW);

		glGenBuffers(1, &texVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, parts.capacity() * 12 * sizeof(GLfloat), nullptr,
			GL_STATIC_DRAW);
		int top = (parts.capacity() - 1) * 12 * sizeof(GLfloat), i = 0;
		unsigned char zero = 0;
		while (i < top) {
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, i, 1, &zero);
			i++;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, particleVBO);

	// copy data into VBO
	int offset = 0;
	for (const auto& p : parts) {
		GLfloat coords[18] = {
			p.location.x, p.location.y, -1,
			p.location.x, p.location.y + 5, -1,
			p.location.x + 5, p.location.y + 5, -1,
			p.location.x + 5, p.location.y + 5, -1,
			p.location.x + 5, p.location.y, -1,
			p.location.x, p.location.y, -1
		};
		
		glBufferSubData(GL_ARRAY_BUFFER, offset, 18 * sizeof(GLfloat), coords);
		offset += 18 * sizeof(GLfloat);
	}
	
	Render::worldShader.use();
	Render::worldShader.enable();
	Colors::blue.use();
	//glUniform4f(Render::worldShader.uniform[WU_tex_color], 0.0f, 0.0f, 1.0f, 1.0f);

	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
	glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texVBO);
	glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, parts.size() * 6);

	glDisableClientState(GL_VERTEX_ARRAY);

	Render::worldShader.disable();
	Render::worldShader.unuse();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
			if (p.velocity.y > -.6)
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

