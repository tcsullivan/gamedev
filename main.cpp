/* ----------------------------------------------------------------------------
** The main file, home of the main loop.
** --------------------------------------------------------------------------*/
// ...
/* ----------------------------------------------------------------------------
** Includes section
** --------------------------------------------------------------------------*/

#include <brice.hpp>

#include <entityx/entityx.h>

#include <window.hpp>
#include <render.hpp>
#include <engine.hpp>

// local library includes
#include <tinyxml2.h>
using namespace tinyxml2;

// local game includes
#include <common.hpp>
#include <config.hpp>
#include <world.hpp>
#include <ui.hpp>
#include <gametime.hpp>
#include <player.hpp>

#include <fstream>
#include <mutex>
#include <chrono>

using namespace std::literals::chrono_literals;

/* ----------------------------------------------------------------------------
** Variables section
** --------------------------------------------------------------------------*/

// the currently used folder to grab XML files
std::string xmlFolder;

// the current menu
Menu *currentMenu;

// keeps a simple palette of colors for single-color draws
GLuint colorIndex;

// the mouse's texture
GLuint mouseTex;

// the center of the screen
vec2 offset;

/*
 * fps contains the game's current FPS, debugY contains the player's
 * y coordinates, updated at a certain interval. These are used in
 * the debug menu (see below).
 */

static unsigned int fps=0;
//static float debugY=0;

// handles all logic operations
void logic(void);

// handles all rendering operations
void render(void);

/*******************************************************************************
** MAIN ************************************************************************
********************************************************************************/

int main(int argc, char *argv[])
{
	static bool worldReset = false, worldDontReallyRun = false;
	std::string worldActuallyUseThisXMLFile;

	// handle command line arguments
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			std::string s = argv[i];

			if (s == "--reset" || s == "-r")
				worldReset = true;
			else if (s == "--dontrun" || s == "-d")
				worldDontReallyRun = true;
			else if (s == "--xml" || s == "-x")
				worldActuallyUseThisXMLFile = argv[i + 1];
		}
	}

	game::engine.init();

	// initialize GLEW
#ifndef __WIN32__
	glewExperimental = GL_TRUE;
#endif

	GLenum err;
	if ((err = glewInit()) != GLEW_OK)
		UserError(std::string("GLEW was not able to initialize! Error: ") + reinterpret_cast<const char *>(glewGetErrorString(err)));

	// start the random number generator
	randInit(millis());

	// 'basic' OpenGL setup
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetSwapInterval(1); // v-sync
	SDL_ShowCursor(SDL_DISABLE); // hide the mouse
	glViewport(0, 0, game::SCREEN_WIDTH, game::SCREEN_HEIGHT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(1,1,1,1);

	// TODO
	Texture::initColorIndex();

	// initialize shaders
	std::cout << "Initializing shaders!\n";

	// create shaders
	Render::initShaders();

	// load up some fresh hot brice
	game::briceLoad();
	game::briceUpdate();

	// load sprites used in the inventory menu. See src/inventory.cpp
	//initInventorySprites();

	// load mouse texture, and other inventory textures
	mouseTex = Texture::loadTexture("assets/mouse.png");

	// get a world
	if (xmlFolder.empty())
		xmlFolder = "xml/";

	// read in all XML file names in the folder
	std::vector<std::string> xmlFiles;
	if (getdir(std::string("./" + xmlFolder).c_str(), xmlFiles))
		UserError("Error reading XML files!!!");

	// alphabetically sort files
	strVectorSortAlpha(&xmlFiles);

	if (worldReset) {
		for (const auto &xf : xmlFiles) {
			if (xf[0] != '.') {
				XMLDocument xmld;
				auto file = xmlFolder + xf;
				xmld.LoadFile(file.c_str());

				auto xmle = xmld.FirstChildElement("World");

				if (xmle == nullptr) {
					xmle = xmld.FirstChildElement("IndoorWorld");

					if (xmle == nullptr)
						continue;
				}

				xmle = xmle->FirstChildElement();
				while (xmle) {
					xmle->DeleteAttribute("x");
					xmle->DeleteAttribute("y");
					xmle->DeleteAttribute("health");
					xmle->DeleteAttribute("alive");
					xmle->DeleteAttribute("dindex");
					xmle = xmle->NextSiblingElement();
				}

				xmld.SaveFile(file.c_str(), false);
			}
		}

		game::briceClear();

		std::ofstream pdat ("xml/.main.dat", std::ios::out);
		pdat.close();
	}

	if (!worldActuallyUseThisXMLFile.empty()) {
		game::engine.getSystem<WorldSystem>()->load(worldActuallyUseThisXMLFile);
	} else {
		// load the first valid XML file for the world
		for (const auto &xf : xmlFiles) {
			if (xf[0] != '.') {
				// read it in
				std::cout << "File to load: " << xf << '\n';
				game::engine.getSystem<WorldSystem>()->load(xf);
				break;
			}
		}
	}

	ui::menu::init();



	if (!worldDontReallyRun) {
		// the main loop, in all of its gloriousness..
		std::thread thMain ([&] {
			const bool &run = game::engine.shouldRun;
			while (run) {
				game::time::mainLoopHandler();

				if (game::time::tickHasPassed())
					logic();

				game::engine.update(game::time::getDeltaTime());

				std::this_thread::sleep_for(1ms);
			}
		});

		// the debug loop, gets debug screen values
		std::thread thDebug ([&] {
			const bool &run = game::engine.shouldRun;
			while (run) {
				fps = 1000 / game::time::getDeltaTime();
//				debugY = player->loc.y;

				std::this_thread::sleep_for(1s);
			}
		});

		const bool &run = game::engine.shouldRun;
		while (run) {
			render();
			game::engine.render(0);
		}

		thMain.join();
		thDebug.join();
		//game::engine.getSystem<WorldSystem>()->thAmbient.join();
	}

	// put away the brice for later
	game::briceSave();

	// free library resources
    Mix_HaltMusic();
    Mix_CloseAudio();

//    destroyInventory();
	ui::destroyFonts();
    Texture::freeTextures();

	// close up the game stuff
	game::engine.getSystem<WorldSystem>()->save();

	game::engine.getSystem<WindowSystem>()->die();

    return 0; // Calls everything passed to atexit
}

