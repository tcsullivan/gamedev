/*! @file main.cpp
	@brief The file that links everything together for the game to run.
	The  main game loop contains all of the global variables the game uses, and it runs the main game loop, the render loop, and the logic loop that control all of the entities.
*/

#include <fstream>
#include <istream>
#include <thread>

#include <common.h>
#include <world.h>
#include <ui.h>
#include <entities.h>

/*
 *	TICKS_PER_SEC & MSEC_PER_TICK
 *
 *	The game's main loop mainly takes care of two things: drawing to the
 *	screen and handling game logic, from user input to world gravity stuff.
 *	The call for rendering is made every time the main loop loops, and then
 *	uses interpolation for smooth drawing to the screen. However, the call
 *	for logic would be preferred to be run every set amount of time.
 *
 *
 *	The logic loop is currently implemented to run at a certain interval
 *	that we call a 'tick'. As one may now guess, TICKS_PER_SEC defines the
 *	amount of ticks that should be made every second. MSEC_PER_TICK then
 *	does a simple calculation of how many milliseconds elapse per each
 * 	'tick'. Simple math is then done in the main loop using MSEC_PER_TICK
 *	to call the logic handler when necessary.
 *
*/

#define TICKS_PER_SEC 20
#define MSEC_PER_TICK (1000/TICKS_PER_SEC)

/*
 *	window & mainGLContext
 *
 *	In order to draw using SDL and its openGL facilities SDL requires
 *	an SDL_Window object, which spawns a window for the program to draw
 *	to. Once the SDL_Window is initialized, an SDL_GLContext is made so
 *	that openGL calls can be made to SDL. The game requires both of these
 *	variables to initialize.
 *
*/

SDL_Window    *window = NULL;
SDL_GLContext  mainGLContext = NULL;

/*
 *	bgImage contains the GLuint returned when creating a texture for the
 *	background image. Currently there is only one background image for the
 *	main world; this will be changed and bgImage likely removed once better
 *	backgrounds are implemented.
 *
*/

GLuint  bgDay, bgNight, bgMtn, bgTreesFront, bgTreesMid, bgTreesFar, invUI;

/*
 *	gameRunning
 *
 *	This is one of the most important variables in the program. The main
 *	loop of the game is set to break once this variable is set to false.
 *	The only call to modify this variable is made in src/ui.cpp, where it
 *	is set to false if either an SDL_QUIT message is received (the user
 *	closes the window through their window manager) or if escape is pressed.
 *
*/

bool gameRunning;

float handAngle;

/*
 *	currentWorld 	-	This is a pointer to the current world that the player
 * 						is in. Most drawing/entity handling is done through this
 * 						variable. This should only be changed when layer switch
 * 						buttons are pressed (see src/ui.cpp), or when the player
 * 						enters a Structure/Indoor World (see src/ui.cpp again).
 *
 *	player			-	This points to a Player object, containing everything for
 * 						the player. Most calls made with currentWorld require a
 * 						Player object as an argument, and glOrtho is set based
 * 						off of the player's coordinates. This is probably the one
 * 						Entity-derived object that is not pointed to in the entity
 * 						array.
 *
*/

World  							*currentWorld=NULL;
Player 							*player;
/*
 *	Tells if player is currently inside a structure.
*/

extern bool worldInside;

/*
 *	tickCount contains the number of ticks generated since main loop entrance.
 *	This variable might be used anywhere.
 *
 *	deltaTime is used for interpolation stuff.
 *
 *	Pretty sure these variables are considered static as they might be externally
 *	referenced somewhere.
*/

unsigned int tickCount = 0;
unsigned int deltaTime = 0;

/*
 *
*/

GLuint fragShader;
GLuint shaderProgram;

Mix_Chunk *crickets;

/*
 *	names is used to open a file containing all possible NPC names. It is externally
 *	referenced in src/entities.cpp for getting random names.
*/

std::istream *names;

/*
 *	loops is used for texture animation. It is believed to be passed to entity
 *	draw functions, although it may be externally referenced instead.
*/

unsigned int loops = 0;	// Used for texture animation

