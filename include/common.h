#ifndef COMMON_H
#define COMMON_H

/*
 *	Include basic C/C++ facilities
*/

#include <iostream>
#include <cstdlib>
#include <vector>
#include <math.h>

/*
 *	Include GLEW and the SDL 2 headers
*/

#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

/*
 *	Create a basic 2-point structure for coordinate saving
*/

typedef struct {
	float x;
	float y;
} vec2;

/*
 *	Define the game's name (displayed in the window title),
 *	the desired window dimensions,
 *	and whether or not we want the window to be fullscreen.
*/

#define GAME_NAME		"Independent Study v.0.2 alpha"

#define SCREEN_WIDTH	1280
#define SCREEN_HEIGHT	720

//#define FULLSCREEN

/*
 *	Define the length of a single HLINE.
 * 
 *	The game has a great amount of elements that need to be drawn or detected, and having each
 *	of them use specific hard-coded numbers would be painful to debug. As a solution, this
 *	definition was made. Every item being drawn to the screen and most object detection/physic
 *	handling is done based off of this number. Increasing it will give the game a zoomed-in
 *	feel, while decreasing it will do the opposite.
 * 
*/

#define HLINE 3	// 3 as in 3 pixels

/*
 *	Define 'our' random number generation library. Eventually these macros will be replaced
 *	with actual functions.
 * 
*/

#define initRand(s) srand(s)
#define getRand()	rand()

/*
 *	At the bottom of this header is the prototype for DEBUG_prints, which writes a formatted
 *	string to the console containing the callee's file and line number. This macro simplifies
 *	it to a simple printf call.
 * 
 *	DEBUG must be defined for this macro to function.
 * 
*/

#define DEBUG_printf( message, ...) DEBUG_prints(__FILE__, __LINE__, message, __VA_ARGS__ )

/*
 *	References the variable in main.cpp, used for smoother drawing.
*/

extern unsigned int deltaTime;

/*
 *	Loads an image from the given file path and attempts to make a texture out of it. The
 *	resulting GLuint is returned (used to recall the texture in glBindTexture).
 * 
*/

GLuint loadTexture(const char *fileName);

/*
 *	Prints a formatted debug message to the console, along with the callee's file and line
 *	number.
 * 
*/

void DEBUG_prints(const char* file, int line, const char *s,...);

#endif // COMMON_H
