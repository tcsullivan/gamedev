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
#include <string>
#include <vector>
#include <math.h>
#include <string>
#include <fstream>	
#include <thread>

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

/**
 * This flag lets the compiler know that we want to use shaders.
 */

#define SHADERS

template<typename N>
N abso(N v){
	if(v < 0.0){
		return v * -1;
	}else
		return v;
}

extern GLuint colorIndex;

/**
 * This structure contains a set of coordinates for ease of coding.
 */

typedef struct {
	float x;
	float y;
} vec2;

typedef struct {
	float x;
	float y;
	float z;
} vec3;

/**
 * This structure contains two sets of coordinates for ray drawing.
 */

typedef struct {
	vec2 start;
	vec2 end;
} Ray;

typedef struct{
	float red;
	float green;
	float blue;
} Color;

#include <Texture.h>

/**
 * Define the game's name (displayed in the window title).
 */

#define GAME_NAME		"Independent Study v.0.5 alpha - NOW WITH SOUND!"

/**
 * The desired width of the game window.
 */

//#define SCREEN_WIDTH	1280
extern unsigned int SCREEN_WIDTH;

/**
 * The desired height of the game window.
 */

//#define SCREEN_HEIGHT	720
extern unsigned int SCREEN_HEIGHT;

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

#define HLINE 4

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

extern GLuint shaderProgram;

/**
 * This class contains a string for identification and a value. It can be used to
 * save certain events for and decisions so that they can be recalled later.
 */

class Condition {
private:
	char *id;
	void *value;
public:
	Condition(const char *_id,void *val);
	~Condition();
	
	bool sameID(const char *s);
	void *getValue(void);
};

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

/**
 * We've encountered many problems when attempting to create delays for triggering
 * the logic function. As a result, we decided on using the timing libraries given
 * by <chrono> in the standard C++ library. This function simply returns the amount
 * of milliseconds that have passed sine the epoch.
 */

#ifdef __WIN32__
#define millis()	SDL_GetTicks()
#else
unsigned int millis(void);
#endif // __WIN32__

int getdir(const char *dir, std::vector<std::string> &files);
void strVectorSortAlpha(std::vector<std::string> *v);

const char *readFile(const char *path);

int strCreateFunc(const char *equ);

extern void *NULLPTR;

#endif // COMMON_H
