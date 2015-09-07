#include <iostream>
#include "SDL.h"
#include "SDL_opengl.h"

bool gameRunning = true;

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
//#define FULLSCREEN

    //window be rendered to
    SDL_Window* window = NULL;
    SDL_Surface* renderSurface = NULL;
    SDL_GLContext mainGLContext = NULL;
    SDL_Renderer* gRenderer = NULL;

    const float sh = SCREEN_HEIGHT;
    const float sw = SCREEN_WIDTH;

int main(int argc, char** argv){
    //runs startup procedures
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
            std::cout << "SDL was not able to initialize! Error: " << SDL_GetError() << std::endl;
        }else{
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
                }else{
                    //get window surface
                    renderSurface = SDL_GetWindowSurface(window);
                    //set renderer
                    gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
                    if(gRenderer == NULL){
                        std::cout << "The variable 'gRenderer' was not able to initialize! Error: " << SDL_GetError() << std::endl;\
                    }
                    //background white
                    SDL_FillRect(renderSurface, NULL, SDL_MapRGB(renderSurface->format, 0xFF, 0xFF, 0xFF));

                    //update window
                    SDL_UpdateWindowSurface(window);

                }
            }

        }


    //closes the window and frees resources
    SDL_GL_DeleteContext(mainGLContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
