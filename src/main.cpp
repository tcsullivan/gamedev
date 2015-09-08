#include <common.h>

SDL_Window    *window = NULL;
SDL_Surface   *renderSurface = NULL;
SDL_GLContext  mainGLContext = NULL;

bool gameRunning = true;

UIClass ui;
Window win;

int main(int argc,char **argv){
    //runs start-up procedures
    if(!SDL_Init(SDL_INIT_VIDEO)){
    	atexit(SDL_Quit);
    	if(!(IMG_Init(IMG_INIT_PNG)&IMG_INIT_PNG)){
			std::cout<<"Could not init image libraries!\n"<<std::endl;
			return -1;
		}
		atexit(IMG_Quit);
		//Turn on double Buffering
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        //create the window
        window = SDL_CreateWindow("Independent Study v.0.1 alpha", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
                                  #ifdef FULLSCREEN
                                  | SDL_WINDOW_FULLSCREEN
                                  #endif // FULLSCREEN
                                  );
        if(window){
        	//set OpenGL context
            mainGLContext = SDL_GL_CreateContext(window);
            if(mainGLContext == NULL){
            	std::cout << "The OpenGL context failed to initialize! Error: " << SDL_GetError() << std::endl;
            }
		}else{
			std::cout << "The window failed to generate! Error: " << SDL_GetError() << std::endl;
        	return -1;
        }
    }else{
		std::cout << "SDL was not able to initialize! Error: " << SDL_GetError() << std::endl;
		return -1;
	}
	
	
	/**************************
	****     GAMELOOP      ****
	**************************/
	
	win.setupRender();
	while(gameRunning){
		ui.handleEvents();
		win.render();
	}
	
	/**************************
	****  CLOSE PROGRAM    ****
	**************************/
	
	
    //closes the window and frees resources
    SDL_GL_DeleteContext(mainGLContext);
    SDL_DestroyWindow(window);
    return 0;
}
