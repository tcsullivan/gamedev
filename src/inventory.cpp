#include <inventory.hpp>

#include <common.hpp>
#include <events.hpp>
//#include <texture.hpp>
//#include <render.hpp>

constexpr const char* iconTexturePath = "config/invIcons.txt";

static std::vector<Texture> iconTextures;

void InventorySystem::configure(entityx::EventManager &ev)
{
    ev.subscribe<KeyDownEvent>(*this);
}

void InventorySystem::loadIcons(void) {
    iconTextures.clear();
    auto icons = readFileA(iconTexturePath);
    for (const auto& s : icons)
        iconTextures.push_back(s);
}

void InventorySystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
    (void)en;
    (void)ev;
    (void)dt;

	// TODO TODO TODO TODO until we do something
	return;

    //vec2 start = vec2(offset.x, 100);// - game::SCREEN_WIDTH / 2 + 20, game::SCREEN_HEIGHT - 40);

    //std::cout << start.x << ' ' << start.y << std::endl;

    /*Render::textShader.use();
        glActiveTexture(GL_TEXTURE0);
        Colors::black.use();
        Render::useShader(&Render::textShader);
        Render::drawRect(start, start + 20, -9.9f);
    Render::textShader.unuse();*/
}

void InventorySystem::receive(const KeyDownEvent &kde)
{
    (void)kde;
}
