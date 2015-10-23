#include <cstdio> // fopen
#include <chrono> // see millis()

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

static GLuint  bgImage;

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

bool gameRunning = true;

/*
 *	currentWorld 	-	This is a pointer to the current world that the player
 * 				is in. Most drawing/entity handling is done through this
 * 				variable. This should only be changed when layer switch
 * 				buttons are pressed (see src/ui.cpp), or when the player
 * 				enters a Structure/Indoor World (see src/ui.cpp again).
 * 
 *	player		-	This points to a Player object, containing everything for
 * 				the player. Most calls made with currentWorld require a
 * 				Player object as an argument, and glOrtho is set based
 * 				off of the player's coordinates. This is probably the one
 * 				Entity-derived object that is not pointed to in the entity
 * 				array.
 * 
 *	entity		 -	Contains pointers to 'all' entities that have been created in
 * 				the game, including NPCs, Structures, and Mobs. World draws
 * 				and entity handling done by the world cycle through entities
 * 				using this array. Entities made that aren't added to this
 * 				array probably won't be noticable by the game.
 * 
 *	npc		 -	An array of all NPCs in the game. It's not exactly clear how
 * 				NPC initing is done, their constructed in this array, then set
 * 				to be pointed to by entity, then maybe spawned with Entity->spawn().
 * 				See src/entities.cpp for more.
 * 				This variable might be referenced as an extern in other files.
 * 
 *	build		 -	An array of all Structures in the game. Entries in entity point to
 *				these, allowing worlds to handle the drawing and stuff of these.
 * 				See src/entities.cpp for more.
 * 
 *	mob		 -	An array of all Mobs in the game, entity entries should point to these
 * 				so the world can take care of them. See src/entities.cpp for more.
 * 
*/

World  						*currentWorld=NULL;
Player 						*player;
std::vector<Entity *	>	 entity;
std::vector<Structures *>	 build;
std::vector<Mob			>	 mob;

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
 *	names is used to open a file containing all possible NPC names. It is externally
 *	referenced in src/entities.cpp for getting random names.
 *
*/

FILE *names;

/*
 *	These variables are used by SDL_mixer to create sound.
 *	horn is not currently used, although it may be set.
 * 
*/

Mix_Music *music;
Mix_Chunk *horn;

