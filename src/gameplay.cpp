#include <common.h>
#include <entities.h>
#include <world.h>
#include <ui.h>

#include <tinyxml2.h>

using namespace tinyxml2;

extern World	*currentWorld;
extern Player	*player;

/*
 *	new world 
 *	gen
 * 	setbackground
 * 	setbgm
 * 	add...
 * 
 */

/*
 *	World definitions
 */

/*
 *	initEverything() start
 */

std::vector<World *>	earth;

void destroyEverything(void);
void initEverything(void){
	const char *gentype;
	std::vector<std::string> xmlFiles;
	char *file;
	XMLDocument xml;
	if(getdir("./xml/",xmlFiles)){
		std::cout<<"Error reading XML files!!!1"<<std::endl;
		abort();
	}
	for(auto x : xmlFiles){
		if(strncmp(x.c_str(),".",1) && strncmp(x.c_str(),"..",2)){
			file = new char[4 + x.size()];
			strncpy(file,"xml/",4);
			strcpy(file+4,x.c_str());
			xml.LoadFile(file);
			delete[] file;

			earth.push_back(new World());
			earth.back()->setBackground((WORLD_BG_TYPE)atoi(xml.FirstChildElement("World")->FirstChildElement("Background")->GetText()));
			earth.back()->setBGM(xml.FirstChildElement("World")->FirstChildElement("BGM")->GetText());
			gentype = xml.FirstChildElement("World")->FirstChildElement("GenerationType")->GetText();
			if(!strcmp(gentype,"Random")){
				std::cout<<"rand\n";
				earth.back()->generate(atoi(xml.FirstChildElement("World")->FirstChildElement("GenerationWidth")->GetText()));
			}else{
				abort();
			}
		}
	}
	
	player = new Player();
	player->spawn(200,100);

	currentWorld = earth.front();
	currentWorld->bgmPlay(NULL);
	atexit(destroyEverything);
}

extern std::vector<int (*)(NPC *)> AIpreload;
extern std::vector<NPC *> AIpreaddr;

void destroyEverything(void){
	
	while(!AIpreload.empty())
		AIpreload.pop_back();
	while(!AIpreaddr.empty())
		AIpreaddr.pop_back();
}
