/** @file common.h
 * @brief Common items needed by most other files.
 * 
 * This file contains headers, variables and functions that are needed in
 * most other files included in this project.
 */

#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <cstdlib>
#include <vector>
#include <math.h>
#include <string>
#include <fstream>	

#define GLEW_STATIC
#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#ifdef __WIN32__
typedef unsigned int uint;
#undef near
#endif

#include <Texture.h>

/**
 * This flag lets the compiler know that we want to use shaders.
 */

#define SHADERSs

/**
 * This structure contains a set of coordinates for ease of coding.
 */

typedef struct {
	float x;
	float y;
} vec2;

/**
 * This structure contains two sets of coordinates for ray drawing.
 */

typedef struct {
	vec2 start;
	vec2 end;
} Ray;

/**
 * Define the game's name (displayed in the window title).
 */

#define GAME_NAME		"Independent Study v.0.4 alpha"

/**
 * The desired width of the game window.
 */

#define SCREEN_WIDTH	1024

/**
 * The desired height of the game window.
 */

#define SCREEN_HEIGHT	720

//#define FULLSCREEN

/**
 * Define the length of a single HLINE.
 * The game has a great amount of elements that need to be drawn or detected, and having each
 * of them use specific hard-coded numbers would be painful to debug. As a solution, this
 * definition was made. Every item being drawn to the screen and most object detection/physic
 * handling is done based off of this number. Increasing it will give the game a zoomed-in
 * feel, while decreasing it will do the opposite.
 * 
 */

#define HLINE 3

/**
 * A 'wrapper' for libc's srand(), as we hope to eventually have our own random number
 * generator.
 */

#define initRand(s) srand(s)

/**
 * A 'wrapper' for libc's rand(), as we hope to eventually have our own random number
 * generator.
 */

#define getRand() rand()

/**
 * Included in common.h is a prototype for DEBUG_prints, which writes a formatted
 * string to the console containing the callee's file and line number. This macro simplifies
 * it to a simple printf call.
 * 
 * DEBUG must be defined for this macro to function.
 */

#define DEBUG_printf( message, ...) DEBUG_prints(__FILE__, __LINE__, message, __VA_ARGS__ )

/**
 * Defines pi for calculations that need it.
 */

#define PI 3.1415926535


/**
 * References the variable in main.cpp, used for smoother drawing.
 */

extern unsigned int deltaTime;

/**
 * References the variable in main.cpp, used for drawing with the player.
 */
 
extern vec2 offset;

/**
 * Counts the number of times logic() (see main.cpp) has been called, for animating
 * sprites.
 */
extern unsigned int loops;

/**
 *	Prints a formatted debug message to the console, along with the callee's file and line
 *	number.
 */

void DEBUG_prints(const char* file, int line, const char *s,...);

/**
 * Sets color using glColor3ub(), but handles potential overflow.
 */

void safeSetColor(int r,int g,int b);

/**
 * Sets color using glColor4ub(), but handles potential overflow.
 */

void safeSetColorA(int r,int g,int b,int a);

#endif // COMMON_H
