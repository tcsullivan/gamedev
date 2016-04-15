#include <config.hpp>

#include <ui.hpp>

using namespace tinyxml2;

extern unsigned int HLINE;
extern unsigned int SCREEN_WIDTH;
extern unsigned int SCREEN_HEIGHT;
extern bool		 	FULLSCREEN;

extern float VOLUME_MASTER;
extern float VOLUME_MUSIC;
extern float VOLUME_SFX;

extern std::string xmlFolder;

XMLDocument xml;
XMLElement *scr;
XMLElement *vol;

namespace config {

	void read(void) {
		unsigned int uval;
		float fval;
		bool bval;

		xml.LoadFile("config/settings.xml");
		scr = xml.FirstChildElement("screen");

		if (scr->QueryUnsignedAttribute("width",&uval) == XML_NO_ERROR)
			SCREEN_WIDTH = uval;
		else SCREEN_WIDTH = 1280;
		if (scr->QueryUnsignedAttribute("height",&uval) == XML_NO_ERROR)
			SCREEN_HEIGHT = uval;
		else SCREEN_HEIGHT = 800;
		if (scr->QueryBoolAttribute("fullscreen",&bval) == XML_NO_ERROR)
			FULLSCREEN = bval;
		else FULLSCREEN = false;
		if (xml.FirstChildElement("hline")->QueryUnsignedAttribute("size",&uval) == XML_NO_ERROR)
			HLINE = uval;
		else HLINE = 3;

		vol = xml.FirstChildElement("volume");

		if (vol->FirstChildElement("master")->QueryFloatAttribute("volume",&fval) == XML_NO_ERROR)
			VOLUME_MASTER = fval;
		else VOLUME_MASTER = 50;
		if (vol->FirstChildElement("music")->QueryFloatAttribute("volume",&fval) == XML_NO_ERROR)
			VOLUME_MUSIC = fval;
		else VOLUME_MUSIC = 50;
		if (vol->FirstChildElement("sfx")->QueryFloatAttribute("volume",&fval) == XML_NO_ERROR)
			VOLUME_SFX = fval;
		else VOLUME_SFX = 50;

		xmlFolder = xml.FirstChildElement("world")->Attribute("start");
		if (xmlFolder=="\0")xmlFolder = "xml/";
		std::cout << "Folder: " << xmlFolder << std::endl;

		ui::initFonts();
		ui::setFontFace(xml.FirstChildElement("font")->Attribute("path"));

		if (xml.FirstChildElement("debug"))
			ui::debug = ui::posFlag = true;

		config::update();
	}

	void update(void) {
		Mix_Volume(0,VOLUME_MASTER);
		Mix_Volume(1,VOLUME_SFX * (VOLUME_MASTER/100.0f));
		Mix_VolumeMusic(VOLUME_MUSIC * (VOLUME_MASTER/100.0f));
	}

	void save(void) {
		vol->FirstChildElement("master")->SetAttribute("volume",VOLUME_MASTER);
		vol->FirstChildElement("music")->SetAttribute("volume",VOLUME_MUSIC);
		vol->FirstChildElement("sfx")->SetAttribute("volume", VOLUME_SFX);

		xml.SaveFile("config/settings.xml", false);
	}
}
