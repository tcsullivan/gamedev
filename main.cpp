#include <common.h>
#include <world.h>
#include <ui.h>
#include <entities.h>
#include <cstdio>
#include <chrono>

#define TICKS_PER_SEC 20
#define MSEC_PER_TICK (1000/TICKS_PER_SEC)

SDL_Window    *window = NULL;
SDL_Surface   *renderSurface = NULL;
SDL_GLContext  mainGLContext = NULL;

bool gameRunning = true;

World *currentWorld=NULL;
Player *player;

std::vector<Entity*>entity;
std::vector<NPC>npc;
std::vector<Structures *>build;

int mx, my;
FILE* names;

Mix_Music *music;
Mix_Chunk *horn;

extern void initEverything(void);

void logic();
void render();
void mainLoop(void);

unsigned int millis(void){
	std::chrono::system_clock::time_point now=std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

int main(int argc, char *argv[]){
	// Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0){
		std::cout << "SDL was not able to initialize! Error: " << SDL_GetError() << std::endl;
		return -1;
	}
    atexit(SDL_Quit);
    // Initialize SDL_image
    if(!(IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG)&(IMG_INIT_PNG|IMG_INIT_JPG))){
		std::cout<<"Could not init image libraries!\n"<<std::endl;
		return -1;
	}
	atexit(IMG_Quit);

	 //Initialize SDL_mixer 
	if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ){
		std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
	}
	atexit(Mix_Quit);

	// Turn on double buffering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // Create the window
    window = SDL_CreateWindow("Independent Study v.0.2 alpha", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
                              #ifdef FULLSCREEN
                              | SDL_WINDOW_FULLSCREEN
                              #endif // FULLSCREEN
                              );
    if(window==NULL){
		std::cout << "The window failed to generate! Error: " << SDL_GetError() << std::endl;
		std::cout << "Window address: "<<window<<std::endl;
        return -1;
    }
    //set OpenGL context
    mainGLContext = SDL_GL_CreateContext(window);
    if(mainGLContext == NULL){
		std::cout << "The OpenGL context failed to initialize! Error: " << SDL_GetError() << std::endl;
        return -1;
    }

	ui::initFonts();									// Initialize text rendering with a font from ttf/
	ui::setFontFace("ttf/Perfect DOS VGA 437.ttf");
	initRand(millis());									// Initialize the random number generator with millis()

	glViewport(0,0,SCREEN_WIDTH, SCREEN_HEIGHT);		// Switch to pixel-based rendering, not coordinates (the -1 to 1 stuff)
	glEnable(GL_BLEND);									// Allow transparency
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	SDL_ShowCursor(SDL_DISABLE);						// Hide mouse cursor so we can draw our own

	//************************************************************************//
	//	WORLD GENERATION STUFF												  //
	//************************************************************************//

	names = fopen("assets/names_en-us", "r+");	// Open the names file
	
	initEverything();							// Run world maker thing in src/gameplay.cpp
	
	/**************************
	****     GAMELOOP      ****
	**************************/
	
	//Load music
	music = Mix_LoadMUS("assets/BennyHillTheme.wav");
	horn = Mix_LoadWAV("assets/air-horn-club-sample_1.wav");
	if( music == NULL ){
		printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
	}
	Mix_PlayMusic( music, -1 );
	
	while(gameRunning){
		mainLoop();
	}
	
	/**************************
	****  CLOSE PROGRAM    ****
	**************************/
	
    //closes the window and frees resources
    fclose(names);
    SDL_GL_DeleteContext(mainGLContext);
    SDL_DestroyWindow(window);
    return 0;
}

static unsigned int fps=0;
static float debugY=0;

unsigned int deltaTime=0;

void mainLoop(void){
	static unsigned int debugDiv=0;
	static unsigned int tickCount = 0,
						prevTime    = 0,
						currentTime = 0,
						deltatime   = 0;
	unsigned int i;
	
	if(!currentTime)currentTime=millis();
	prevTime = currentTime;
	currentTime = millis();
	deltatime = currentTime - prevTime;
	deltaTime = deltatime;
	
	if(prevTime + MSEC_PER_TICK >= millis()){
		logic();
		prevTime = millis();
	}

	player->loc.y+=player->vel.y*deltatime;
	player->loc.x+=(player->vel.x*player->speed)*deltatime;
	for(int i=0;i<=entity.size();i++){
		entity[i]->loc.x += entity[i]->vel.x * deltatime;
		entity[i]->loc.y += entity[i]->vel.y * deltatime;
		if(entity[i]->vel.x<0)entity[i]->left=true;
		if(entity[i]->vel.x>0)entity[i]->left=false;
	}
	
	if(++debugDiv==20){
		fps=1000/deltatime;
		debugDiv=0;
	}else if(!(debugDiv%10)){
		debugY = player->loc.y;
	}
	
	render();
}

