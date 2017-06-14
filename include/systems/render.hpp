#ifndef SYSTEM_RENDER_HPP_
#define SYSTEM_RENDER_HPP_

#include <entityx/entityx.h>

#include <texture.hpp>

#include <string>

class RenderSystem : public entityx::System<RenderSystem> {
private:
	static std::string loadTexString;
	static Texture loadTexResult;

public:
	static Texture loadTexture(const std::string& file);
	static void render(void);

	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;
};


#endif // SYSTEM_RENDER_HPP_
