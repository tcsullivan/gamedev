/**
 * @file config.hpp
 * @brief Functions for loading/saving game settings.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace game {
	/**
	 * The size of an HLINE, according to the save file.
	 * This is the default "unit of measurement" in the game. Drawing scales to
	 * this, and it is used in game logic.
	 */
	extern unsigned int HLINE;

	/**
	 * The width of the screen, in pixels.
	 */
	extern unsigned int SCREEN_WIDTH;
	
	/**
	 * The height of the screen, in pixels.
	 */
	extern unsigned int SCREEN_HEIGHT;

	/**
	 * The window is fullscreen if this is true.
	 */
	extern bool         FULLSCREEN;

	namespace config {
		/**
		 * The current volume level of the master channel.
		 * Volumes are percentages, 0 to 100.
		 */
		extern float VOLUME_MASTER;

		/**
		 * Volume level of the background music (BGM).
		 */
		extern float VOLUME_MUSIC;

		/**
		 * Volume level of game sound effects.
		 */
		extern float VOLUME_SFX;

		/**
		 * The path of the folder to load world XML files from.
		 */
		extern std::string xmlFolder;
		
		/**
		 * Reads the settings file (config/settings.xml) into the game.
		 * Default values are hardcoded in (see src/config.cpp).
		 */
		void read(void);

		/**
		 * Updates settings with the current values.
		 */
		void update(void);

		/**
		 * Saves the current settings to the settings file.
		 */
		void save(void);
	}
}

#endif //CONFIG_H
