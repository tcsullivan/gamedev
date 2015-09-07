///THIS FILE IS USED FOR VARIABLES THAT WILL BE ACCESED BY MULTIPLE CLASSES/FILES
#include <iostream>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "UIClass.h" //This can access SDL_Event e, if it won't compile for you, move it down to right above the ui object definition I guess :P

//SDL VARIABLES
SDL_Window    *window = NULL;
SDL_Surface   *renderSurface = NULL;
SDL_GLContext  mainGLContext = NULL;

//WINODWS VARIABLES
const float sh = SCREEN_HEIGHT;
const float sw = SCREEN_WIDTH;
bool gameRunning = true;
SDL_Event e;

//OTHER VARIABLES
UIClass ui;

//FUNCTIONS
