#include <common.h>
#include <world.h>
#include <ui.h>
#include <entities.h>
#include <cstdio>
#include <chrono>

SDL_Window    *window = NULL;
SDL_Surface   *renderSurface = NULL;
SDL_GLContext  mainGLContext = NULL;

bool gameRunning = true;

World *currentWorld=NULL;
Player *player;

void logic();
void render();

unsigned int millis(void){
	std::chrono::system_clock::time_point now=std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

int main(int argc, char *argv[]){
	// Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO)){
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

	ui::initFonts();
	ui::setFontFace("ttf/VCR_OSD_MONO_1.001.ttf");
	ui::setFontSize(16);
	initRand(millis()); // fix

	glViewport(0,0,SCREEN_WIDTH, SCREEN_HEIGHT);
	glClearColor(.3,.5,.8,0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	SDL_ShowCursor(SDL_DISABLE);
	
	/**************************
	****     GAMELOOP      ****
	**************************/

	World *test=new World(SCREEN_WIDTH/2);
	test->addLayer(400);
	currentWorld=test;
	player=new Player();
	player->spawn(0,100);

	while(gameRunning){
		render();
		logic();
	}
	
	/**************************
	****  CLOSE PROGRAM    ****
	**************************/
	
    //closes the window and frees resources
    SDL_GL_DeleteContext(mainGLContext);
    SDL_DestroyWindow(window);
    return 0;
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
	glClear(GL_COLOR_BUFFER_BIT); 					//clear the matrix on the top of the stack

	/**************************
	**** RENDER STUFF HERE ****
	**************************/

	currentWorld->draw(&player->loc);
	player->draw();
	//ui::putString(0,0,"Hello");

	/**************************
	****  CLOSE THE LOOP   ****
	**************************/

	glPopMatrix(); 									//take the matrix(s) off the stack to pass them to the renderer
	SDL_GL_SwapWindow(window); 						//give the stack to SDL to render it
}

void logic(){
	ui::handleEvents();
	currentWorld->detect(&player->loc,&player->vel,player->width);
	player->loc.y+=player->vel.y;
	player->loc.x+=player->vel.x;
	std::cout<<"("<<player->loc.x<<","<<player->loc.y<<")"<<std::endl;
}	