void render(){
													//a matrix is a blank canvas for the computer to draw on, the matrices are stored in a "stack"
													//GL_PROJECTION has 2 matrices
													//GL_MODELVIEW has 32 matrices
	glMatrixMode(GL_PROJECTION); 					//set the matrix mode as projection so we can set the ortho size and the camera settings later on
	glPushMatrix(); 								//push the  matrix to the top of the matrix stack
	glLoadIdentity(); 								//replace the entire matrix stack with the updated GL_PROJECTION mode
	glOrtho(player->loc.x-SCREEN_WIDTH/2,player->loc.x+SCREEN_WIDTH/2,0,SCREEN_HEIGHT,-1,1);
	glMatrixMode(GL_MODELVIEW); 					//set the matrix to modelview so we can draw objects
	glPushMatrix(); 								//push the  matrix to the top of the matrix stack
	glLoadIdentity(); 								//replace the entire matrix stack with the updated GL_MODELVIEW mode
	glPushMatrix();									//basically here we put a blank canvas (new matrix) on the screen to draw on
	glPushAttrib( GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT );
	glClear(GL_COLOR_BUFFER_BIT); 					//clear the matrix on the top of the stack

	/**************************
	**** RENDER STUFF HERE ****
	**************************/

	currentWorld->draw(&player->loc);	// Draw the world around the player
	glColor3ub(0,0,0);
	player->near=true;
	player->draw();						// Draw the player
	player->inv->draw();

	ui::draw();							// Draw any UI elements if they need to be

	if(ui::debug){
		ui::setFontSize(16);
		ui::putText(player->loc.x-SCREEN_WIDTH/2,SCREEN_HEIGHT-ui::fontSize,"FPS: %d\nG:%d\nRes: %ux%u\nE: %d\nPOS: (x)%+.2f\n     (y)%+.2f\nQc: %u",
					fps,player->ground,SCREEN_WIDTH,SCREEN_HEIGHT,entity.size(),player->loc.x,debugY,player->qh.current.size());
	}

	/**************************
	****  CLOSE THE LOOP   ****
	**************************/
	mx = ui::mouse.x + player->loc.x;
	my = ui::mouse.y;
	my = 720 - my;
	mx -= (SCREEN_WIDTH/2);
	glColor3ub(255,255,255);
	glBegin(GL_TRIANGLES);
		glVertex2i(mx,my);
		glVertex2i(mx+HLINE*3.5,my);
		glVertex2i(mx,my-HLINE*3.5);
	glEnd();

	glPopMatrix(); 									//take the matrix(s) off the stack to pass them to the renderer
	SDL_GL_SwapWindow(window); 						//give the stack to SDL to render it
}

void logic(){
	ui::handleEvents();
	currentWorld->detect(player);
	for(int i=0;i<=entity.size();i++){
		if(entity[i]->alive&&entity[i]->type == NPCT){
			entity[i]->wander((rand()%120 + 30), &entity[i]->vel);
			if( pow((entity[i]->loc.x - player->loc.x),2) + pow((entity[i]->loc.y - player->loc.y),2) <= pow(40*HLINE,2)){
				if(mx >= entity[i]->loc.x && mx <= entity[i]->loc.x + entity[i]->width && my >= entity[i]->loc.y && my <= entity[i]->loc.y + entity[i]->width){
					entity[i]->near=true;
					if(SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(SDL_BUTTON_RIGHT)){
						entity[i]->interact();
						std::cout <<"["<<i<<"] -> "<< entity[i]->name << ", " << (std::string)(entity[i]->gender == MALE ? "Male" : "Female") << std::endl;
						//Mix_PlayChannel( -1, horn, 0);
					}
				}else entity[i]->near=false;
			}
		}
	}
}
