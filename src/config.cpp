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
	
	unsigned int uval;
	bool bval;
	//float fval;
	
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
	VOLUME_MUSIC = vol->FirstChildElement("music")->FloatAttribute("volume");

}

void updateConfig(void){
	XMLElement *vol = xml.FirstChildElement("volume")->FirstChildElement("master")->ToElement();
	vol->SetAttribute("volume",VOLUME_MASTER);
	
	xml.SaveFile("config/settings.xml", false);
}
