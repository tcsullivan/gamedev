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
#include <cmath>
#include <algorithm>
#include <list>
#include <iterator>

#ifndef __WIN32__
#	include <thread>
#else
#	include <win32thread.hpp>
#endif // __WIN32__

#include <shader_utils.hpp>

#define GLEW_STATIC
#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>

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
 *	This separates xml strings with
 */
std::vector<std::string> StringTokenizer(const std::string& str, char delim);

/**
 * This structure contains a set of coordinates for ease of coding.
 */

typedef struct {
	int x;
	int y;
} ivec2;

typedef ivec2 dim2;

class vec2 {
public:
	float x;
	float y;

	vec2 ()
	{
		x = y = 0.0f;
	}

	vec2 (float _x, float _y)
	{
		x = _x;
		y = _y;
	}

	bool operator==(const vec2 &v) {
		return (x == v.x) && (y == v.y);
	}
	template<typename T>
	const vec2 operator=(const T &n) {
		x = y = n;
		return *this;
	}
	template<typename T>
	const vec2 operator+(const T &n) {
		return vec2 (x + n, y + n);
	}
};

class vec3{
public:
	float x;
	float y;
	float z;

	vec3 ()
	{
		x = y = z = 0.0f;
	}

	vec3 (float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	vec3 (float _x, float _y)
	{
		x = _x;
		y = _y;
		z = 1.0f;
	}

};

/**
 * This structure contains two sets of coordinates for ray drawing.
 */

typedef struct {
	vec2 start;
	vec2 end;
} Ray;

class Color{
public:
	float red;
	float green;
	float blue;
	float alpha;
    Color()
	{
		red = green = blue = alpha = 0;
	}
	Color(float r, float g ,float b)
	{
		red = r;
		green = g;
		blue = b;
        alpha = 255;
	}
    Color(float r, float g, float b, float a)
    {
        red = r;
        green = g;
        blue = b;
        alpha = a;
    }
	Color operator-=(float a) {
		red-=a;
		green-=a;
		blue-=a;
		return{red+a,green+a,blue+a};
	}
	Color operator+=(float a) {
		return{red+a,green+a,blue+a};
	}
	Color operator=(float a) {
		return{red=a,green=a,blue=a};
	}
};

/*
 * A function used to tell the program what shader, attributes, and uniforms
 * we want to draw our rectangles to. See below |
 *                                             \|/
 */
void useShader(GLuint *sh, GLint *tu, GLint *ca, GLint *ta);

/*
 * A function to draw a colored box for opengl
 * To use it, the lower left hand and upper right hand coords are passed along
 */
void drawRect(vec2 ll, vec2 ur, float z);

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

// reference to the shader programs we use throughout
extern GLuint textShader;
extern GLint textShader_attribute_coord;
extern GLint textShader_attribute_tex;
extern GLint textShader_uniform_texture;
extern GLint textShader_uniform_color;

extern GLuint worldShader;
extern GLint worldShader_attribute_coord;
extern GLint worldShader_attribute_tex;
extern GLint worldShader_uniform_texture;
extern GLint worldShader_uniform_texture_normal;
extern GLint worldShader_uniform_color;
extern GLint worldShader_uniform_transform;
extern GLint worldShader_uniform_ambient;
extern GLint worldShader_uniform_light;
extern GLint worldShader_uniform_light_color;
extern GLint worldShader_uniform_light_impact;
extern GLint worldShader_uniform_light_amt;

extern Color ambient;
/**
 *	Prints a formatted debug message to the console, along with the callee's file and line
 *	number.
 */
void DEBUG_prints(const char* file, int line, const char *s,...);

// TODO make sure we don't use these. Then burn them.
/**
 * Sets color using glColor3ub(), but handles potential overflow.
 */
void safeSetColor(int r,int g,int b);

/**
 * Sets color using glColor4ub(), but handles potential overflow.
 */
void safeSetColorA(int r,int g,int b,int a);

unsigned int millis(void);

// reads the names of files in a directory into the given string vector
int getdir(std::string dir, std::vector<std::string> &files);

// sorts a vector of strings alphabetically
void strVectorSortAlpha(std::vector<std::string> *v);

// reads the given file into a buffer and returns a pointer to the buffer
const char *readFile(const char *path);
std::string readFile(const std::string& path);
std::vector<std::string> readFileA(const std::string& path);

// aborts the program, printing the given error
void UserError(std::string reason);

namespace std {
	template<class T>
	constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
		return (v > hi) ? hi : ((v > lo) ? v : lo);
	}
}

#endif // COMMON_H
