/* ----------------------------------------------------------------------------
** The main file, home of the main loop.
** --------------------------------------------------------------------------*/
// ...
/* ----------------------------------------------------------------------------
** Includes section
** --------------------------------------------------------------------------*/

// local library includes
#include <tinyxml2.h>
using namespace tinyxml2;

// local game includes
#include <common.hpp>
#include <config.hpp>
#include <entities.hpp>
#include <world.hpp>
#include <ui.hpp>
#include <gametime.hpp>

/* ----------------------------------------------------------------------------
** Variables section
** --------------------------------------------------------------------------*/

// the game's window title name
constexpr const char *GAME_NAME = "Independent Study v0.7 alpha - NOW WITH lights and snow and stuff";

// the current weather, declared in world.cpp
extern WorldWeather weather;

// SDL's window object
SDL_Window *window = NULL;

// main loop runs based on this variable's value
bool gameRunning = false;

// world objects for the current world and the two that are adjacent
World *currentWorld        = NULL,
	  *currentWorldToLeft  = NULL,
	  *currentWorldToRight = NULL;

// the currently used folder to grab XML files
std::string xmlFolder;

// the current menu
Menu *currentMenu;

// the player object
Player *player;

// shaders for rendering
GLuint fragShader;
GLuint shaderProgram;

// keeps a simple palette of colors for single-color draws
GLuint colorIndex;

// the mouse's texture
GLuint mouseTex;

// the center of the screen
vec2 offset;

// handles all logic operations
void logic(void);

// handles all rendering operations
void render(void);

// takes care of *everything*
void mainLoop(void);

/*******************************************************************************
** MAIN ************************************************************************
********************************************************************************/

int main(int argc, char *argv[]){
	(void)argc;
	(void)argv;

	static SDL_GLContext mainGLContext = NULL;

	// attempt to initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
		UserError(std::string("SDL was not able to initialize! Error: ") + SDL_GetError());
	atexit(SDL_Quit);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	// attempt to initialize SDL_image
	if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG)))
		UserError(std::string("Could not init image libraries! Error: ") + IMG_GetError());
	atexit(IMG_Quit);

	// attempt to initialize SDL_mixer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
		UserError(std::string("SDL_mixer could not initialize! Error: ") + Mix_GetError());
	Mix_AllocateChannels(8);
	atexit(Mix_Quit);

	// update values by reading the config file (config/settings.xml)
	game::config::read();

	// create the SDL window object
	window = SDL_CreateWindow(GAME_NAME,
							  SDL_WINDOWPOS_UNDEFINED,	// Spawn the window at random (undefined) x and y coordinates
							  SDL_WINDOWPOS_UNDEFINED,	//
							  game::SCREEN_WIDTH,
							  game::SCREEN_HEIGHT,
							  SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | (game::FULLSCREEN ? SDL_WINDOW_FULLSCREEN : 0)
							  );

    if (window == NULL)
		UserError(std::string("The window failed to generate! SDL_Error: ") + SDL_GetError());

    // create the OpenGL object that SDL provides
    if ((mainGLContext = SDL_GL_CreateContext(window)) == NULL)
		UserError(std::string("The OpenGL context failed to initialize! SDL_Error: ") + SDL_GetError());

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

	const GLchar *shaderSource = readFile("frig.frag");
	GLint bufferln = GL_FALSE;
	int logLength;

	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &shaderSource, NULL);
	glCompileShader(fragShader);

	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &bufferln);
	glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);

	std::vector<char> fragShaderError ((logLength > 1) ? logLength : 1);

	glGetShaderInfoLog(fragShader, logLength, NULL, &fragShaderError[0]);
	std::cout << &fragShaderError[0] << std::endl;

	if (bufferln == GL_FALSE)
		UserError("Error compiling shader");

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);
	glValidateProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &bufferln);
    glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> programError((logLength > 1) ? logLength : 1);
    glGetProgramInfoLog(shaderProgram, logLength, NULL, &programError[0]);
    std::cout << &programError[0] << std::endl;

	delete[] shaderSource;

	glEnable(GL_MULTISAMPLE);

	/*
	 * Load sprites used in the inventory menu. See src/inventory.cpp
	 */

	initInventorySprites();
	// load mouse texture, and other inventory textures
	mouseTex = Texture::loadTexture("assets/mouse.png");


	// read in all XML file names in the folder
	std::vector<std::string> xmlFiles;
	if (xmlFolder.empty())
		xmlFolder = "xml/";
	if (getdir(std::string("./" + xmlFolder).c_str(), xmlFiles))
		UserError("Error reading XML files!!!");

	// alphabetically sort files
	strVectorSortAlpha(&xmlFiles);

	// load the first valid XML file for the world
	for (const auto &xf : xmlFiles) {
		if (xf[0] != '.' && strcmp(&xf[xf.size() - 3], "dat")){
			// read it in
			std::cout << "File to load: " << xf << '\n';
			currentWorld = loadWorldFromXML(xf);
			break;
		}
	}

	// spawn the player
	player = new Player();
	player->sspawn(0,100);
	ui::menu::init();
	currentWorld->bgmPlay(NULL);

	// make sure the world was made
	if (currentWorld == NULL)
		UserError("Plot twist: The world never existed...?");

	/**************************
	****     GAMELOOP      ****
	**************************/

	// the main loop, in all of its gloriousness..
	gameRunning = true;
	std::thread([&]{while (gameRunning)
		mainLoop();
	}).detach();

	while(gameRunning)
		render();

    // free library resources
    Mix_HaltMusic();
    Mix_CloseAudio();

    destroyInventory();
	ui::destroyFonts();
    Texture::freeTextures();

    SDL_GL_DeleteContext(mainGLContext);
    SDL_DestroyWindow(window);

	// close up the game stuff
	currentWorld->save();
	//delete currentWorld;
	//delete[] currentXML;
	//aipreload.clear();

    return 0; // Calls everything passed to atexit
}

