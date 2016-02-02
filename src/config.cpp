#include <config.h>

using namespace tinyxml2;

extern unsigned int HLINE;
extern unsigned int SCREEN_WIDTH;
extern unsigned int SCREEN_HEIGHT;
extern bool		 	FULLSCREEN;

extern float 		 VOLUME_MASTER;
extern float 		 VOLUME_MUSIC;

XMLDocument xml;

void readConfig(void){
	XMLElement *scr;
	XMLElement *vol;
	xml.LoadFile("config/settings.xml");
	scr = xml.FirstChildElement("screen");
	SCREEN_WIDTH  = scr->UnsignedAttribute("width");
	SCREEN_HEIGHT = scr->UnsignedAttribute("height");
	FULLSCREEN    = scr->BoolAttribute("fullscreen");
	HLINE         = xml.FirstChildElement("hline")->UnsignedAttribute("size");

	vol = xml.FirstChildElement("volume");
	VOLUME_MASTER = vol->FirstChildElement("master")->FloatAttribute("volume");
	VOLUME_MUSIC = vol->FirstChildElement("music")->FloatAttribute("volume");

}

void updateConfig(void){
	XMLElement *vol = xml.FirstChildElement("volume")->FirstChildElement("master")->ToElement();
	vol->SetAttribute("volume",VOLUME_MASTER);
	
	xml.SaveFile("config/settings.xml", false);
}
