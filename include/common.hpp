#ifndef COMMON_H
#define COMMON_H

/**
 * @file common.h
 * @brief Common items needed by most other files.
 */

// standard library includes
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <list>
#include <iterator>

// alternative windows thread library
#ifndef __WIN32__
#include <thread>
#else
#include <win32thread.hpp>
#endif // __WIN32__

// local library includes
#define GLEW_STATIC
#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <shader_utils.hpp>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>

// game library includes
#include <config.hpp>

// windows stuff
#ifdef __WIN32__
using uint = unsigned int;
#undef near
#endif

/**
 * Prints a formatted string to the terminal with file and line number, for debugging
 */
#define DEBUG_printf(message, ...) DEBUG_prints(__FILE__, __LINE__, message, __VA_ARGS__)

#define coalesce(v1, v2) ((v1 != nullptr) ? v1 : v2)

#include <vector2.hpp>

using vec2 = vector2<float>;
using dim2 = vector2<int>;

/**
 * A structure for three-dimensional points.
 */
struct vec3 {
	float x; /**< The x coordinate */
	float y; /**< The y coordinate */
	float z; /**< The z coordinate */

	vec3(float _x = 0.0f, float _y = 0.0f, float _z = 1.0f)
		: x(_x), y(_y), z(_z) {}
};

/**
 * This structure contains two sets of coordinates for ray drawing.
 */

typedef struct {
	vec2 start; /**< The start coordinate of the ray */
	vec2 end;   /**< The end coordinate of the ray */
} Ray;

/**
 * Keeps track of an RGBA color.
 */
class Color{
public:
	float red;   /**< The amount of red, 0-255 or 0.0-1.0 depending on usage */
	float green; /**< The amount of green */
	float blue;  /**< The amount of blue */
	float alpha; /**< Transparency */

	Color(float r = 0, float g = 0, float b = 0, float a = 255)
		: red(r), green(g), blue(b), alpha(a) {}

	Color operator-(const float& a) {
		return Color(red - a, green - a, blue - a, alpha);
	}

	Color operator+(const float& a) {
		return Color(red + a, green + a, blue + a, alpha);
	}
};

/**
 * The amount of game ticks that should occur each second.
 */
constexpr unsigned int TICKS_PER_SEC = 20;

/**
 * The amount of milliseconds it takes for a game tick to fire.
 */
constexpr float MSEC_PER_TICK = 1000.0f / TICKS_PER_SEC;

/**
 * Separates a string into tokens using the given delimiter.
 *
 * @param  the string to parse
 * @param  the delimiting character
 * @return a vector of the tokens
 */
std::vector<std::string> StringTokenizer(const std::string& str, char delim);

/**
 * Returns a measurement in HLINEs
 *
 * @param the number of HLINEs, integer or decimal
 * @return the number in HLINEs
 */
template<typename T>
inline T HLINES(const T &n)
{
	return (static_cast<T>(game::HLINE) * n);
}

/**
 * A generically-named function to start the random number generator.
 * This currently redirects to the library's default, but allows for
 * a custom generator to be easily implemented.
 */
#define randInit srand

/**
 * Gets a random number (is a function).
 */
#define randGet rand

// defines pi for calculations that need it.
constexpr float PI = 3.1415926535f;

// references the variable in main.cpp, used for drawing with the player
extern vec2 offset;

/**
 *	Prints a formatted debug message to the console, along with the callee's file and line
 *	number.
 */
void DEBUG_prints(const char* file, int line, const char *s,...);

unsigned int millis(void);

// reads the names of files in a directory into the given string vector
int getdir(std::string dir, std::list<std::string>& files);

// reads the given file into a buffer and returns a pointer to the buffer
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