/*
 *	initEverything
 *
 *	Before the main loop, things like the player, entities, and worlds should
 *	be created. This game has not reached the point that these can be scripted
 *	or programmed, so this function substitues for that. It is defined in
 *	src/gameplay.cpp.
 *
*/

extern void initEverything(void);

/*
 *	mainLoop is in fact the main loop, which runs 'infinitely' (as long as gameRunning
 *
 *
 *
 *	is set). Each loop updates timing values (tickCount and deltaTime), runs logic()
 *	if MSEC_PER_TICK milliseconds have passed, and then runs render().
 *
 *	logic handles all user input and entity/world physics.
 *
 *	render handles all drawing to the window, calling draw functions for everything.
 *
*/

void logic(void);
void render(void);
void mainLoop(void);

std::string readFile(const char *filePath) {
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);

    if(!fileStream.is_open()) {
        std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
        return "";
    }

    std::string line = "";
    while(!fileStream.eof()) {
        std::getline(fileStream, line);
        content.append(line + "\n");
    }

    fileStream.close();
    return content;
}

/*
 *	This offset is used as the player offset in the world drawing so
 *	everything can be moved according to the player
*/

vec2 offset;																			/*	OFFSET!!!!!!!!!!!!!!!!!!!! */

extern WEATHER weather;

extern bool fadeEnable;
extern bool fadeWhite;
extern bool fadeFast;
extern int  fadeIntensity;

/*******************************************************************************
 * MAIN ************************************************************************
 *******************************************************************************/
