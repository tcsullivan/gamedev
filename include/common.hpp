/** @file common.h
 * @brief Common items needed by most other files.
 *
 * This file contains headers, variables and functions that are needed in
 * most other files included in this project.
 */

#ifndef COMMON_H
#define COMMON_H

// holy moly
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <cmath>
#include <algorithm>

#define GLEW_STATIC
#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <config.hpp>

#ifdef __WIN32__
typedef unsigned int uint;
#undef near
#endif

// the number of ticks that should occur in one second
const unsigned int TICKS_PER_SEC = 20;

// the number of milliseconds inbetween each tick
const float MSEC_PER_TICK = 1000.0f / TICKS_PER_SEC;

// segfault-debugging output
//#define SEGFAULT

#ifdef SEGFAULT
#define C(x) std::cout << x << std::endl
#else
#define C(x)
#endif

// printf's a message to the console with file/line info
#define DEBUG_printf(message, ...) DEBUG_prints(__FILE__, __LINE__, message, __VA_ARGS__)

extern GLuint colorIndex;	// Texture.cpp?

/**
 * This structure contains a set of coordinates for ease of coding.
 */

typedef struct {
	int x;
	int y;
} ivec2;

typedef ivec2 dim2;

struct _vec2 {
	float x;
	float y;

	bool operator==(const _vec2 &v) {
		return (x == v.x) && (y == v.y);
	}
	template<typename T>
	const _vec2 operator=(const T &n) {
		x = y = n;
		return *this;
	}
	template<typename T>
	const _vec2 operator+(const T &n) {
		return _vec2 {x + n, y + n};
	}
};
typedef struct _vec2 vec2;

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

struct _color {
	float red;
	float green;
	float blue;
	_color operator-=(float a) {
		red-=a;
		green-=a;
		blue-=a;
		return{red+a,green+a,blue+a};
	}
	_color operator+=(float a) {
		return{red+a,green+a,blue+a};
	}
	_color operator=(float a) {
		return{red=a,green=a,blue=a};
	}
};

typedef struct _color Color;

// gets the length of `n` HLINEs
template<typename T>
inline T HLINES(const T &n)
{
	return (static_cast<T>(game::HLINE) * n);
}

// random number generator initializer (TODO decide how to random gen. numbers)
#define randInit    srand

// gets random number
#define randGet     rand

// defines pi for calculations that need it.
constexpr const float PI = 3.1415926535f;

// references the variable in main.cpp, used for drawing with the player
extern vec2 offset;

// the shader program created in main.cpp
extern GLuint shaderProgram;

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

// use our own millis function if we can, windows doesn't like <chrono> at the moment...
#ifdef __WIN32__
#define millis() SDL_GetTicks()
#else
unsigned int millis(void);
#endif // __WIN32__

// reads the names of files in a directory into the given string vector
int getdir(std::string dir, std::vector<std::string> &files);

// sorts a vector of strings alphabetically
void strVectorSortAlpha(std::vector<std::string> *v);

// reads the given file into a buffer and returns a pointer to the buffer
const char *readFile(const char *path);

// aborts the program, printing the given error
void UserError(std::string reason);

namespace std {
	template<class T>
	constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
		if (v < hi)
			return (v > lo) ? v : lo;
		else
			return (v < hi) ? v : hi;
	}
}

#endif // COMMON_H
