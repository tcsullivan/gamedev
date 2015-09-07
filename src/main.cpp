#include <iostream>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
//#define FULLSCREEN

SDL_Window    *window = NULL;
SDL_Surface   *renderSurface = NULL;
SDL_GLContext  mainGLContext = NULL;

const float sh = SCREEN_HEIGHT;
const float sw = SCREEN_WIDTH;
bool gameRunning = true;

int main(int argc,char **argv){
	SDL_Event e;
    //runs startup procedures
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
		std::cout << "SDL was not able to initialize! Error: " << SDL_GetError() << std::endl;
	}else{
		atexit(SDL_Quit);
		//Turn on double Buffering
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        //create the window
        window = SDL_CreateWindow("Sword Swinger", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
                                  #ifdef FULLSCREEN
                                  | SDL_WINDOW_FULLSCREEN
                                  #endif // FULLSCREEN
                                  );
        if(window == NULL){
        	std::cout << "The window failed to generate! Error: " << SDL_GetError() << std::endl;
        }else{
            //set opengl context
            mainGLContext = SDL_GL_CreateContext(window);
            if(mainGLContext == NULL){
            	std::cout << "The OpenGL context failed to initialize! Error: " << SDL_GetError() << std::endl;
            }
		}
    }
	// main loop
	glClearColor(1,1,1,0);
	while(gameRunning){
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_UpdateWindowSurface(window);
		while(SDL_PollEvent(&e)){
			if(e.type==SDL_QUIT)
				gameRunning=false;
		}
	}
    //closes the window and frees resources
    SDL_GL_DeleteContext(mainGLContext);
    SDL_DestroyWindow(window);
    return 0;
}
