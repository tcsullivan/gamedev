#ifndef COMMON_H
#define COMMON_H

typedef struct{float x; float y;}vec2;

///THIS FILE IS USED FOR VARIABLES THAT WILL BE ACCESED BY MULTIPLE CLASSES/FILES

#include <iostream>
#include <cstdlib>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>
#include <UIClass.h>
#include <entities.h>
#include <World.h>

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720
#define SCREEN_RATIO  ((float)SCREEN_WIDTH/(float)SCREEN_HEIGHT)
//#define FULLSCREEN

#define HLINE (2.0f / (SCREEN_WIDTH / 4))
//#define HLINE (SCREEN_RATIO)

#define irand srand
#define grand rand

template<typename T, size_t N>
int eAmt(T (&)[N]){return N;}

//SDL VARIABLES
extern SDL_Window    *window;
extern SDL_Surface   *renderSurface;
extern SDL_GLContext  mainGLContext;

//WINDOW VARIABLES
extern bool gameRunning;

#endif // COMMON_H
