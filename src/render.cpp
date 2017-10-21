#include <render.hpp>

#include <iostream>

#include <config.hpp>
#include <error.hpp>
#include <glm.hpp>
#include <font.hpp>
#include <texture.hpp>

extern vec2 offset;

static Shader *currentShader = nullptr;

void preRender(void);
void render(const int&);

namespace Render {

Shader worldShader;
Shader textShader;

void initShaders(void)
{
    // create the world shader
    worldShader.create("shaders/world.vert", "shaders/world.frag");
    worldShader.addUniform("texture");
    worldShader.addUniform("ortho");
    worldShader.addUniform("tex_color");
    worldShader.addUniform("transform");
    worldShader.addUniform("ambientLight");
    worldShader.addUniform("lightImpact");
    worldShader.addUniform("light");
    worldShader.addUniform("lightColor");
    worldShader.addUniform("lightSize");

    // create the text shader
    textShader.create("shaders/new.vert", "shaders/new.frag");
    textShader.addUniform("sampler");
    textShader.addUniform("ortho"); // actually not used, ortho in new.vert is mislabeled (actually transform)
    textShader.addUniform("tex_color");
    textShader.addUniform("ortho"); // this is transform
}

void useShader(Shader *s)
{
    currentShader = s;
}

void drawRect(vec2 ll, vec2 ur, float z)
{
    GLfloat verts[] = {ll.x, ll.y, z,
                       ur.x, ll.y, z,
                       ur.x, ur.y, z,

                       ur.x, ur.y, z,
                       ll.x, ur.y, z,
                       ll.x, ll.y, z};

    GLfloat tex[] = {0.0, 1.0,
                     1.0, 1.0,
                     1.0, 0.0,

                     1.0, 0.0,
                     0.0, 0.0,
                     0.0, 1.0};

    glUniform1i(currentShader->uniform[WU_texture], 0);
    currentShader->enable();

    glVertexAttribPointer(currentShader->coord, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(currentShader->tex, 2, GL_FLOAT, GL_FALSE, 0, tex);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    currentShader->disable();
}

void init(void)
{
#ifndef __WIN32__
	glewExperimental = GL_TRUE;
#endif

	auto glewError = glewInit();
	UserAssert(glewError == GLEW_OK, std::string("GLEW was not able to initialize! Error: ")
		+ reinterpret_cast<const char *>(glewGetErrorString(glewError)));

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);               // anti-aliasing
	SDL_GL_SetSwapInterval(game::vsync);                       // v-sync
	SDL_ShowCursor(SDL_DISABLE);                               // hide the cursor
	glViewport(0, 0, game::SCREEN_WIDTH, game::SCREEN_HEIGHT); // pixel coordinates
	glEnable(GL_BLEND);                                        // alpha enabling
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);         //
	glClearColor(1, 1, 1, 1);                                  // white clear color

	std::cout << "Initializing shaders!\n";
	initShaders();
	::Colors::init();
}

void render(const int& fps)
{
	preRender();
	::render(fps);
}

} // namespace render

#include <engine.hpp>
#include <gametime.hpp>
#include <inventory.hpp>
#include <particle.hpp>
#include <player.hpp>
#include <ui.hpp>
#include <window.hpp>
#include <world.hpp>

#include <entityx/entityx.h>

void preRender(void)
{
	static const glm::mat4 view = glm::lookAt(
		glm::vec3(0.0f, 0.0f,  0.0f),  // pos
		glm::vec3(0.0f, 0.0f, -10.0f), // looking at
		glm::vec3(0.0f, 1.0f,  0.0f)   // up vector
	);

	static const auto& SCREEN_WIDTH2  = game::SCREEN_WIDTH / 2.0f;
	static const auto& SCREEN_HEIGHT2 = game::SCREEN_HEIGHT / 2.0f;

	//
	// set the ortho
	//

	auto ploc = PlayerSystem::getPosition();
	offset.x = ploc.x + PlayerSystem::getWidth() / 2;

	const auto& worldWidth = WorldSystem::getWidth();
	if (worldWidth < (int)SCREEN_WIDTH2 * 2)
		offset.x = 0;
	else if (offset.x - SCREEN_WIDTH2 < worldWidth * -0.5f)
		offset.x = ((worldWidth * -0.5f) + SCREEN_WIDTH2);
	else if (offset.x + SCREEN_WIDTH2 > worldWidth *  0.5f)
		offset.x = ((worldWidth *  0.5f) - SCREEN_WIDTH2);

	// ortho y snapping (TODO center w/ height)
	offset.y = std::max(ploc.y, SCREEN_HEIGHT2);

	// "setup"
	glm::mat4 projection = glm::ortho(floor(offset.x - SCREEN_WIDTH2),             // left
	                                  floor(offset.x + SCREEN_WIDTH2),             // right
	                                  floor(offset.y - SCREEN_HEIGHT2),            // bottom
	                                  floor(offset.y + SCREEN_HEIGHT2),            // top
	                                  static_cast<decltype(floor(10.0f))>(10.0),   // near
	                                  static_cast<decltype(floor(10.0f))>(-10.0)); // far

	glm::mat4 ortho = projection * view;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    Render::worldShader.use();
		glUniformMatrix4fv(Render::worldShader.uniform[WU_ortho], 1, GL_FALSE, glm::value_ptr(ortho));
		glUniformMatrix4fv(Render::worldShader.uniform[WU_transform], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	Render::worldShader.unuse();

	Render::textShader.use();
		glUniformMatrix4fv(Render::textShader.uniform[WU_transform], 1, GL_FALSE, glm::value_ptr(ortho));
    	glUniform4f(Render::textShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.0);
	Render::textShader.unuse();
}

extern std::size_t getUsedMem(void);
extern int balance;
void render(const int& fps)
{
	preRender();

	WorldSystem::render();
	RenderSystem::render();
	AttackSystem::render();
	ParticleSystem::render();
	UISystem::render();
	InventorySystem::render();
	ui::draw();
	FontSystem::render();

	// draw the debug overlay if desired
	if (ui::debug) {
		auto pos = PlayerSystem::getPosition();
		FontSystem::setFontZ(Render::ZRange::Debug);
		UISystem::putText(vec2(offset.x - game::SCREEN_WIDTH / 2, (offset.y + game::SCREEN_HEIGHT / 2) - FontSystem::getSize()),
		    "loc: %s\noffset: %s\nfps: %d\nticks: %d\npcount: %d\nxml: %s\nmem: %llukb (%d)",
			pos.toString(2).c_str(), offset.toString(2).c_str(), fps,
			game::time::getTickCount(), ParticleSystem::getCount(),
			WorldSystem::getXMLFile().c_str(), getUsedMem() / 1024, balance
			);
		FontSystem::setFontZ();
	}
	
	WindowSystem::render();
}
