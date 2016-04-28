/*! @file main.cpp
 *	@brief The file that links everything together for the game to run.
 *	The  main game loop contains all of the global variables the game uses, and it runs the main game loop, the render loop, and the logic loop that control all of the entities.
 */

#include <tinyxml2.h>
using namespace tinyxml2;

/*
 * Game includes
 */

#include <common.hpp>
#include <config.hpp>
#include <entities.hpp>
#include <world.hpp>
#include <ui.hpp>
#include <gametime.hpp>

// SDL's window object
SDL_Window *window = NULL;

// main loop runs based on this variable's value
bool gameRunning;

// world objects for the current world and the two that are adjacent
World *currentWorld        = NULL,
	  *currentWorldToLeft  = NULL,
	  *currentWorldToRight = NULL;

Menu *currentMenu;

// the player object
Player *player;

/**
 * TODO
 */

GLuint fragShader;

/**
 * TODO
 */

GLuint shaderProgram;

/**
 *	Threads and various variables to be used when multithreading the game,
 *  mutex will prevent multiple threads from changing the same data,
 *  and the condition_variable will wait for threads to reach the same point
 */

std::mutex mtx;
std::condition_variable cv;
ThreadPool pool(10);

/*
 *	loops is used for texture animation. It is believed to be passed to entity
 *	draw functions, although it may be externally referenced instead.
*/

/**
 * TODO
 */

GLuint colorIndex;

/**
 * TODO
 */

GLuint mouseTex;

/**
 * Used for texture animation. It is externally referenced by ui.cpp
 * and entities.cpp.
 */

unsigned int loops = 0;

/**
 * Gives a coordinate based off of the player's location to allow for drawing to
 * be in a constant 'absolute' place on the window.
 */

vec2 offset;

std::string xmlFolder;

extern WorldWeather weather;

unsigned int SCREEN_WIDTH;
unsigned int SCREEN_HEIGHT;
unsigned int HLINE;
bool FULLSCREEN;

float VOLUME_MASTER;
float VOLUME_MUSIC;
float VOLUME_SFX;

/**
 * Defined in gameplay.cpp, should result in `currentWorld` containing a pointer
 * to a valid World.
 */

extern void initEverything(void);

/**
 * The game logic function, should handle all logic-related operations for the
 * game.
 */

void logic(void);

/**
 * The game render function, should handle all drawing to the window.
 */

void render(void);

/**
 * The main loop, calls logic(), render(), and does timing operations in the
 * appropriate order.
 */

void mainLoop(void);

/*******************************************************************************
 * MAIN ************************************************************************
 *******************************************************************************/