/*
 * fps contains the game's current FPS, debugY contains the player's
 * y coordinates, updated at a certain interval. These are used in
 * the debug menu (see below).
 */

static unsigned int fps=0;
static float debugY=0;

void mainLoop(void){
	static unsigned int debugDiv=0;			// A divisor used to update the debug menu if it's open
	World *prev;

	game::time::mainLoopHandler();

	if (currentMenu) {
		return;
	} else {
		// handle keypresses - currentWorld could change here
		prev = currentWorld;
		ui::handleEvents();

		if(prev != currentWorld){
			currentWorld->bgmPlay(prev);
			ui::dialogBoxExists = false;
		}

		if (game::time::tickHasPassed())
			logic();

		currentWorld->update(player, game::time::getDeltaTime());
		currentWorld->detect(player);

		if (++debugDiv == 20) {
			debugDiv=0;

			fps = 1000 / game::time::getDeltaTime();
			debugY = player->loc.y;
		}
		SDL_Delay(1);
	}

	SDL_Delay(1);
}

void render() {
	auto SCREEN_WIDTH = game::SCREEN_WIDTH;
	auto SCREEN_HEIGHT = game::SCREEN_HEIGHT;

	// offset should contain the coordinates of the center of the player's view
	offset.x = player->loc.x + player->width/2;
	offset.y = SCREEN_HEIGHT/2;

	// snap the player's view if we're at a world edge
	if (currentWorld->getTheWidth() < (int)SCREEN_WIDTH) {
		offset.x = 0;
	} else {
		if (player->loc.x - SCREEN_WIDTH/2 < currentWorld->getTheWidth() * -0.5f)
			offset.x = ((currentWorld->getTheWidth() * -0.5f) + SCREEN_WIDTH / 2) + player->width / 2;
		if (player->loc.x + player->width + SCREEN_WIDTH/2 > currentWorld->getTheWidth() *  0.5f)
			offset.x = ((currentWorld->getTheWidth() *  0.5f) - SCREEN_WIDTH / 2) - player->width / 2;
	}

	if(player->loc.y > SCREEN_HEIGHT/2)
		offset.y = player->loc.y + player->height;

	// "setup"
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(floor(offset.x - SCREEN_WIDTH  / 2), floor(offset.x + SCREEN_WIDTH  / 2),
	        floor(offset.y - SCREEN_HEIGHT / 2), floor(offset.y + SCREEN_HEIGHT / 2),
			20, -20);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_DEPTH_BUFFER_BIT);

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw the world
	//player->near = true; // allow player's name to be drawn
	currentWorld->draw(player);

	// draw the player's inventory
	player->inv->draw();

	// draw the fade overlay
	ui::drawFade();

	// draw ui elements
	ui::draw();

	/*
	 * Draw the debug overlay if it has been enabled.
	 */

	if(ui::debug){
		ui::putText(offset.x-SCREEN_WIDTH/2, (offset.y+SCREEN_HEIGHT/2)-ui::fontSize,
					"fps: %d\ngrounded:%d\nresolution: %ux%u\nentity cnt: %d\nloc: (%+.2f, %+.2f)\nticks: %u\nvolume: %f\nweather: %s",
					fps,
					player->ground,
					SCREEN_WIDTH,				// Window dimensions
					SCREEN_HEIGHT,				//
					currentWorld->entity.size(),// Size of entity array
					player->loc.x,				// The player's x coordinate
					debugY,						// The player's y coordinate
					game::time::getTickCount(),
					game::config::VOLUME_MASTER,
					getWorldWeatherStr(weather).c_str()
					);

		if (ui::posFlag) {
			glBegin(GL_LINES);
				glColor3ub(100,100,255);
				for (auto &e : currentWorld->entity) {
					glVertex2i(player->loc.x + player->width / 2, player->loc.y + player->height / 2);
					glVertex2i(e->loc.x + e->width / 2, e->loc.y + e->height / 2);
				}
			glEnd();
		}
	}

	if (currentMenu)
		ui::menu::draw();

	// draw the mouse cursor
	glColor3ub(255,255,255);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mouseTex);
	glBegin(GL_QUADS);
		glTexCoord2f(0,0);glVertex2i(ui::mouse.x			,ui::mouse.y			);
		glTexCoord2f(1,0);glVertex2i(ui::mouse.x+HLINES(5)	,ui::mouse.y		 	);
		glTexCoord2f(1,1);glVertex2i(ui::mouse.x+HLINES(5)	,ui::mouse.y-HLINES(5)	);
		glTexCoord2f(0,1);glVertex2i(ui::mouse.x 			,ui::mouse.y-HLINES(5) 	);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	// wrap up
	glPopMatrix();
	SDL_GL_SwapWindow(window);
}

