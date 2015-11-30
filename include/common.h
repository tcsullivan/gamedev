#ifndef COMMON_H
#define COMMON_H

/*
 *	Include basic C/C++ facilities
*/

#include <iostream>
#include <cstdlib>
#include <vector>
#include <math.h>
#include <thread>

/*
 *	Include GLEW and the SDL 2 headers
*/

#define GLEW_STATIC
#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <fstream>


#ifdef __WIN32__
typedef unsigned int uint;
#undef near
#endif

#include <Texture.h>

/*
 *	This flag lets the compiler know that we are using shaders
*/

#define SHADERSs

/*
 *	Create a basic 2-point structure for coordinate saving
*/

typedef struct {
	float x;
	float y;
} vec2;

typedef struct {
	vec2 start;
	vec2 end;
} Ray;

/*
 *	Define the game's name (displayed in the window title),
 *	the desired window dimensions,
 *	and whether or not we want the window to be fullscreen.
*/

#define GAME_NAME		"Independent Study v.0.4 alpha"

#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT	600

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


#define PI 3.1415926535


/*
 *	References the variable in main.cpp, used for smoother drawing.
*/

extern unsigned int deltaTime;

/*
 *	References the variable in main.cpp, used for drawing with the player
*/
extern vec2 offset;

extern float handAngle;

extern unsigned int loops;

/*
 *	Prints a formatted debug message to the console, along with the callee's file and line
 *	number.
 * 
*/

void DEBUG_prints(const char* file, int line, const char *s,...);

void safeSetColor(int r,int g,int b);
void safeSetColorA(int r,int g,int b,int a);

#endif // COMMON_H
