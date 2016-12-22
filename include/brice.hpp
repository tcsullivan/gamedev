/**
 * @file brice.hpp
 * @brief A system for saving player information.
 */

#ifndef BRICE_H_
#define BRICE_H_

#include <string>

namespace game {

	/**
	 * Allows the player to jump, if set to true.
	 */
	extern bool canJump;

	/**
	 * Allows the player to sprint, if set to true.
	 */
	extern bool canSprint;

	/**
	 * Gets a value from the saved brice and returns it.
	 * @param id the id of the value
	 * @return the string value
	 */
	std::string getValue(const std::string& id);
	
	/**
	 * Sets a value in the brice, creating it if it doesn't exist.
	 * @param id the id of the value
	 * @param value the value
	 * @return true if the value was updated, not created  
	 */
	bool setValue(const std::string& id, const std::string& value);

	/**
	 * Resets the brice to it's default values.
	 * Note: these are hardcoded into the program.
	 */
	void briceClear(void);

	/**
	 * Saves the brice to it's file (brice.dat).
	 */
	void briceSave(void);

	/**
	 * Loads the brice from it's file (brice.dat).
	 */
	void briceLoad(void);

	/**
	 * Reloads the brice. 
	 */
	void briceUpdate(void);
}

#endif // BRICE_H_
