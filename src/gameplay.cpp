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

typedef struct {
	World *ptr;
	char *file;
} WorldXML;


typedef struct {
	NPC *npc;
	unsigned int index;
} NPCDialog;

std::vector<NPCDialog>		npcd;
std::vector<WorldXML>		earthxml;
std::vector<World *>		earth;
std::vector<XMLElement *>	dopt;

int commonAIFunc(NPC *speaker){
	XMLDocument xml;
	XMLElement *exml,*oxml;
	unsigned int idx;
	bool stop = false;
	
	for(auto &n : npcd){
		if(n.npc == speaker){
			idx = n.index;
			break;
		}
	}
	
	for(auto &e : earthxml){
		if(e.ptr == currentWorld){
			xml.LoadFile(e.file);
			
			exml = xml.FirstChildElement("Dialog");
			while(strcmp(exml->Attribute("name"),speaker->name))
				exml = exml->NextSiblingElement();
			
			exml = exml->FirstChildElement();
			
			do{
				if(!strcmp(exml->Name(),"text")){
					if(/*!strcmp(exml->Attribute("name"),speaker->name) &&*/ exml->UnsignedAttribute("id") == idx){
						
						if((oxml = exml->FirstChildElement("option"))){
							const char *op;
							char *bp1 = new char[1],*bp2,*tmp;
							unsigned int idx = 0;
							bp1[0] = '\0';
							while(oxml){
								op = oxml->Attribute("text");
								
								bp2 = new char[strlen(bp1) + strlen(op) + 2];
								strcpy(bp2,bp1);
								
								bp2[idx++] = ':';
								strcpy(bp2+idx,op);
								idx += strlen(op);
								
								tmp = bp1;
								bp1 = bp2;
								delete[] tmp;
								
								dopt.push_back(oxml);
								
								oxml = oxml->NextSiblingElement();
							}
							ui::dialogBox(speaker->name,bp1,false,exml->GetText());
							ui::waitForDialog();
							if(ui::dialogOptChosen){
								exml = dopt[ui::dialogOptChosen-1];
							}
							while(!dopt.empty())
								dopt.pop_back();
						}else{
							ui::dialogBox(speaker->name,"",false,exml->GetText());
							ui::waitForDialog();
						}
						if(exml->Attribute("call")){
							for(auto &n : currentWorld->npc){
								if(!strcmp(n->name,exml->Attribute("call"))){
									if(exml->QueryUnsignedAttribute("callid",&idx) == XML_NO_ERROR){
										for(auto &n2 : npcd){
											if(n2.npc == n){
												n2.index = idx;
												break;
											}
										}
									}
									n->addAIFunc(commonAIFunc,false);
									break;
								}
							}
						}
						if(exml->QueryUnsignedAttribute("nextid",&idx) == XML_NO_ERROR){
							for(auto &n : npcd){
								if(n.npc == speaker){
									n.index = idx;
									break;
								}
							}
							if(exml->QueryBoolAttribute("stop",&stop) == XML_NO_ERROR && stop)
								return 0;
							else if(exml->QueryBoolAttribute("pause",&stop) == XML_NO_ERROR && stop)
								return 1;
							else return commonAIFunc(speaker);
						}
						return 0;
					}
				}
				exml = exml->NextSiblingElement();
			}while(exml);
		}
	}
	return 0;
}

void destroyEverything(void);
void initEverything(void){
	const char *name;
	std::vector<std::string> xmlFiles;
	static char *file;
	bool dialog;
	float spawnx;
	XMLDocument xml;
	XMLElement *wxml;
	
	if(getdir("./xml/",xmlFiles)){
		std::cout<<"Error reading XML files!!!1"<<std::endl;
		abort();
	}
	
	for(auto x : xmlFiles){
		if(strncmp(x.c_str(),".",1) && strncmp(x.c_str(),"..",2)){
			file = new char[5 + x.size()];
			memset(file,0,5 + x.size());
			strncpy(file,"xml/",4);
			strcpy(file+4,x.c_str());
			xml.LoadFile(file);

			wxml = xml.FirstChildElement("World")->FirstChildElement();

			earth.push_back(new World());
			
			do{
				name = wxml->Name();
				
				if(!strcmp(name,"style")){
					earth.back()->setBackground((WORLD_BG_TYPE)wxml->UnsignedAttribute("background"));
					earth.back()->setBGM(wxml->Attribute("bgm"));
				}else if(!strcmp(name,"generation")){
					if(!strcmp(wxml->Attribute("type"),"Random")){
						earth.back()->generate(wxml->UnsignedAttribute("width"));
					}
				}else if(!strcmp(name,"mob")){
					if(wxml->QueryFloatAttribute("x",&spawnx) != XML_NO_ERROR)
						earth.back()->addMob(wxml->UnsignedAttribute("type"),getRand() % earth.back()->getTheWidth() / 2,100);
					else
						earth.back()->addMob(wxml->UnsignedAttribute("type"),wxml->FloatAttribute("x"),wxml->FloatAttribute("y"));
				}else if(!strcmp(name,"npc")){
					if(wxml->QueryFloatAttribute("x",&spawnx) != XML_NO_ERROR)
						earth.back()->addNPC(getRand() % earth.back()->getTheWidth() / 2.0f,100);
					else
						earth.back()->addNPC(wxml->FloatAttribute("x"),wxml->FloatAttribute("y"));
					
					if(wxml->Attribute("name")){
						delete[] earth.back()->npc.back()->name;
						earth.back()->npc.back()->name = new char[strlen(wxml->Attribute("name"))+1];
						strcpy(earth.back()->npc.back()->name,wxml->Attribute("name"));
					}
					dialog = false;
					if(wxml->QueryBoolAttribute("hasDialog",&dialog) == XML_NO_ERROR && dialog){
						for(auto &ex : earthxml){
							if(ex.ptr == earth.back())
								goto SKIP;
						}
						earthxml.push_back((WorldXML){earth.back(),new char[64]});
						strcpy(earthxml.back().file,file);
SKIP:
						earth.back()->npc.back()->addAIFunc(commonAIFunc,false);
						npcd.push_back((NPCDialog){earth.back()->npc.back(),0});
					}
				}
				
				wxml = wxml->NextSiblingElement();
			}while(wxml);
			
			delete[] file;
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