/*
 *	loops is used for texture animation. It is believed to be passed to entity
 *	draw functions, although it may be externally referenced instead.
 * 
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

/*
 *	millis
 * 
 *	We've encountered many problems when attempting to create delays for triggering
 *	the logic function. As a result, we decided on using the timing libraries given
 *	by <chrono> in the standard C++ library. This function simply returns the amount
 *	of milliseconds that have passed sine the epoch.
 * 
*/
unsigned int millis(void){
	std::chrono::system_clock::time_point now=std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

/*******************************************************************************
 * MAIN ************************************************************************
 *******************************************************************************/
int main(int argc, char *argv[]){
	/*
	 *	Initialize GLEW libraries, and exit if there was an error.
	 *	Not sure what they're for yet.
	 * 
	*/
	
	if(glewInit() < 0){
		std::cout << "GLEW was not able to initialize! Error: " << std::endl;
		return -1;
	}
	
	/*
	 *	(Attempt to) Initialize SDL libraries so that we can use SDL facilities and eventually
	 *	make openGL calls. Exit if there was an error.
	 * 
	*/
	
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0){
		std::cout << "SDL was not able to initialize! Error: " << SDL_GetError() << std::endl;
		return -1;
	}
	
	// Run SDL_Quit when main returns
    atexit(SDL_Quit);
    
    /*
     *	(Attempt to) Initialize SDL_image libraries with IMG_INIT_PNG so that we can load PNG
     *	textures for the entities and stuff.
     * 
    */
    
    if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)){
		std::cout << "Could not init image libraries! Error: " << IMG_GetError() << std::endl;
		return -1;
	}
	
	// Run IMG_Quit when main returns
	atexit(IMG_Quit);
	
	/*
	 *	(Attempt to) Initialize SDL_mixer libraries for loading and playing music/sound files.
	 * 
	*/
	
	if(Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
		std::cout << "SDL_mixer could not initialize! Error: " << Mix_GetError() << std::endl;
	}
	
	// Run Mix_Quit when main returns
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
	 *	TODO - Initialize shaders n' stuff
	*/
	
	/*
	GLuint fragShader;
	GLuint shaderProgram;6da

	const GLchar *shaderSource = "shader.frag";
	GLint bufferln = GL_FALSE;

	shaderProgram = glCreateProgram();
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		
	glShaderSource(fragShader, 1, shaderSource, NULL);
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &bufferln);
	
	if(bufferln == GL_TRUE){
		std::cout << "Error compiling shader" << std::endl;
	}
	
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);
	glValidateProgram(shaderProgram);
	*/
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_MULTISAMPLE);

	/*
	 *	Open the names file containing potential names for NPCs and store it in the names file
	 *	pointer. This will be reference by getName in src/entities.cpp when NPCs are spawned. 
	 * 
	*/
	
	names = fopen("assets/names_en-us", "r+");
	
	/*
	 *	Create all the worlds, entities, mobs, and the player. This function is defined in
	 *	src/gameplay.cpp
	 * 
	*/
	
	initEverything();
	
	/*
	 *	Open a test background music file and sound. The background music is then played indefinitely
	 *	while the sound is never referenced again.
	 * 
	*/
	
	music = Mix_LoadMUS("assets/BennyHillTheme.wav");			// as in gamedev/assets/<sound>
	horn  = Mix_LoadWAV("assets/air-horn-club-sample_1.wav");	//
	
	Mix_VolumeMusic(15);		// Set the volume
	Mix_PlayMusic( music, -1 );	// Play music forever
	
	/*
	 *	Load a temporary background image.
	*/
	
	bgImage=Texture::loadTexture("assets/bg.png");
	
	/*
	 *	Load sprites used in the inventory menu. See src/inventory.cpp
	*/
	
	initInventorySprites();
	
	/**************************
	****     GAMELOOP      ****
	**************************/
	
	while(gameRunning){
		mainLoop();
	}
	
	/**************************
	****   CLOSE PROGRAM   ****
	**************************/
	
    /*
     *  Close the window and free resources
    */
    
    fclose(names);
    
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
						prevPrevTime= 0;	// shit
						
	unsigned int i;							// Used for `for` loops
	
	if(!currentTime){						// Initialize currentTime if it hasn't been
		currentTime=millis();
		prevPrevTime=currentTime;
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
	
	if(prevPrevTime + MSEC_PER_TICK >= currentTime){
		logic();
		prevPrevTime = currentTime;
	}

	/*
	 *	Update player and entity coordinates.
	*/
	
	player->loc.y+= player->vel.y				*deltaTime;
	player->loc.x+=(player->vel.x*player->speed)*deltaTime;
	
	for(int i=0;i<=entity.size();i++){
		
		entity[i]->loc.x += entity[i]->vel.x * deltaTime;
		entity[i]->loc.y += entity[i]->vel.y * deltaTime;
		
			 if(entity[i]->vel.x<0)entity[i]->left=true;
		else if(entity[i]->vel.x>0)entity[i]->left=false;
	}
	
	/*
	 * 	Update debug variables if necessary
	*/
	
	if(++debugDiv==20){
		debugDiv=0;
		
		fps=1000/deltaTime;
		
	}else if(!(debugDiv%10)){
		debugY = player->loc.y;
	}
	
	render();	// Call the render loop
}

