#ifndef COMMON_H
#define COMMON_H

typedef struct{float x; float y;}vec2;

///THIS FILE IS USED FOR VARIABLES THAT WILL BE ACCESED BY MULTIPLE CLASSES/FILES

#include <iostream>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>
#include <UIClass.h>
#include <entities.h>
#include <World.h>

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 800
#define FULLSCREEN

#define HLINE (2.0f / (SCREEN_WIDTH / 4))

//SDL VARIABLES
extern SDL_Window    *window;
extern SDL_Surface   *renderSurface;
extern SDL_GLContext  mainGLContext;

//WINDOW VARIABLES
extern bool gameRunning;

extern int grand(void);

#endif // COMMON_H
