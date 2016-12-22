/**
 * @file gametime.hpp
 * @brief Handles time related operations
 */

#ifndef GAMETIME_H_
#define GAMETIME_H_

namespace game {
    namespace time {
		/**
		 * Sets the game's tick count to the desired amount.
		 * @param t desired tick count
		 */
        void setTickCount(unsigned int t);

		/**
		 * Gets the current tick count.
		 * @return the tick count
		 */
        unsigned int getTickCount(void);

		/**
		 * Calculates and returns the delta time.
		 * @return the delta time
		 */
        unsigned int getDeltaTime(void);

		/**
		 * Increments the game's tick count.
		 */
        void tick(void);

		/**
		 * Increments the game's tick count by the given amount of ticks.
		 * @param ticks the number of ticks to add
		 */
        void tick(unsigned int ticks);

		/**
		 * Determines if a tick has passed since the last call to this function.
		 * @return if a tick has passed
		 */
        bool tickHasPassed(void);

		/**
		 * Handles time updating.
		 * This should be called from the game's main loop.
		 */
        void mainLoopHandler(void);
    }
}

#endif // GAMETIME_H_