void render() {
	const auto SCREEN_WIDTH = game::SCREEN_WIDTH;
	const auto SCREEN_HEIGHT = game::SCREEN_HEIGHT;

	auto ps = game::engine.getSystem<PlayerSystem>();
	offset.x = ps->getPosition().x + ps->getWidth() / 2;

	const auto& worldWidth = game::engine.getSystem<WorldSystem>()->getWidth();
	if (worldWidth < (int)SCREEN_WIDTH)
		offset.x = 0;
	else if (offset.x - SCREEN_WIDTH / 2 < worldWidth * -0.5f)
		offset.x = ((worldWidth * -0.5f) + SCREEN_WIDTH / 2);
	else if (offset.x + SCREEN_WIDTH / 2 > worldWidth *  0.5f)
		offset.x = ((worldWidth *  0.5f) - SCREEN_WIDTH / 2);

	// ortho y snapping
	offset.y = /*std::max(player->loc.y + player->height / 2,*/ SCREEN_HEIGHT / 2.0f; /*);*/

	// "setup"
	glm::mat4 projection = glm::ortho(floor(offset.x - SCREEN_WIDTH / 2),          // left
	                                  floor(offset.x + SCREEN_WIDTH / 2),          // right
	                                  floor(offset.y - SCREEN_HEIGHT / 2),         // bottom
	                                  floor(offset.y + SCREEN_HEIGHT / 2),         // top
	                                  static_cast<decltype(floor(10.0f))>(10.0),   // near
	                                  static_cast<decltype(floor(10.0f))>(-10.0)); // far

	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),   // pos
								 glm::vec3(0.0f, 0.0f, -10.0f), // looking at
								 glm::vec3(0.0f, 1.0f, 0.0f));  // up vector

	glm::mat4 ortho = projection * view;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO add depth
    glEnable(GL_DEPTH_TEST);

	Render::textShader.use();
		glUniformMatrix4fv(Render::textShader.uniform[WU_transform], 1, GL_FALSE, glm::value_ptr(ortho));
    	glUniform4f(Render::textShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.0);
	Render::textShader.unuse();
    Render::worldShader.use();
		glUniformMatrix4fv(Render::worldShader.uniform[WU_ortho], 1, GL_FALSE, glm::value_ptr(ortho));
		glUniformMatrix4fv(Render::worldShader.uniform[WU_transform], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	Render::worldShader.unuse();

	// draw the world and player
	game::engine.getSystem<WorldSystem>()->render();

	// draw the player's inventory
	//player->inv->draw();

	// draw the fade overlay
	ui::drawFade();

	// draw ui elements
	ui::draw();

	// draw the debug overlay if desired
	if (ui::debug) {
		auto pos = game::engine.getSystem<PlayerSystem>()->getPosition();
		ui::putText(offset.x - SCREEN_WIDTH / 2, (offset.y + SCREEN_HEIGHT / 2) - ui::fontSize,
		            "loc: (%+.2f, %+.2f)\noffset: (%+.2f, %+.2f)\nfps: %d\nticks: %d\nxml: %s",
					pos.x,
					pos.y,
					offset.x,
					offset.y,
					fps,
					game::time::getTickCount(),
					game::engine.getSystem<WorldSystem>()->getXMLFile().c_str()
		            );
		/*ui::putText(offset.x-SCREEN_WIDTH/2, (offset.y+SCREEN_HEIGHT/2)-ui::fontSize,
					"fps: %d\ngrounded:%d\nresolution: %ux%u\nentity cnt: %d\nloc: (%+.2f, %+.2f)\nticks: %u\nvolume: %f\nweather: %s\nxml: %s",
					fps,
					0,//player->ground,
					SCREEN_WIDTH,				// Window dimensions
					SCREEN_HEIGHT,				//
					0,//currentWorld->entity.size(),// Size of entity array
					0,//player->loc.x,				// The player's x coordinate
					debugY,						// The player's y coordinate
					game::time::getTickCount(),
					game::config::VOLUME_MASTER,
					game::engine.getSystem<WorldSystem>()->getWeatherStr().c_str(),
					""//currentXML.c_str()
				);*/
	}

	// draw the menu
	if (currentMenu)
		ui::menu::draw();

	// draw the mouse
	Render::textShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mouseTex);
		Render::useShader(&Render::textShader);
		Render::drawRect(ui::mouse, ui::mouse + 15, -9.9);
	Render::textShader.unuse();
}