void logic(){
	static bool NPCSelected    = false;
	static bool ObjectSelected = false;

	// exit the game if the player falls out of the world
	if (player->loc.y < 0)
		gameRunning = false;

	if (player->inv->usingi) {
		for (auto &e : currentWorld->entity) {
			if (player->inv->usingi && !e->isHit() &&
				player->inv->detectCollision(vec2 { e->loc.x, e->loc.y }, vec2 { e->loc.x + e->width, e->loc.y + e->height})) {
				e->takeHit(25, 10);
				break;
			}
		}
		player->inv->usingi = false;
	}

	for (auto &e : currentWorld->entity) {
		if (e->isAlive() && ((e->type == NPCT) || (e->type == MERCHT) || (e->type == OBJECTT))) {
			if (e->type == OBJECTT && ObjectSelected) {
				e->near = false;
				continue;
			} else if (e->canMove) {
				e->wander((rand() % 120 + 30));
				if (NPCSelected) {
					e->near = false;
					continue;
				}
			}

			if(e->isInside(ui::mouse) && player->isNear(*e)) {
				e->near = true;
				if (e->type == OBJECTT)
					ObjectSelected = true;
				else
					NPCSelected = true;

				if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)) && !ui::dialogBoxExists)
					e->interact();
			} else {
				e->near = false;
			}
		} else if (e->type == MOBT) {
			e->near = player->isNear(*e);
			e->wander();
		}
	}

	// switch from day to night?
	auto tickCount = game::time::getTickCount();
	if (!(tickCount % DAY_CYCLE) || !tickCount){
		if (weather == WorldWeather::Sunny)
			weather = WorldWeather::Dark;
		else
			weather = WorldWeather::Sunny;
	}

	// calculate the world shading value
	worldShade = 50 * sin((tickCount + (DAY_CYCLE / 2)) / (DAY_CYCLE / PI));

	// update fades
	ui::fadeUpdate();

	// create weather particles if necessary
	 if (weather == WorldWeather::Rain) {
		 for (unsigned int r = (randGet() % 25) + 11; r--;) {
			 currentWorld->addParticle(randGet() % currentWorld->getTheWidth() - (currentWorld->getTheWidth() / 2),
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
			 currentWorld->addParticle(randGet() % currentWorld->getTheWidth() - (currentWorld->getTheWidth() / 2),
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
	 }

	// increment game ticker
	game::time::tick();
	NPCSelected = false;
	ObjectSelected = false;
}
