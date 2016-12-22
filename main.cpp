/**
 * @file main.cpp
 * The main file, where it all happens.
 */

// standard library includes
#include <fstream>
#include <mutex>
#include <chrono>
using namespace std::literals::chrono_literals;

// local library includes
#include <entityx/entityx.h>
#include <tinyxml2.h>
using namespace tinyxml2;

// our own includes
#include <brice.hpp>
#include <config.hpp>
#include <common.hpp>
#include <engine.hpp>
#include <gametime.hpp>
#include <player.hpp>
#include <window.hpp>
#include <world.hpp>
#include <render.hpp>
#include <ui.hpp>

/**
 * The currently used folder to look for XML files in.
 */
std::string xmlFolder;

/**
 * The current menu, if any are open (TODO why is this here)
 */
Menu *currentMenu;

/**
 * The current center of the screen, updated in main render.
 */
vec2 offset;

/**
 * The current FPS of the game.
 */
static unsigned int fps = 0;

void render(void);

/**
 * The main program.
 * Init, load, run. Die.
 */
int main(int argc, char *argv[])
{
	static bool worldReset = false, worldDontReallyRun = false;
	std::string worldActuallyUseThisXMLFile;

	//
	// get command line arguments, if any
	//

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

	//
	// init the main game engine
	//

	game::engine.init();
	// used three times below
	auto worldSys = game::engine.getSystem<WorldSystem>();

	//
	// initialize GLEW
	//

#ifndef __WIN32__
	glewExperimental = GL_TRUE;
#endif

	auto glewError = glewInit();
	if (glewError != GLEW_OK)
		UserError(std::string("GLEW was not able to initialize! Error: ")
			+ reinterpret_cast<const char *>(glewGetErrorString(glewError)));

	//
	// start the random number generator (TODO our own?)
	//

	randInit(millis());

	//
	// some basic OpenGL setup stuff
	//

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	// enable v-sync (TODO but 1000 fps?)
	SDL_GL_SetSwapInterval(1);
	// hide the cursor
	SDL_ShowCursor(SDL_DISABLE);
	// switch to pixel grid
	glViewport(0, 0, game::SCREEN_WIDTH, game::SCREEN_HEIGHT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(1, 1, 1, 1);

	//
	// initialize shaders
	//

	std::cout << "Initializing shaders!\n";
	Render::initShaders();
	Colors::init();

	//
	// load some saved data
	//

	game::briceLoad();
	game::briceUpdate();

	//
	// get a world
	//

	if (xmlFolder.empty())
		xmlFolder = "xml/";

	// read in all XML file names in the folder
	std::vector<std::string> xmlFiles;
	if (getdir(std::string("./" + xmlFolder).c_str(), xmlFiles))
		UserError("Error reading XML files!!!");

	// alphabetically sort files
	strVectorSortAlpha(&xmlFiles);

	// kill the world if needed
	if (worldReset) {
		// TODO TODO TODO we do xml/*.dat now...
		game::briceClear();
	}

	// either load the given XML, or find one
	if (!worldActuallyUseThisXMLFile.empty()) {
		worldSys->load(worldActuallyUseThisXMLFile);
	} else {
		// load the first valid XML file for the world
		for (const auto &xf : xmlFiles) {
			if (xf[0] != '.') {
				// read it in
				std::cout << "File to load: " << xf << '\n';
				worldSys->load(xf);
				break;
			}
		}
	}

	//
	// initialize ui
	//

	ui::menu::init();


	/////////////////////////////
	//                         //
	// actually start the game //
	//                         //
	/////////////////////////////

	if (!worldDontReallyRun) {
		// the main loop, in all of its gloriousness...
		std::thread thMain ([&] {
			const bool &run = game::engine.shouldRun;
			while (run) {
				game::time::mainLoopHandler();

				if (game::time::tickHasPassed()) {
					// calculate the world shading value
					worldShade = 50 * sin((game::time::getTickCount() + (DAY_CYCLE / 2)) / (DAY_CYCLE / PI));

					// update fades
					ui::fadeUpdate();

					// increment game ticker
					game::time::tick();
				}

				game::engine.update(game::time::getDeltaTime());

				std::this_thread::sleep_for(1ms);
			}
		});

		// the debug loop, gets debug screen values
		std::thread thDebug ([&] {
			const bool &run = game::engine.shouldRun;
			while (run) {
				fps = 1000 / game::time::getDeltaTime(); // TODO really?
				std::this_thread::sleep_for(1s);
			}
		});

		// thre render loop, renders
		const bool &run = game::engine.shouldRun;
		while (run) {
			render();
			game::engine.render(0);
		}

		// on game end, get back together
		thMain.join();
		thDebug.join();
		//game::engine.getSystem<WorldSystem>()->thAmbient.join(); // segfault or something
	}

	//
	// put away the brice for later, save world
	//

	game::briceSave();
	worldSys->save();

	//
	// close things, free stuff, yada yada
	//

    Mix_HaltMusic();
    Mix_CloseAudio();

	ui::destroyFonts();
    unloadTextures();

	game::engine.getSystem<WindowSystem>()->die();

	//
	// goodbye
	//

    return 0; // Calls everything passed to atexit
}

void render() {
	static const Texture mouseTex ("assets/mouse.png");
	static const glm::mat4 view = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 0.0f),   // pos
		glm::vec3(0.0f, 0.0f, -10.0f), // looking at
		glm::vec3(0.0f, 1.0f, 0.0f)    // up vector
	);

	static const auto& SCREEN_WIDTH2  = game::SCREEN_WIDTH / 2.0f;
	static const auto& SCREEN_HEIGHT2 = game::SCREEN_HEIGHT / 2.0f;

	//
	// set the ortho
	//

	auto ps = game::engine.getSystem<PlayerSystem>();
	auto ploc = ps->getPosition();
	offset.x = ploc.x + ps->getWidth() / 2;

	const auto& worldWidth = game::engine.getSystem<WorldSystem>()->getWidth();
	if (worldWidth < (int)SCREEN_WIDTH2 * 2)
		offset.x = 0;
	else if (offset.x - SCREEN_WIDTH2 < worldWidth * -0.5f)
		offset.x = ((worldWidth * -0.5f) + SCREEN_WIDTH2);
	else if (offset.x + SCREEN_WIDTH2 > worldWidth *  0.5f)
		offset.x = ((worldWidth *  0.5f) - SCREEN_WIDTH2);

	// ortho y snapping
	offset.y = std::max(ploc.y /*+ player->height / 2*/, SCREEN_HEIGHT2);

	// "setup"
	glm::mat4 projection = glm::ortho(floor(offset.x - SCREEN_WIDTH2),             // left
	                                  floor(offset.x + SCREEN_WIDTH2),             // right
	                                  floor(offset.y - SCREEN_HEIGHT2),            // bottom
	                                  floor(offset.y + SCREEN_HEIGHT2),            // top
	                                  static_cast<decltype(floor(10.0f))>(10.0),   // near
	                                  static_cast<decltype(floor(10.0f))>(-10.0)); // far

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
		ui::putText(offset.x - SCREEN_WIDTH2, (offset.y + SCREEN_HEIGHT2) - ui::fontSize,
		            "loc: %s\noffset: %s\nfps: %d\nticks: %d\nxml: %s",
					pos.toString().c_str(), offset.toString().c_str(), fps,
					game::time::getTickCount(), game::engine.getSystem<WorldSystem>()->getXMLFile().c_str());
	}

	// draw the menu
	if (currentMenu)
		ui::menu::draw();

	// draw the mouse
	Render::textShader.use();
		glActiveTexture(GL_TEXTURE0);
		mouseTex.use();
		Render::useShader(&Render::textShader);
		Render::drawRect(ui::mouse, ui::mouse + 15, -9.9);
	Render::textShader.unuse();
}
