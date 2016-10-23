#include <inventory.hpp>

#include <common.hpp>
#include <events.hpp>
#include <texture.hpp>
#include <render.hpp>

constexpr const char* ICON_TEX_FILE_PATH = "config/invIcons.txt";

static std::vector<GLuint> iconTextures;

void InventorySystem::configure(entityx::EventManager &ev)
{
    ev.subscribe<KeyDownEvent>(*this);
}

void InventorySystem::loadIcons(void) {
    iconTextures.clear();
    auto icons = readFileA(ICON_TEX_FILE_PATH);
    for (const auto& s : icons)
        iconTextures.push_back(Texture::loadTexture(s));
}

void InventorySystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
    (void)en;
    (void)ev;
    (void)dt;

    static auto color = Texture::genColor(Color(0, 0, 0));
    vec2 start = vec2(offset.x, 100);// - game::SCREEN_WIDTH / 2 + 20, game::SCREEN_HEIGHT - 40);

    //std::cout << start.x << ' ' << start.y << std::endl;

    Render::textShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, color);
        Render::useShader(&Render::textShader);
        Render::drawRect(start, start + 20, -9.9f);
    Render::textShader.unuse();
}

void InventorySystem::receive(const KeyDownEvent &kde)
{
    (void)kde;
}
