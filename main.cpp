/**
 * @file main.cpp
 * The main file, where it all happens.
 */

// standard library includes
#include <chrono>
using namespace std::literals::chrono_literals;

// local library includes
#include <entityx/entityx.h>

// our own includes
#include <brice.hpp>
#include <config.hpp>
#include <common.hpp>
#include <engine.hpp>
#include <error.hpp>
#include <fileio.hpp>
#include <font.hpp>
#include <gametime.hpp>
#include <window.hpp>
#include <world.hpp>
#include <render.hpp>
#include <ui.hpp>
#include <inventory.hpp>

/**
 * The currently used folder to look for XML files in.
 */
std::string xmlFolder = "./xml/";

/**
 * The current center of the screen, updated in main render.
 */
vec2 offset;

std::size_t getUsedMem(void);
void deleteRemaining(void);
extern int balance;

void print(void) {
	std::cout << "Used memory: " << getUsedMem() << " bytes\n";
	std::cout << "New/delete balance: " << balance << '\n';
}

/**
 * The main program.
 * Init, load, run. Die.
 */
int main(int argc, char *argv[])
{
	static bool worldReset = false, worldDontReallyRun = false;
	std::string worldActuallyUseThisXMLFile;

	print();
	atexit(print);
	//atexit(deleteRemaining);

	// handle command line args
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

	// initialize the renderer
	Render::init();

	// start the random number generator
	randInit(millis());

	// load some saved data
	game::briceLoad();
	game::briceUpdate();

	// read in all XML file names in the folder
	std::list<std::string> xmlFiles;
	if (getdir(std::string("./" + xmlFolder), xmlFiles))
		UserError("Error reading XML files!!!");

	// kill the world if needed
	if (worldReset) {
		for (const auto& s : xmlFiles) {
			if (s.find(".dat", s.size() - 4) != std::string::npos) {
				std::string re = xmlFolder;
				re.append(s);
				auto r = re.c_str();
				std::cout << "Removing " << r << "...\n";
				std::remove(r);
			}
		}

		// TODO TODO TODO we do xml/*.dat now... kill that
		game::briceClear();
	}

	// either load the given XML, or find one
	if (!worldActuallyUseThisXMLFile.empty()) {
		WorldSystem::load(worldActuallyUseThisXMLFile);
	} else {
		// load the first valid XML file for the world
		for (const auto &xf : xmlFiles) {
			if (xf[0] != '.') {
				// read it in
				std::cout << "File to load: " << xf << '\n';
				WorldSystem::load(xf);
				break;
			}
		}
	}

	/////////////////////////////
	//                         //
	// actually start the game //
	//                         //
	/////////////////////////////


	InventorySystem::add("Hunters Bow", 1);
	InventorySystem::add("Wood Sword", 1);
	InventorySystem::add("Arrow", 198);

	std::list<SDL_Event> eventQueue;

	if (!worldDontReallyRun) {
		// the main loop, in all of its gloriousness...
		GameThread gtMain ([&] {
			game::time::mainLoopHandler();

			if (game::time::tickHasPassed()) {
				// calculate the world shading value
				extern int worldShade; // TODO kill
				worldShade = 50 * sin((game::time::getTickCount() + (DAY_CYCLE / 2)) / (DAY_CYCLE / PI));

				// increment game ticker
				game::time::tick();
			}

			while (!eventQueue.empty()) {
				game::events.emit<MainSDLEvent>(eventQueue.back());
				eventQueue.pop_back();
			}

			game::engine.update(game::time::getDeltaTime());
			std::this_thread::sleep_for(1ms);
		});
		
		static int fps = 0, fpsInternal = 0;

		// the debug loop, gets debug screen values
		GameThread gtDebug ([&] {
			fps = fpsInternal, fpsInternal = 0;
			std::this_thread::sleep_for(1s);
		});

		/*GameThread gtFade ([&] {
			ui::fadeUpdate();
			std::this_thread::sleep_for(20ms);
		});*/

		// the render loop, renders
		const bool &run = game::engine.shouldRun;
		while (run) {
			fpsInternal++;
			Render::render(fps);
			
			SDL_Event e;
			while (SDL_PollEvent(&e)) {
				ui::handleGLEvent(e);
				eventQueue.push_back(e);
			}
		}

		// on game end, get back together
		gtMain.stop();
		gtDebug.stop();
		//gtFade.stop();
		//game::engine.getSystem<WorldSystem>()->thAmbient.join(); // segfault or something
	}

	// save
	game::briceSave();
	WorldSystem::save();

	// exit
    Mix_HaltMusic();

    unloadTextures();

	WorldSystem::die();
	WindowSystem::die();

	return 0; // Calls everything passed to atexit
}

constexpr int memEntries = 2048;

static void* mems[memEntries];
static std::size_t sizs[memEntries];

int balance = 0;

std::size_t getUsedMem(void)
{
	std::size_t total = 0;
	for (int i = 0; i < memEntries; i++)
		total += sizs[i];

	return total;
}

void deleteRemaining(void)
{
	for (int i = 0; i < memEntries; i++) {
		if (mems[i] != nullptr) {
			balance--;
			std::free(mems[i]);
			mems[i] = nullptr;
			sizs[i] = 0;
		}
	}
		
}

void *operator new(std::size_t n) throw (std::bad_alloc)
{
	balance++;

	auto buf = std::malloc(n);
	if (buf == nullptr)
		throw std::bad_alloc();

	for (int i = 0; i < memEntries; i++) {
		if (mems[i] == nullptr) {
			mems[i] = buf;
			sizs[i] = n;
			break;
		}
	}

	return buf;
}

void operator delete(void* p) throw ()
{
	if (p != nullptr) {
		balance--;
		for (int i = 0; i < memEntries; i++) {
			if (mems[i] == p) {
				std::free(p);
				mems[i] = nullptr;
				sizs[i] = 0;
				break;
			}
		}
	}
}

void operator delete(void* p, std::size_t n) throw ()
{
	(void)n;

	if (p != nullptr) {
		balance--;
		for (int i = 0; i < memEntries; i++) {
			if (mems[i] == p) {
				std::free(p);
				mems[i] = nullptr;
				sizs[i] = 0;
				break;
			}
		}
	}
}