int main(/*int argc, char *argv[]*/){
	//*argv = (char *)argc;
	gameRunning=false;

	/*!
	 *	(Attempt to) Initialize SDL libraries so that we can use SDL facilities and eventually
	 *	make openGL calls. Exit if there was an error.
	*/

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0){
		std::cout << "SDL was not able to initialize! Error: " << SDL_GetError() << std::endl;
		return -1;
	}

	// Run SDL_Quit when main returns
	atexit(SDL_Quit);

	/*!
	 *	(Attempt to) Initialize SDL_image libraries with IMG_INIT_PNG so that we can load PNG
	 *	textures for the entities and stuff.
	*/

	if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)){
		std::cout << "Could not init image libraries! Error: " << IMG_GetError() << std::endl;
		return -1;
	}

	// Run IMG_Quit when main returns
	atexit(IMG_Quit);

	/*!
	 *	(Attempt to) Initialize SDL_mixer libraries for loading and playing music/sound files.
	 *
	*/

	if(Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
		std::cout << "SDL_mixer could not initialize! Error: " << Mix_GetError() << std::endl;
		return -1;
	}
	Mix_AllocateChannels(8);

	// Run Mix_Quit when main returns
	atexit(Mix_CloseAudio);
	atexit(Mix_Quit);

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

	window = SDL_CreateWindow(GAME_NAME,
							  SDL_WINDOWPOS_UNDEFINED,	// Spawn the window at random (undefined) x and y coordinates
							  SDL_WINDOWPOS_UNDEFINED,	//
							  SCREEN_WIDTH,
							  SCREEN_HEIGHT,
							  SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
#ifdef FULLSCREEN
							  | SDL_WINDOW_FULLSCREEN
#endif // FULLSCREEN
				  );

    /*
     *	Exit if the window cannot be created
    */

    if(window==NULL){
		std::cout << "The window failed to generate! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    /*
     *	Create the SDL OpenGL context. Once created, we are allowed to use OpenGL functions.
     *	Saving this context to mainGLContext does not appear to be necessary as mainGLContext
     *	is never referenced again.
     *
    */

    if((mainGLContext = SDL_GL_CreateContext(window)) == NULL){
		std::cout << "The OpenGL context failed to initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

	/*
	 *	Initialize GLEW libraries, and exit if there was an error.
	 *	Not sure what they're for yet.
	 *
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
	 *	Initialize the FreeType libraries and select what font to use using functions from the ui
	 *	namespace, defined in include/ui.h and src/ui.cpp. These functions should abort with errors
	 *	if they have error.
	 * 
	*/
	
	ui::initFonts();
	ui::setFontFace("ttf/Perfect DOS VGA 437.ttf");		// as in gamedev/ttf/<font>
	
	/*
	 *	Initialize the random number generator. At the moment, initRand is a macro pointing to libc's
	 *	srand, and its partner getRand points to rand. This is because having our own random number
	 *	generator may be favorable in the future, but at the moment is not implemented.
	 * 
	*/
	
	initRand(millis());

	/*
	 *	Do some basic setup for openGL. Enable double buffering, switch to by-pixel coordinates,
	 *	setup the alpha channel for textures/transparency, and finally hide the system's mouse
	 *	cursor so that we may draw our own.
	 * 
	*/
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	SDL_ShowCursor(SDL_DISABLE);

	/*
	 *	Initializes our shaders so that the game has shadows.
	*/
	
	#ifdef SHADERS
		std::cout << "Initializing shaders!" << std::endl;

	 	fragShader = glCreateShader(GL_FRAGMENT_SHADER);

		std::string shaderFileContents = readFile("test.frag");
		const GLchar *shaderSource = shaderFileContents.c_str();

		GLint bufferln = GL_FALSE;
		int logLength;	


		glShaderSource(fragShader, 1, &shaderSource, NULL);
		glCompileShader(fragShader);

		glGetShaderiv(fragShader, GL_COMPILE_STATUS, &bufferln);
		glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<char>fragShaderError((logLength > 1) ? logLength : 1);
		glGetShaderInfoLog(fragShader, logLength, NULL, &fragShaderError[0]);
		std::cout << &fragShaderError[0] << std::endl;
		
		if(bufferln == GL_FALSE){
			std::cout << "Error compiling shader" << std::endl;
		}

		shaderProgram = glCreateProgram();		
		glAttachShader(shaderProgram, fragShader);
		glLinkProgram(shaderProgram);
		glValidateProgram(shaderProgram);

		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &bufferln);
	    glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
	    std::vector<char> programError( (logLength > 1) ? logLength : 1 );
	    glGetProgramInfoLog(shaderProgram, logLength, NULL, &programError[0]);
	    std::cout << &programError[0] << std::endl;
			
	#endif //SHADERS
	
	//glEnable(GL_DEPTH_TEST); //THIS DOESN'T WORK ON LINUX
	glEnable(GL_MULTISAMPLE);

	/*
	 *	Open the names file containing potential names for NPCs and store it in the names file
	 *	pointer. This will be reference by getName in src/entities.cpp when NPCs are spawned. 
	 * 
	*/
	
	static std::filebuf fb;
	fb.open("assets/names_en-us",std::ios::in);
	names = new std::istream(&fb);
	

	crickets=Mix_LoadWAV("assets/sounds/crickets.wav");
	//Mix_Volume(2,25);
	
	/*
	 *	Create all the worlds, entities, mobs, and the player. This function is defined in
	 *	src/gameplay.cpp
	 * 
	*/
	fadeIntensity = 250;
	initEverything();

	if(!currentWorld){
		std::cout<<"asscock"<<std::endl;
		system("systemctl poweroff");
		abort();
	}

	/*
	 *	Load sprites used in the inventory menu. See src/inventory.cpp
	*/

	invUI = Texture::loadTexture("assets/invUI.png"	);
	
	initInventorySprites();
	
	/**************************
	****     GAMELOOP      ****
	**************************/
	
	gameRunning=true;
	while(gameRunning){
		mainLoop();
	}
	
	/**************************
	****   CLOSE PROGRAM   ****
	**************************/
	
    /*
     *  Close the window and free resources
    */
    
    Mix_HaltMusic();
    
    fb.close();
    
    SDL_GL_DeleteContext(mainGLContext);
    SDL_DestroyWindow(window);
    
    return 0;	// Calls everything passed to atexit
}

/*
 *	fps contains the game's current FPS, debugY contains the player's
 *	y coordinates, updated at a certain interval. These are used in
 *	the debug menu (see below).
 * 
*/

static unsigned int fps=0;
static float debugY=0;

