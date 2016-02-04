#include <config.h>

using namespace tinyxml2;

extern unsigned int HLINE;
extern unsigned int SCREEN_WIDTH;
extern unsigned int SCREEN_HEIGHT;
extern bool		 	FULLSCREEN;

extern float 		 VOLUME_MASTER;
extern float 		 VOLUME_MUSIC;
extern float 		 VOLUME_SFX;

XMLDocument xml;
XMLElement *scr;
XMLElement *vol;

void readConfig(){
	unsigned int uval;
	bool bval;

	xml.LoadFile("config/settings.xml");
	scr = xml.FirstChildElement("screen");
	
	if(scr->QueryUnsignedAttribute("width",&uval) == XML_NO_ERROR)
		SCREEN_WIDTH = uval;
	else SCREEN_WIDTH = 1280;
	if(scr->QueryUnsignedAttribute("height",&uval) == XML_NO_ERROR)
		SCREEN_HEIGHT = uval;
	else SCREEN_HEIGHT = 800;
	if(scr->QueryBoolAttribute("fullscreen",&bval) == XML_NO_ERROR)
		FULLSCREEN = bval;
	else FULLSCREEN = false;
	if(xml.FirstChildElement("hline")->QueryUnsignedAttribute("size",&uval) == XML_NO_ERROR)
		HLINE = uval;
	else HLINE = 3;
	
	/*SCREEN_WIDTH  = scr->UnsignedAttribute("width");
	SCREEN_HEIGHT = scr->UnsignedAttribute("height");
	FULLSCREEN    = scr->BoolAttribute("fullscreen");
	HLINE         = xml.FirstChildElement("hline")->UnsignedAttribute("size");*/

	vol = xml.FirstChildElement("volume");
	VOLUME_MASTER = vol->FirstChildElement("master")->FloatAttribute("volume");
	VOLUME_MUSIC  = vol->FirstChildElement("music")->FloatAttribute("volume");
	VOLUME_SFX    = vol->FirstChildElement("sfx")->FloatAttribute("volume");

}

void updateConfig(){
	vol->FirstChildElement("master")->SetAttribute("volume",VOLUME_MASTER);
	vol->FirstChildElement("music")->SetAttribute("volume",VOLUME_MUSIC);
	vol->FirstChildElement("sfx")->SetAttribute("volume", VOLUME_SFX);

	Mix_Volume(0,VOLUME_MASTER);
	Mix_Volume(1,VOLUME_SFX);
	Mix_VolumeMusic(VOLUME_MUSIC);
}

void saveConfig(){
	xml.SaveFile("config/settings.xml", false);
}
