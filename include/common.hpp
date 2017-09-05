/**
 * @file common.hpp
 * @brief Common things needed by all files, in theory.
 */
#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <algorithm>
#include <cctype>

// windows stuff
#ifdef __WIN32__
using uint = unsigned int;
#undef near
#endif

// defines pi for calculations that need it.
constexpr float PI = 3.1415926535f;

/**
 * Gets millisecond count since epoch.
 * @return number of milliseconds
 */
unsigned int millis(void);

/*namespace std {
	template<class T>
	constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
		return (v > hi) ? hi : ((v > lo) ? v : lo);
	}
}*/

#endif // COMMON_HPP_