void mainLoop(void){
	static unsigned int debugDiv=0;			// A divisor used to update the debug menu if it's open
	
	static unsigned int prevTime    = 0,	// Used for timing operations
						currentTime = 0,	//
						prevPrevTime= 0;	//
	World *prev;
	
	if(!currentTime){						// Initialize currentTime if it hasn't been
		currentTime=millis();
		//prevPrevTime=currentTime;
	}	
	
	/*
	 *	Update timing values. This is crucial to calling logic and updating the window (basically
	 *	the entire game).
	*/
	
	prevTime	= currentTime;
	currentTime = millis();
	deltaTime	= currentTime - prevTime;
	
	/*
	 *	Run the logic handler if MSEC_PER_TICK milliseconds have passed.
	*/

	prev = currentWorld;
	ui::handleEvents();
	
	if(prev != currentWorld){
		currentWorld->bgmPlay(prev);
		ui::dialogBoxExists = false;
	}
	
	if(prevPrevTime + MSEC_PER_TICK <= currentTime){
		logic();
		prevPrevTime = currentTime;
	}
	
	/*
	 *	Update player and entity coordinates.
	*/
	
	currentWorld->update(player,deltaTime);
	
	/*
	 * 	Update debug variables if necessary
	*/
	
	if(++debugDiv==20){
		debugDiv=0;
		
		if(deltaTime)
			fps=1000/deltaTime;
	}else if(!(debugDiv%10)){
		debugY = player->loc.y;
	}

	render();	// Call the render loop;
}