void logic(){
//	static bool NPCSelected    = false;
//	static bool ObjectSelected = false;

	// exit the game if the player falls out of the world
	/*if (player->loc.y < 0)
		game::endGame();*/

	/*if (player->inv->usingi) {
		for (auto &e : currentWorld->entity) {
			if (player->inv->usingi && !e->isHit() &&
				player->inv->detectCollision(vec2 { e->loc.x, e->loc.y }, vec2 { e->loc.x + e->width, e->loc.y + e->height})) {
				e->takeHit(25, 10);
				break;
			}
		}
		player->inv->usingi = false;
	}*/

	/*for (auto &e : currentWorld->entity) {
		if (e->isAlive() && ((e->type == NPCT) || (e->type == MERCHT) || (e->type == OBJECTT))) {
			if (e->type == OBJECTT && ObjectSelected) {
				e->near = false;
				continue;
			} else if (e->canMove) {
				if (!currentWorld->goWorldLeft(dynamic_cast<NPC *>(e)))
					currentWorld->goWorldRight(dynamic_cast<NPC *>(e));
				e->wander((rand() % 120 + 30));
				if (NPCSelected) {
					e->near = false;
					continue;
				}
			}

			if(e->isInside(ui::mouse) && player->isNear(e)) {
				e->near = true;
				if (e->type == OBJECTT)
					ObjectSelected = true;
				else
					NPCSelected = true;

				if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)) && !ui::dialogBoxExists) {
					if (ui::mouse.x < player->loc.x && player->right)
						player->left = true, player->right = false;
					else if(ui::mouse.x > player->loc.x && player->left)
						player->right = true, player->left = false;
					e->interact();
				}
			} else {
				e->near = false;
			}
		} else if (e->type == MOBT) {
			e->near = player->isNear(e);
			e->wander();
		}
	}*/

	// calculate the world shading value
	worldShade = 50 * sin((game::time::getTickCount() + (DAY_CYCLE / 2)) / (DAY_CYCLE / PI));

	// update fades
	ui::fadeUpdate();

	// create weather particles if necessary
	/*auto weather = game::engine.getSystem<WorldSystem>()->getWeatherId();
	auto worldWidth = game::engine.getSystem<WorldSystem>()->getWidth();
	if (weather == WorldWeather::Rain) {
		for (unsigned int r = (randGet() % 25) + 11; r--;) {
			currentWorld->addParticle(randGet() % worldWidth - (worldWidth / 2),
									  offset.y + game::SCREEN_HEIGHT / 2,
									  HLINES(1.25),										// width
									  HLINES(1.25),										// height
									  randGet() % 7 * .01 * (randGet() % 2 == 0 ? -1 : 1),	// vel.x
									  (4 + randGet() % 6) * .05,							// vel.y
									  { 0, 0, 255 },										// RGB color
									  2500,												// duration (ms)
									  (1 << 0) | (1 << 1)									// gravity and bounce
									  );
		}
	} else if (weather == WorldWeather::Snowy) {
		for (unsigned int r = (randGet() % 25) + 11; r--;) {
			currentWorld->addParticle(randGet() % worldWidth - (worldWidth / 2),
									  offset.y + game::SCREEN_HEIGHT / 2,
									  HLINES(1.25),										// width
									  HLINES(1.25),										// height
							.0001 + randGet() % 7 * .01 * (randGet() % 2 == 0 ? -1 : 1),	// vel.x
									  (4 + randGet() % 6) * -.03,							// vel.y
									  { 255, 255, 255 },									// RGB color
									  5000,												// duration (ms)
									  0													// no gravity, no bounce
									  );
		}
	}*/

	// increment game ticker
	game::time::tick();
	//NPCSelected = false;
	//ObjectSelected = false;
}