int main(int argc, char *argv[]){
	(void)argc;
	(void)argv;

	static SDL_GLContext mainGLContext = NULL;

	gameRunning = false;

	/**
	 * (Attempt to) Initialize SDL libraries so that we can use SDL facilities and eventually
	 * make openGL calls. Exit if there was an error.
	 */

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0){
		std::cout << "SDL was not able to initialize! Error: " << SDL_GetError() << std::endl;
		return -1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	// Run SDL_Quit when main returns
	atexit(SDL_Quit);

	/**
	 * (Attempt to) Initialize SDL_image libraries with IMG_INIT_PNG so that we can load PNG
	 * textures for the entities and stuff.
	 */

	if(!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))){
		std::cout << "Could not init image libraries! Error: " << IMG_GetError() << std::endl;
		return -1;
	}

	// Run IMG_Quit when main returns
	atexit(IMG_Quit);

	/**
	 * (Attempt to) Initialize SDL_mixer libraries for loading and playing music/sound files.
	 */

	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
		std::cout << "SDL_mixer could not initialize! Error: " << Mix_GetError() << std::endl;
		return -1;
	}

	Mix_AllocateChannels(8);

	config::update();

	// Run Mix_Quit when main returns
	atexit(Mix_Quit);

	/**
	 * Load saved settings into the game (see config/settings.xml)
	 */

	config::read();

	/*
	 *	Create a window for SDL to draw to. Most parameters are the default, except for the
	 *	following which are defined in include/common.h:
	 *
	 *	GAME_NAME		the name of the game that is displayed in the window title bar
	 *	SCREEN_WIDTH	the width of the created window
	 *	SCREEN_HEIGHT	the height of the created window
	 *	FULLSCREEN		makes the window fullscreen
	 *
	 */

	uint32_t SDL_CreateWindowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | (FULLSCREEN ? SDL_WINDOW_FULLSCREEN : 0);

	window = SDL_CreateWindow(GAME_NAME,
							  SDL_WINDOWPOS_UNDEFINED,	// Spawn the window at random (undefined) x and y coordinates
							  SDL_WINDOWPOS_UNDEFINED,	//
							  SCREEN_WIDTH,
							  SCREEN_HEIGHT,
							  SDL_CreateWindowFlags
							 );

    /*
     * Exit if the window cannot be created
     */

    if(window==NULL){
		std::cout << "The window failed to generate! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    /*
     * Create the SDL OpenGL context. Once created, we are allowed to use OpenGL functions.
     * Saving this context to mainGLContext does not appear to be necessary as mainGLContext
     * is never referenced again.
     */

    if((mainGLContext = SDL_GL_CreateContext(window)) == NULL){
		std::cout << "The OpenGL context failed to initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

	/*
	 * Initialize GLEW libraries, and exit if there was an error.
	 * Not sure what they're for yet.
	 */

	GLenum err;
#ifndef __WIN32__
	glewExperimental = GL_TRUE;
#endif
	if((err=glewInit()) != GLEW_OK){
		std::cout << "GLEW was not able to initialize! Error: " << glewGetErrorString(err) << std::endl;
		return -1;
	}

	/*
	 * Initialize the random number generator. At the moment, initRand is a macro pointing to libc's
	 * srand, and its partner getRand points to rand. This is because having our own random number
	 * generator may be favorable in the future, but at the moment is not implemented.
	 */

	initRand(millis());

	/*
	 * Do some basic setup for openGL. Enable double buffering, switch to by-pixel coordinates,
	 * setup the alpha channel for textures/transparency, and finally hide the system's mouse
	 * cursor so that we may draw our own.
	 */

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetSwapInterval(0);

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	SDL_ShowCursor(SDL_DISABLE);

	Texture::initColorIndex();

	/*
	 * Initializes our shaders so that the game has shadows.
	 */

	std::cout << "Initializing shaders!" << std::endl;

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

	/*
	 * Create all the worlds, entities, mobs, and the player. This function is defined in
	 * src/gameplay.cpp
	 */

	initEverything();

	if(!currentWorld){
		std::cout<<"currentWorld == NULL!"<<std::endl;
#ifndef __WIN32__
		system("systemctl poweroff");
#else
		system("shutdown -s -t 0");
#endif // __WIN32__
		abort();
	}


	mouseTex = Texture::loadTexture("assets/mouse.png");

	/**************************
	****     GAMELOOP      ****
	**************************/

	std::cout << "Num threads: " << std::thread::hardware_concurrency() << std::endl;

	glClearColor(1,1,1,1);
	//ui::toggleBlackFast();

	gameRunning = true;
	while (gameRunning)
		mainLoop();

	/**************************
	****   CLOSE PROGRAM   ****
	**************************/

    /*
     * Close the window and free resources
     */

    Mix_HaltMusic();
    Mix_CloseAudio();

    destroyInventory();
	ui::destroyFonts();
    Texture::freeTextures();

    SDL_GL_DeleteContext(mainGLContext);
    SDL_DestroyWindow(window);

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

	gtime::mainLoopHandler();

	if (currentMenu)
		goto MENU;

	// handle keypresses - currentWorld could change here
	prev = currentWorld;
	ui::handleEvents();

	if(prev != currentWorld){
		currentWorld->bgmPlay(prev);
		ui::dialogBoxExists = false;
	}

	if (gtime::tickHasPassed())
		logic();

	currentWorld->update(player, gtime::getDeltaTime());
	currentWorld->detect(player);

	if (++debugDiv == 20) {
		debugDiv=0;

		fps = 1000 / (gtime::getDeltaTime() + .000001);
		if (!(debugDiv % 10))
			debugY = player->loc.y;
	}
MENU:
	render();
}

void render() {

	 /*
	  *	This offset variable is what we use to move the camera and locked
	  *	objects on the screen so they always appear to be in the same relative area
	  */

	offset.x = player->loc.x + player->width/2;
	offset.y = SCREEN_HEIGHT/2;

	/*
	 * If the camera will go off of the left  or right of the screen we want to lock it so we can't
	 * see past the world render
	 */

	if(currentWorld->getTheWidth() < (int)SCREEN_WIDTH){
		offset.x = 0;
	}else{
		if(player->loc.x - SCREEN_WIDTH/2 < currentWorld->getTheWidth() * -0.5f)
			offset.x = ((currentWorld->getTheWidth() * -0.5f) + SCREEN_WIDTH / 2) + player->width / 2;
		if(player->loc.x + player->width + SCREEN_WIDTH/2 > currentWorld->getTheWidth() *  0.5f)
			offset.x = ((currentWorld->getTheWidth() *  0.5f) - SCREEN_WIDTH / 2) - player->width / 2;
	}

	if(player->loc.y > SCREEN_HEIGHT/2)
		offset.y = player->loc.y + player->height;

	/*
	 *	These functions run everyloop to update the current stacks presets
	 *
	 *	Matrix 	----	A matrix is a blank "canvas" for the renderer to draw on,
	 *					this canvas can be rotated, scales, skewed, etc..
	 *
	 *	Stack 	----	A stack is exactly what it sounds like, it is a stack.. A
	 *					stack is a "stack" of matrices for the renderer to draw on.
	 *					Each stack can be made up of varying amounts of matricies.
	 *
	 *	glMatrixMode	This changes our current stacks mode so the drawings below
	 *					it can take on certain traits.
	 *
	 *	GL_PROJECTION	This is the matrix mode that sets the cameras position,
	 *					GL_PROJECTION is made up of a stack with two matrices which
	 *					means we can make up to 2 seperate changes to the camera.
	 *
	 *	GL_MODELVIEW	This matrix mode is set to have the dimensions defined above
	 *					by GL_PROJECTION so the renderer can draw only what the camera
	 *					is looking at. GL_MODELVIEW has a total of 32 matrices on it's
	 *					stack, so this way we can make up to 32 matrix changes like,
	 *					scaling, rotating, translating, or flipping.
	 *
	 *	glOrtho			glOrtho sets our ortho, or our cameras resolution. This can also
	 *					be used to set the position of the camera on the x and y axis
	 *					like we have done. The glOrtho must be set while the stack is in
	 *					GL_PROJECTION mode, as this is the mode that gives the
	 *					camera properties.
	 *
	 *	glPushMatrix	This creates a "new" matrix. What it really does is pull a matrix
	 *					off the bottom of the stack and puts it on the top so the renderer
	 *					can draw on it.
	 *
	 *	glLoadIdentity	This scales the current matrix back to the origin so the
	 *					translations are seen normally on a stack.
	 */

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	// glOrtho((offset.x-SCREEN_WIDTH/2),(offset.x+SCREEN_WIDTH/2),(offset.y-SCREEN_HEIGHT/2),(offset.y+SCREEN_HEIGHT/2),-1,1);
	glOrtho(floor(offset.x-SCREEN_WIDTH/2),floor(offset.x+SCREEN_WIDTH/2),floor(offset.y-SCREEN_HEIGHT/2),floor(offset.y+SCREEN_HEIGHT/2),20,-20);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	/*
	 * glPushAttrib		This passes attributes to the renderer so it knows what it can
	 *					render. In our case, GL_DEPTH_BUFFER_BIT allows the renderer to
	 *					draw multiple objects on top of one another without blending the
	 *					objects together; GL_LIGHING_BIT allows the renderer to use shaders
	 *					and other lighting effects to affect the scene.
	 *
	 * glClear 			This clears the new matrices using the type passed. In our case:
	 *					GL_COLOR_BUFFER_BIT allows the matrices to have color on them
	 */

	glPushAttrib(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/**************************
	**** RENDER STUFF HERE ****
	**************************/

	/*
	 * Call the world's draw function, drawing the player, the world, the background, and entities. Also
	 * draw the player's inventory if it exists.
	 */

	player->near=true;			// Draw the player's name

	currentWorld->draw(player);

	/*
	 * Draw the player's inventory.
	 */

	player->inv->draw();

	/*
	 * Here we draw a black overlay if it's been requested.
	 */

	ui::drawFade();

	/*
	 * Draw UI elements. This includes the player's health bar and the dialog box.
	 */

	ui::draw();

	/*
	 * Draw the debug overlay if it has been enabled.
	 */

	if(ui::debug){

		ui::putText(offset.x-SCREEN_WIDTH/2,
					(offset.y+SCREEN_HEIGHT/2)-ui::fontSize,
					"fps: %d\ngrounded:%d\nresolution: %ux%u\nentity cnt: %d\nloc: (%+.2f, %+.2f)\nticks: %u\nvolume: %f\nweather: %s",
					fps,
					player->ground,
					SCREEN_WIDTH,				// Window dimensions
					SCREEN_HEIGHT,				//
					currentWorld->entity.size(),// Size of entity array
					player->loc.x,				// The player's x coordinate
					debugY,						// The player's y coordinate
					gtime::getTickCount(),
					VOLUME_MASTER,
					getWorldWeatherStr(weather).c_str()
					);

		if (ui::posFlag) {
			glBegin(GL_LINES);
				/*glColor3ub(255,0,0);
				glVertex2i(0,0);
				glVertexdw2i(0,SCREEN_HEIGHT);*/

				/*glColor3ub(255,255,255);
				glVertex2i(player->loc.x + player->width/2,0);
				glVertex2i(player->loc.x + player->width/2,SCREEN_HEIGHT);
				glVertex2i(offset.x - SCREEN_WIDTH / 2, player->loc.y + player->height / 2);
				glVertex2i(offset.x + SCREEN_WIDTH / 2, player->loc.y + player->height / 2);*/

				/*glVertex2i(-SCREEN_WIDTH / 2 + offset.x, player->loc.y);
				glVertex2i( SCREEN_WIDTH / 2 + offset.x, player->loc.y);*/

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
		glTexCoord2f(1,0);glVertex2i(ui::mouse.x+HLINE*5	,ui::mouse.y		 	);
		glTexCoord2f(1,1);glVertex2i(ui::mouse.x+HLINE*5	,ui::mouse.y-HLINE*5	);
		glTexCoord2f(0,1);glVertex2i(ui::mouse.x 			,ui::mouse.y-HLINE*5 	);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	/**************************
	****  END RENDERING   ****
	**************************/

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

			switch (e->subtype) {
			case MS_RABBIT:
			case MS_BIRD:
				e->wander((rand()%240 + 15));
				break;
			case MS_TRIGGER:
			case MS_PAGE:
				e->wander(0);
				break;
			case MS_DOOR:
				break;
			default:
				std::cout<<"Unhandled mob of subtype "<<e->subtype<<"."<<std::endl;
				break;
			}
		}
	}

	/*
	 *	Switch between day and night (SUNNY and DARK) if necessary.
	 */

	if (!(gtime::getTickCount() % DAY_CYCLE) || !gtime::getTickCount()){
		if (weather == WorldWeather::Sunny)
			weather = WorldWeather::Dark;
		else {
			weather = WorldWeather::Sunny;
			Mix_Pause(2);
		}
	}

	/*
	 *	Calculate an in-game shading value (as opposed to GLSL shading).
	*/

	worldShade = 50 * sin((gtime::getTickCount() + (DAY_CYCLE / 2)) / (DAY_CYCLE / PI));

	/*
	 *	Transition to and from black if necessary.
	*/

	ui::fadeUpdate();

	/*
	 * Rain?
	 */

	 if (weather == WorldWeather::Rain) {
		 for (unsigned int r = (randGet() % 25) + 11; r--;) {
			 currentWorld->addParticle(randGet() % currentWorld->getTheWidth() - (currentWorld->getTheWidth() / 2),
									   offset.y + SCREEN_HEIGHT / 2,
									   HLINE * 1.25,										// width
									   HLINE * 1.25,										// height
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
									   offset.y + SCREEN_HEIGHT / 2,
									   HLINE * 1.25,										// width
									   HLINE * 1.25,										// height
							.0001 + randGet() % 7 * .01 * (randGet() % 2 == 0 ? -1 : 1),	// vel.x
									   (4 + randGet() % 6) * -.03,							// vel.y
									   { 255, 255, 255 },									// RGB color
									   5000,												// duration (ms)
									   0													// no gravity, no bounce
									  );
		 }
	 }

	/*
	 *	Increment a loop counter used for animating sprites.
	*/

	loops++;
	gtime::tick();
	NPCSelected=false;	// See above
}
