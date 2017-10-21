#ifndef SYSTEM_LIGHT_HPP_
#define SYSTEM_LIGHT_HPP_

#include <color.hpp>
#include <render.hpp>
#include <vector2.hpp>

#include <entityx/entityx.h>
#include <vector>

struct Light {
	vec2 pos;
	float radius;
	Color color;

	Light(vec2 p = vec2(), float r = 0, Color c = Color())
		: pos(p), radius(r), color(c) {}
};

class LightSystem : public entityx::System<LightSystem> {
private:
	static std::vector<Light> lights;

public:
	void update(entityx::EntityManager& en, entityx::EventManager& ev, entityx::TimeDelta dt) override;

	static void render(void);

	static int addLight(vec2 pos, float radius, Color color = Color(1, 1, 1));
	static void updateLight(int index, vec2 pos, float radius = -1);
	static void removeLight(int index);

	static inline void clear(void)
	{ lights.clear(); }
};

#endif // SYSTEM_LIGHT_HPP_