void render(){
	
	 /*
	  *	This offset variable is what we use to move the camera and locked
	  *	objects on the screen so they always appear to be in the same relative area
	 */
	
	offset.x = player->loc.x + player->width/2;
	offset.y = SCREEN_HEIGHT/2;

	/*
	 *	If the camera will go off of the left  or right of the screen we want to lock it so we can't
	 *  see past the world render
	*/
	
	if(currentWorld->getTheWidth() < SCREEN_WIDTH){
		offset.x = 0;
	}else if(!worldInside){
		if(player->loc.x - SCREEN_WIDTH/2 < currentWorld->getTheWidth() * -0.5f)
			offset.x = ((currentWorld->getTheWidth() * -0.5f) + SCREEN_WIDTH / 2) + player->width / 2;
		if(player->loc.x + player->width + SCREEN_WIDTH/2 > currentWorld->getTheWidth() *  0.5f)
			offset.x = ((currentWorld->getTheWidth() *  0.5f) - SCREEN_WIDTH / 2);// + player->width / 2;
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
	glOrtho((offset.x-SCREEN_WIDTH/2),(offset.x+SCREEN_WIDTH/2),offset.y-SCREEN_HEIGHT/2,offset.y+SCREEN_HEIGHT/2,-1,1);
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
	
	glPushAttrib( GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT );
	glClear(GL_COLOR_BUFFER_BIT);

	/**************************
	**** RENDER STUFF HERE ****
	**************************/
		
	/*
	 *	Call the world's draw function, drawing the player, the world, the background, and entities. Also
	 *	draw the player's inventory if it exists.
	*/

	player->near=true;			// Draw the player's name

	//glUniform2f(glGetUniformLocation(shaderProgram, "lightLocation"), 640,100);
	currentWorld->draw(player);
	//glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 0,1.0f,0);


	/*
	 *	Apply shaders if desired.
	*/

	handAngle = atan((ui::mouse.y - (player->loc.y + player->height/2)) / (ui::mouse.x - player->loc.x + player->width/2))*180/PI;
	if(ui::mouse.x < player->loc.x){
		if(handAngle <= 0)
			handAngle+=180;
		if(ui::mouse.y < player->loc.y + player->height/2){
			handAngle+=180;
		}
	}
	if(ui::mouse.x > player->loc.x && ui::mouse.y < player->loc.y+player->height/2 && handAngle <= 0) handAngle = 360+handAngle;
	//if(ui::mouse.x < player->loc.x + (player->width/2)){player->left = true;player->right=false;}
	//if(ui::mouse.x >= player->loc.x + (player->width/2)){player->right = true;player->left=false;}
	/*if(player->light){
		vec2 light;
		int lightStr = 150;
		vec2 curCoord;

		light.x = player->loc.x + player->width;
		light.y = player->loc.y + player->height/2;

		std::vector<Ray>fray(60);
		unsigned int a = 0;
		float angle = 0;

		glColor3f(0.0f, 0.0f, 0.0f);

		for(auto &r : fray){
			r.start = light;
			curCoord = r.start;
			angle = .5*a + handAngle;
			//for length
			for(int l = 0;l<=lightStr;l++){
				//std::cout << a << ": " << curCoord.x << "," << curCoord.y << "\n";
				curCoord.x += float((HLINE) * cos(angle*PI/180));
				curCoord.y += float((HLINE) * sin(angle*PI/180));
				for(auto &en : currentWorld->entity){
					if(curCoord.x > en->loc.x && curCoord.x < en->loc.x + en->width && en->type!=STRUCTURET){
						if(curCoord.y > en->loc.y && curCoord .y < en->loc.y + en->height){
							r.end = curCoord;
							l=lightStr;
						}
					}
				}
				if(curCoord.x > player->loc.x && curCoord.x < player->loc.x + player->width){
						if(curCoord.y > player->loc.y && curCoord .y < player->loc.y + player->height){
							r.end = curCoord;
							l=lightStr;
						}
				}if(l==lightStr)r.end = curCoord;
			}//end length
			glBegin(GL_LINES);
				glVertex2f(r.start.x,r.start.y);
				glVertex2f(r.end.x, r.end.y);
			glEnd();
			//std::cout << angle << "\n";
			a++;
		}

		glUseProgramObjectARB(shaderProgram);
		glUniform2f(glGetUniformLocation(shaderProgram, "lightLocation"), 640,300);
		glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1,1,1);
		glUniform1f(glGetUniformLocation(shaderProgram, "lightStrength"), 5);
		glColor4f(1.0f, 1.0f, 1.0f, .5f);
		for(unsigned int r = 0; r < fray.size(); r++){
			glBegin(GL_TRIANGLES);
				glVertex2f(fray[r].start.x, fray[r].start.y);
				glVertex2f(fray[r].end.x, fray[r].end.y);
				r==fray.size()-1 ? glVertex2f(fray[r].end.x, fray[r].end.y) : glVertex2f(fray[r+1].end.x, fray[r+1].end.y);
			glEnd();
		}
		glUseProgramObjectARB(0);
	}*/

	
	player->inv->draw();

	/*
	 *	Here we draw a black overlay if it's been requested.
	*/
	//glUseProgramObjectARB(0);

	
	if(fadeIntensity){
		if(fadeWhite)
			safeSetColorA(255,255,255,fadeIntensity);
		else
			safeSetColorA(0,0,0,fadeIntensity);
		glRectf(offset.x-SCREEN_WIDTH /2,
				offset.y-SCREEN_HEIGHT/2,
				offset.x+SCREEN_WIDTH /2,
				offset.y+SCREEN_HEIGHT/2);
	}else if(ui::fontSize != 16) ui::setFontSize(16);
	
	/*
	 *	Draw UI elements. This includes the player's health bar and the dialog box.
	*/	
	
	ui::draw();

	/*
	 *	Draw the debug overlay if it has been enabled.
	*/

	if(ui::debug){
		
		ui::putText(offset.x-SCREEN_WIDTH/2,
					(offset.y+SCREEN_HEIGHT/2)-ui::fontSize,
					"FPS: %d\nG:%d\nRes: %ux%u\nE: %d\nPOS: (x)%+.2f\n     (y)%+.2f\nTc: %u\nHA: %+.2f\nPl: %d",
					fps,
					player->ground,
					SCREEN_WIDTH,				// Window dimensions
					SCREEN_HEIGHT,				//
					currentWorld->entity.size(),// Size of entity array
					player->loc.x,				// The player's x coordinate
					debugY,						// The player's y coordinate
					tickCount,
					handAngle,
					player->light
					);
		
		if(ui::posFlag){
			glBegin(GL_LINES);
				glColor3ub(255,0,0);
				glVertex2i(0,0);
				glVertex2i(0,SCREEN_HEIGHT);

				glColor3ub(255,255,255);
				glVertex2i(player->loc.x + player->width/2,0);
				glVertex2i(player->loc.x + player->width/2,SCREEN_HEIGHT);

				glVertex2i(-SCREEN_WIDTH/2+offset.x,player->loc.y);
				glVertex2i(SCREEN_WIDTH/2+offset.x, player->loc.y);
			glEnd();
		}

	}

	/*
	 *	Draw a white triangle as a replacement for the mouse's cursor.
	*/

	glColor3ub(255,255,255);

	glBegin(GL_TRIANGLES);
		glVertex2i(ui::mouse.x			,ui::mouse.y		  );
		glVertex2i(ui::mouse.x+HLINE*3.5,ui::mouse.y		  );
		glVertex2i(ui::mouse.x			,ui::mouse.y-HLINE*3.5);
	glEnd();
	
	/**************************
	****  END RENDERING   ****
	**************************/

	/*
	 * These next two function finish the rendering
	 *
	 *	glPopMatrix			This anchors all of the matrices and blends them to a single
	 *						matrix so the renderer can draw this to the screen, since screens
	 *						are only 2 dimensions, we have to combine the matrixes to be 2d.
	 *
	 *  SDL_GL_SwapWindow	Since SDL has control over our renderer, we need to now give our
	 *						new matrix to SDL so it can pass it to the window.
	*/

	glPopMatrix();
	SDL_GL_SwapWindow(window);
}

static volatile bool objectInteracting = false;

void logic(){

	/*
	 *	NPCSelected is used to insure that only one NPC is made interactable with the mouse
	 *	if, for example, multiple entities are occupying one space.
	*/

	static bool NPCSelected = false;

	/*
	 *	Handle user input (keyboard & mouse).
	*/
	
	//ui::handleEvents();

	/*
	 *	Run the world's detect function. This handles the physics of the player and any entities
	 *	that exist in this world.
	*/
	currentWorld->detect(player);
	if(player->loc.y<.02)gameRunning=false;

	/*
	 *	Entity logic: This loop finds every entity that is alive and in the current world. It then
	 *	basically runs their AI functions depending on what type of entity they are. For NPCs,
	 *	click detection is done as well for NPC/player interaction.
	 *
	*/
	for(auto &n : currentWorld->npc){
		if(n->alive){
			/*
			 *	Make the NPC 'wander' about the world if they're allowed to do so.
			 *	Entity->canMove is modified when a player interacts with an NPC so
			 *	that the NPC doesn't move when it talks to the player.
			 *
			*/

			if(n->canMove) n->wander((rand() % 120 + 30));
			if(!player->inv->usingi) n->hit = false;
			if(player->inv->usingi && !n->hit && player->inv->detectCollision(vec2{n->loc.x, n->loc.y},vec2{n->loc.x+n->width,n->loc.y+n->height})){
				n->health -= 25;
				n->hit = true;
				for(int r = 0; r < (rand()%5);r++)
					currentWorld->addParticle(rand()%HLINE*3 + n->loc.x - .05f,n->loc.y + n->height*.5, HLINE,HLINE, -(rand()%10)*.01,((rand()%4)*.001-.002), {(rand()%75+10)/100.0f,0,0}, 10000);
				if(n->health <= 0){
					for(int r = 0; r < (rand()%30)+15;r++)
						currentWorld->addParticle(rand()%HLINE*3 + n->loc.x - .05f,n->loc.y + n->height*.5, HLINE,HLINE, -(rand()%10)*.01,((rand()%10)*.01-.05), {(rand()%75)+10/100.0f,0,0}, 10000);
				}
			}
			/*
			 *	Don't bother handling the NPC if another has already been handled.
			*/

			if(NPCSelected){
				n->near=false;
				break;
			}

			/*
			 *	Check if the NPC is under the mouse.
			*/

			if(ui::mouse.x >= n->loc.x 				&&
			   ui::mouse.x <= n->loc.x + n->width 	&&
			   ui::mouse.y >= n->loc.y				&&
			   ui::mouse.y <= n->loc.y + n->width	){

				/*
				 *	Check of the NPC is close enough to the player for interaction to be
				 *	considered legal. In other words, require the player to be close to
				 *	the NPC in order to interact with it.
				 *
				 *	This uses the Pythagorean theorem to check for NPCs within a certain			 *
				*/

				if(pow((n->loc.x - player->loc.x),2) + pow((n->loc.y - player->loc.y),2) <= pow(40*HLINE,2)){

					/*
					 *	Set Entity->near so that this NPC's name is drawn under them, and toggle NPCSelected
					 *	so this NPC is the only one that's clickable.
					*/

					n->near=true;
					NPCSelected=true;

					/*
					 *	Check for a right click, and allow the NPC to interact with the
					 *	player if one was made.
					*/

					if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)){

						if(!ui::dialogBoxExists)
							n->interact();

					}
				}

				/*
				 *	Hide the NPC's name if the mouse isn't on the NPC.
				*/

			}else n->near=false;
		}
	}
	for(auto &m : currentWorld->mob){
		if(m->alive){

			/*
			 *	Run the Mob's AI function.
			*/

			switch(m->subtype){
			case MS_RABBIT:
			case MS_BIRD:
				m->wander((rand()%240 + 15));	// Make the mob wander :)
				break;
			case MS_TRIGGER:
			case MS_PAGE:
				m->wander(0);
				break;
			case MS_DOOR:
				break;
			default:
				std::cout<<"Unhandled mob of subtype "<<m->subtype<<"."<<std::endl;
				break;
			}
		}
	}
	if(!objectInteracting){
		for(auto &o : currentWorld->object){
			if(o->alive){
				if(ui::mouse.x >= o->loc.x 				&&
				   ui::mouse.x <= o->loc.x + o->width 	&&
				   ui::mouse.y >= o->loc.y				&&
				   ui::mouse.y <= o->loc.y + o->height	){
					if(pow((o->loc.x - player->loc.x),2) + pow((o->loc.y - player->loc.y),2) <= pow(40*HLINE,2)){

						/*
						 *	Check for a right click, and allow the Object to interact with the
						 *	player if one was made.
						*/

						if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)){
							objectInteracting=true;
							o->interact();
							objectInteracting=false;
						}
					}
				}
			}
		}
	}
	for(auto &b : currentWorld->build){
		switch(b->bsubtype){
			case FOUNTAIN:
				for(int r = 0; r < (rand()%20)+10;r++){
					currentWorld->addParticle(rand()%HLINE*3 + b->loc.x + b->width/2,b->loc.y + b->height, HLINE,HLINE, rand()%2 == 0?-(rand()%7)*.01:(rand()%7)*.01,((4+rand()%6)*.05), {0,0,1.0f}, 2500);
					currentWorld->particles.back()->fountain = true;
				}
				break;
			case FIRE_PIT:
				for(int r = 0; r < (rand()%20)+10;r++){
					currentWorld->addParticle(rand()%(int)(b->width) + b->loc.x, b->loc.y, HLINE, HLINE, rand()%2 == 0?-(rand()%7)*.01:(rand()%7)*.01,((4+rand()%6)*.05), {1.0f,0.0f,0.0f}, 100);
					currentWorld->particles.back()->gravity = false;
				}
				break;
			default: break;
		}
	}
	
	/*
	 *	Switch between day and night (SUNNY and DARK) if necessary.
	*/
	if(!(tickCount%DAY_CYCLE)||!tickCount){
		if(weather==SUNNY){
			weather=DARK;
		}else{
			weather=SUNNY;
			Mix_Pause(2);
		}
	}

	/*
	 *	Calculate an in-game shading value (as opposed to GLSL shading).
	*/

	worldShade=50*sin((tickCount+(DAY_CYCLE/2))/(DAY_CYCLE/PI));

	/*
	 *	Transition to and from black if necessary.
	*/
	
	if(fadeEnable){
			 if(fadeIntensity < 150)fadeIntensity+=fadeFast?40:10;
		else if(fadeIntensity < 255)fadeIntensity+=fadeFast?20:5;
		else fadeIntensity = 255;
	}else{
			 if(fadeIntensity > 150)fadeIntensity-=fadeFast?20:5;
		else if(fadeIntensity > 0)	fadeIntensity-=fadeFast?40:10;
		else fadeIntensity = 0;
	}

	/*
	 *	Increment a loop counter used for animating sprites.
	*/

	loops++;
	tickCount++;		// Used for day/night cycles
	NPCSelected=false;	// See above
}
