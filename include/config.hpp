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
	extern bool FULLSCREEN;

	/**
	 * V-Sync is enabled if this is true. Only checked at program start.
	 */
	extern bool vsync;

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

		extern std::string fontFamily;
		
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

/**
 * The amount of game ticks that should occur each second.
 */
constexpr unsigned int TICKS_PER_SEC = 20;

/**
 * The amount of milliseconds it takes for a game tick to fire.
 */
constexpr float MSEC_PER_TICK = 1000.0f / TICKS_PER_SEC;

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

#endif //CONFIG_H