void render(){
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

	glMatrixMode(GL_PROJECTION); 					//set the matrix mode as projection so we can set the ortho size and the camera settings later on
	glPushMatrix(); 								//push the  matrix to the top of the matrix stack
	glLoadIdentity(); 								//replace the entire matrix stack with the updated GL_PROJECTION mode
	glOrtho(player->loc.x-SCREEN_WIDTH/2,player->loc.x+SCREEN_WIDTH/2,0,SCREEN_HEIGHT,-1,1);
	glMatrixMode(GL_MODELVIEW); 					//set the matrix to modelview so we can draw objects
	glPushMatrix(); 								//push the  matrix to the top of the matrix stack
	glLoadIdentity(); 								//replace the entire matrix stack with the updated GL_MODELVIEW mode
	glPushMatrix();									//basically here we put a blank canvas (new matrix) on the screen to draw on
	
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
	 *  Draw a temporary background image
	*/

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,bgImage);
	
	glBegin(GL_QUADS);
		glTexCoord2i(0,1);glVertex2i(-SCREEN_WIDTH*2,0);
		glTexCoord2i(1,1);glVertex2i( SCREEN_WIDTH*2,0);
		glTexCoord2i(1,0);glVertex2i( SCREEN_WIDTH*2,SCREEN_HEIGHT);
		glTexCoord2i(0,0);glVertex2i(-SCREEN_WIDTH*2,SCREEN_HEIGHT);
	glEnd();
	
	glDisable(GL_TEXTURE_2D);
	
	/*
	 *	Call the world's draw function, drawing the player, the world, and entities. Also
	 *	draw the player's inventory if it exists.
	*/

	player->near=true;			// Draw the player's name
	
	currentWorld->draw(player);
	
	player->inv->draw();

	/*
	 *	Draw UI elements. As of 10/20/2015 this includes the player's health bar and the dialog box.
	*/
	
	ui::draw();

	/*
	 *	Draw the debug overlay if it has been enabled.
	*/

	if(ui::debug){
		
		ui::setFontSize(16);
		
		ui::putText(player->loc.x-SCREEN_WIDTH/2,
					SCREEN_HEIGHT-ui::fontSize,
					"FPS: %d\nG:%d\nRes: %ux%u\nE: %d\nPOS: (x)%+.2f\n     (y)%+.2f\nQc: %u\nTS:%d\n",
					fps,
					player->ground,
					SCREEN_WIDTH,				// Window dimensions
					SCREEN_HEIGHT,				//
					entity.size(),				// Size of entity array
					player->loc.x,				// The player's x coordinate
					debugY,						// The player's y coordinate
					player->qh.current.size(),	// Active quest count
					player->tex->texState
					);
					
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

void logic(){
	/*
	 *	Handle user input (keyboard & mouse).
	*/
	
	ui::handleEvents();
	
	/*
	 *	Run the world's detect function. This handles the physics of the player and any entities
	 *	that exist in this world.
	*/
	
	currentWorld->detect(player);
	
	/*
	 *	Entity logic: This loop finds every entity that is alive and in the current world. It then
	 *	basically runs their AI functions depending on what type of entity they are. For NPCs,
	 *	click detection is done as well for NPC/player interaction.
	 * 
	*/
	 //std::cout << "Game Loop: "<< loops << std::endl;
	
	for(int i=0;i<=entity.size();i++){
		
		/*
		 *	Check if the entity is in this world and is alive.
		*/
		
		if(entity[i]->inWorld==currentWorld&&entity[i]->alive){
			
			/*
			 *	Switch on the entity's type and handle them accordingly.
			*/
			
			switch(entity[i]->type){
				
			case NPCT:	// Handle NPCs
			
				/*
				 *	Make the NPC 'wander' about the world if they're allowed to do so.
				 *	Entity->canMove is modified when a player interacts with an NPC so
				 *	that the NPC doesn't move when it talks to the player.
				 * 
				*/
			
				if(entity[i]->canMove)
					entity[i]->wander((rand() % 120 + 30), &entity[i]->vel);
				
				/*
				 *	Check if the NPC is under the mouse.
				*/
				
				if(ui::mouse.x >= entity[i]->loc.x 						&&
				   ui::mouse.x <= entity[i]->loc.x + entity[i]->width 	&&
				   ui::mouse.y >= entity[i]->loc.y						&&
				   ui::mouse.y <= entity[i]->loc.y + entity[i]->width	){
					   
					/*
					 *	Check of the NPC is close enough to the player for interaction to be
					 *	considered legal. In other words, require the player to be close to
					 *	the NPC in order to interact with it. 
					 * 
					 *	This uses the Pythagorean theorem to check for NPCs within a certain
					 *	radius (40 HLINEs) of the player's coordinates.
					 * 
					*/   
					
					if(pow((entity[i]->loc.x - player->loc.x),2) + pow((entity[i]->loc.y - player->loc.y),2) <= pow(40*HLINE,2)){
						
						/*
						 *	Set Entity->near so that this NPC's name is drawn under them.
						*/
						
						entity[i]->near=true;
						
						/*
						 *	Check for a right click, and allow the NPC to interact with the
						 *	player if one was made.
						*/
						
						if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)){
							
							entity[i]->interact();
							//Mix_PlayChannel( -1, horn, 0);	// Audio feedback
							
						}
					}
					
				/*
				 *	Hide the NPC's name if the mouse isn't on the NPC.
				*/
				
				}else entity[i]->near=false;
				
				break;	// End case NPCT
				
			case MOBT:	// Handle Mobs
			
				/*
				 *	Run the Mob's AI function.
				*/
				
				entity[i]->wander((rand()%240 + 15),&entity[i]->vel);	// Make the mob wander
				
				break;	// End case MOBT
				
			default:break;
			
			}
		}
	}
	
	/*
	 *	Increment a loop counter used for animating sprites.
	*/
	
	loops++;
}
