/**
 * @file debug.hpp
 * @brief Debugging utilities
 */
#ifndef DEBUG_HPP_
#define DEBUG_HPP_

/**
 * Prints a formatted string to the terminal with file and line number, for debugging
 */
#define DEBUG_printf(message, ...) DEBUG_prints(__FILE__, __LINE__, message, __VA_ARGS__)

/**
 *	Prints a formatted debug message to the console, along with the callee's file and line
 *	number.
 */
void DEBUG_prints(const char* file, int line, const char *s,...);

#endif // DEBUG_HPP_
