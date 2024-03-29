#include <systems/light.hpp>

#include <components/light.hpp>
#include <components/position.hpp>
#include <components/solid.hpp>

std::vector<Light> LightSystem::lights;

GLfloat *LightSystem::colorData = nullptr;
GLfloat *LightSystem::coordData = nullptr;

void LightSystem::resizeLights(void)
{
	delete colorData;
	delete coordData;
	colorData = new GLfloat[lights.size() * 4];
	coordData = new GLfloat[lights.size() * 4];
}

void LightSystem::update(entityx::EntityManager& en, entityx::EventManager& ev, entityx::TimeDelta dt) {
	(void)ev;
	(void)dt;

	en.each<Illuminate, Position, Solid>([&](entityx::Entity e, Illuminate& ill, Position& pos,
		Solid& dim) {
		(void)e;
		vec2 p (pos.x, pos.y);
		vec2 d (dim.width, dim.height);
		LightSystem::updateLight(ill.index, p + d / 2); 
	});
}

void LightSystem::render(void) {
	unsigned int offset = 0;
	for (const auto& l : lights) {
		coordData[offset] = l.pos.x, coordData[offset + 1] = l.pos.y,
			coordData[offset + 2] = -5, coordData[offset + 3] = l.radius;
		colorData[offset] = l.color.red, colorData[offset + 1] = l.color.green,
			colorData[offset + 2] = l.color.blue, colorData[offset + 3] = 1.0f;
		offset += 4;
	}

	Render::worldShader.use();
	Render::worldShader.enable();

	glUniform4fv(Render::worldShader.uniform[WU_light], lights.size(), coordData);
	glUniform4fv(Render::worldShader.uniform[WU_light_color], lights.size(), colorData);
	glUniform1i(Render::worldShader.uniform[WU_light_size], lights.size());

	Render::worldShader.disable();
	Render::worldShader.unuse();
}

int LightSystem::addLight(vec2 pos, float radius, Color color) {
	lights.emplace_back(pos, radius, color);
	resizeLights();
	return lights.size() - 1;
}

void LightSystem::updateLight(int index, vec2 pos, float radius) {
	lights[index].pos = pos;
	if (radius >= 0)
		lights[index].radius = radius;
}

void LightSystem::removeLight(int index) {
	lights.erase(lights.begin() + index);
	resizeLights();
}

