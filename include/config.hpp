#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace game {
	extern unsigned int HLINE;
	extern unsigned int SCREEN_WIDTH;
	extern unsigned int SCREEN_HEIGHT;
	extern bool         FULLSCREEN;

	namespace config {
		extern float VOLUME_MASTER;
		extern float VOLUME_MUSIC;
		extern float VOLUME_SFX;

		extern std::string xmlFolder;
		
		void read(void);
		void update(void);
		void save(void);
	}
}

#endif //CONFIG_H
