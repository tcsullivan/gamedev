/**
 * @file random.hpp
 * @brief Facilities to generate random numbers.
 */
#ifndef RANDOM_HPP_
#define RANDOM_HPP_

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

#endif // RANDOM_HPP_
