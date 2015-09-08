#ifndef COMMON_H
#define COMMON_H

///THIS FILE IS USED FOR VARIABLES THAT WILL BE ACCESED BY MULTIPLE CLASSES/FILES

#include <iostream>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <UIClass.h>
#include <windowClass.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
//#define FULLSCREEN

//SDL VARIABLES
extern SDL_Window    *window;
extern SDL_Surface   *renderSurface;
extern SDL_GLContext  mainGLContext;

//WINODWS VARIABLES
extern bool gameRunning;

#endif // COMMON_H
