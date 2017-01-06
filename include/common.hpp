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
#include <unordered_map>

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
#undef near
#endif

/**
 * Prints a formatted string to the terminal with file and line number, for debugging
 */
#define DEBUG_printf(message, ...) DEBUG_prints(__FILE__, __LINE__, message, __VA_ARGS__)

#define BREAKPOINT __asm__("int $3")

#define coalesce(v1, v2) ((v1 != nullptr) ? v1 : v2)

/**
 * Creates a coordinate of integers.
 */
typedef struct {
	int x; /**< The x coordinate */
	int y; /**< The y coordinate */
} ivec2;

/**
 * A pair of x and y for dimensions (i.e. width and height).
 */
typedef ivec2 dim2;

/**
 * Creates a coordinate out of floating point integers.
 */
struct vec2 {
	float x;
	float y;

	vec2(float _x = 0.0f, float _y = 0.0f)
		: x(_x), y(_y) {}

	bool operator==(const vec2 &v) const {
		return (x == v.x) && (y == v.y);
	}

	template<typename T>
	const vec2 operator=(const T &n) {
		x = y = n;
		return *this;
	}

	template<typename T>
	vec2 operator+(T n) const {
		return vec2 (x + n, y + n);
	}

	vec2 operator+(const vec2 &v) {
		return vec2 (x + v.x, y + v.y);
	}

	vec2 operator*(const float&n) const {
		return vec2 (x * n, y * n);
	}

	vec2 operator/(const float& n) const {
		return vec2 (x / n, y / n);
	}

	// std::swap can't work due to being packed

	inline void swapX(vec2 &v) {
		float t = x;
		x = v.x, v.x = t;
	}

	inline void swapY(vec2 &v) {
		float t = y;
		y = v.y, v.y = t;
	}

	std::string toString(void) const {
		return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
	}

} __attribute__ ((packed));

/**
 * A structure for three-dimensional points.
 */
struct vec3 {
	float x; /**< The x coordinate */
	float y; /**< The y coordinate */
	float z; /**< The z coordinate */

	vec3(float _x = 0.0f, float _y = 0.0f, float _z = 1.0f)
		: x(_x), y(_y), z(_z) {}

} __attribute__ ((packed));

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

    Color() {
		red = green = blue = alpha = 0.0f;
	}

	Color(float r, float g ,float b) {
		red = r;
		green = g;
		blue = b;
        alpha = 255;
	}

    Color(float r, float g, float b, float a) {
        red = r;
        green = g;
        blue = b;
        alpha = a;
    }

	Color operator-=(float a) {
		red -= a;
		green -= a;
		blue -= a;
		return{red+a,green+a,blue+a};
	}
	Color operator+=(float a) {
		return{red+a,green+a,blue+a};
	}
	Color operator=(float a) {
		return{red=a,green=a,blue=a};
	}
};

/**
 * The amount of game ticks that should occur each second.
 */
constexpr const unsigned int TICKS_PER_SEC = 20;

/**
 * The amount of milliseconds it takes for a game tick to fire.
 */
constexpr const float MSEC_PER_TICK = 1000.0f / TICKS_PER_SEC;

/**
 * Separates a string into tokens using the given delimiter.
 *
 * @param  the string to parse
 * @param  the delimiting character
 * @return a vector of the tokens
 */
std::vector<std::string> StringTokenizer(const std::string& str, char delim);

/**
 * Seperates a string like, "23,12" to a vec2.
 * 
 * @param s the string to parse
 * @return the vec2 of the values passed in the string
 */
vec2 str2coord(std::string s);

/**
 * A function to draw a colored box for OpenGL.
 * To use it, the lower left hand and upper right hand coords are given.
 *
 * @param the lower left coordinate
 * @param the upper right coordinate
 * @param the z coordinate
 */
void drawRect(vec2 ll, vec2 ur, float z);

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
constexpr const float PI = 3.1415926535f;

// references the variable in main.cpp, used for drawing with the player
extern vec2 offset;

/**
 *	Prints a formatted debug message to the console, along with the callee's file and line
 *	number.
 */
void DEBUG_prints(const char* file, int line, const char *s,...);

unsigned int millis(void);

// reads the names of files in a directory into the given string vector
int getdir(std::string dir, std::vector<std::string> &files);

// sorts a vector of strings alphabetically
void strVectorSortAlpha(std::vector<std::string> *v);

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
