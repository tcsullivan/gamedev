#include <config.hpp>

#include <ui.hpp>
#include <fileio.hpp>

#include <SDL2/SDL_mixer.h>

#include <tinyxml2.h>
using namespace tinyxml2;

namespace game {
	unsigned int HLINE;
	unsigned int SCREEN_WIDTH;
	unsigned int SCREEN_HEIGHT;
	bool         FULLSCREEN;
	bool vsync;

	namespace config {
		static XMLDocument xml;
		static XMLElement *vol;

		float VOLUME_MASTER;
		float VOLUME_MUSIC;
		float VOLUME_SFX;

		std::string xmlFolder;
		std::string fontFamily;

		void read(void) {
			if (!fileExists("config/settings.xml"))
				copyFile("config/settings.xml", "config/settings.xml.example");

			xml.LoadFile("config/settings.xml");
			auto exml = xml.FirstChildElement("screen");

			if (exml->QueryUnsignedAttribute("width", &SCREEN_WIDTH) != XML_NO_ERROR)
				SCREEN_WIDTH = 1024;
			if (exml->QueryUnsignedAttribute("height", &SCREEN_HEIGHT) != XML_NO_ERROR)
				SCREEN_HEIGHT = 768;
			if (exml->QueryBoolAttribute("fullscreen", &FULLSCREEN) != XML_NO_ERROR)
				FULLSCREEN = false;
			if (exml->QueryBoolAttribute("vsync", &vsync) != XML_NO_ERROR)
				vsync = true;

			if (xml.FirstChildElement("hline")->QueryUnsignedAttribute("size", &HLINE) != XML_NO_ERROR)
				HLINE = 3;

			vol = exml = xml.FirstChildElement("volume");
			if (exml->FirstChildElement("master")->QueryFloatAttribute("volume", &VOLUME_MASTER) != XML_NO_ERROR)
				VOLUME_MASTER = 50;
			if (exml->FirstChildElement("music")->QueryFloatAttribute("volume", &VOLUME_MUSIC) != XML_NO_ERROR)
				VOLUME_MUSIC = 50;
			if (exml->FirstChildElement("sfx")->QueryFloatAttribute("volume", &VOLUME_SFX) != XML_NO_ERROR)
				VOLUME_SFX = 50;

			xmlFolder = xml.FirstChildElement("world")->StrAttribute("start");

			// FONT SETUP
			fontFamily = xml.FirstChildElement("font")->Attribute("path");

			if (xml.FirstChildElement("debug"))
				ui::debug = ui::posFlag = true;

			config::update();
		}

		void update(void) {
			Mix_Volume(0, static_cast<int>(VOLUME_SFX * (VOLUME_MASTER / 100)));
			Mix_VolumeMusic(static_cast<int>(VOLUME_MUSIC * (VOLUME_MASTER / 100)));
		}

		void save(void) {
			vol->FirstChildElement("master")->SetAttribute("volume", VOLUME_MASTER);
			vol->FirstChildElement("music")->SetAttribute("volume", VOLUME_MUSIC);
			vol->FirstChildElement("sfx")->SetAttribute("volume", VOLUME_SFX);

			xml.SaveFile("config/settings.xml", false);
		}
	}
}
