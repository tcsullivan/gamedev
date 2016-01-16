#include <common.h>
#include <entities.h>
#include <world.h>
#include <ui.h>

#include <tinyxml2.h>

using namespace tinyxml2;

extern World	*currentWorld;
extern Player	*player;

typedef struct {
	NPC *npc;
	unsigned int index;
} NPCDialog;

std::vector<XMLElement *>		dopt;

int commonAIFunc(NPC *speaker){
	XMLDocument xml;
	XMLElement *exml,*oxml;
	const char *name;
	unsigned int idx = 0;
	bool stop = false;
	
	xml.LoadFile(currentXML);
	exml = xml.FirstChildElement("Dialog");
	
	while(strcmp(exml->Attribute("name"),speaker->name))
		exml = exml->NextSiblingElement();
	
	exml = exml->FirstChildElement();
	
	do{
		if(!strcmp(exml->Name(),"text")){
			if(exml->UnsignedAttribute("id") == speaker->dialogIndex){
				
				if((oxml = exml->FirstChildElement("give"))){
					while(oxml){
						player->inv->addItem((ITEM_ID)oxml->UnsignedAttribute("id"),oxml->UnsignedAttribute("count"));
						oxml = oxml->NextSiblingElement();
					}
				}
				
				if((oxml = exml->FirstChildElement("option"))){
					const char *op;
					char *bp1 = new char[1],*bp2,*tmp;
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
				if((name = exml->Attribute("call"))){
					for(auto &n : currentWorld->npc){
						if(!strcmp(n->name,name)){
							if(exml->QueryUnsignedAttribute("callid",&idx) == XML_NO_ERROR){
								n->dialogIndex = idx;
							}
							n->addAIFunc(commonAIFunc,false);
							break;
						}
					}
				}
				if(exml->QueryUnsignedAttribute("nextid",&idx) == XML_NO_ERROR){
					speaker->dialogIndex = idx;
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
	return 0;
}

void destroyEverything(void);
void initEverything(void){
	std::vector<std::string> xmlFiles;
	XMLDocument xml;
	
	/*
	 * Read the XML directory into an array.
	 */
	
	if(getdir("./xml/",xmlFiles)){
		std::cout<<"Error reading XML files!!!1"<<std::endl;
		abort();
	}
	
	/*
	 * Sort the files alphabetically.
	 */
	
	strVectorSortAlpha(&xmlFiles);
	
	/*
	 * Load the first file found as currentWorld.
	 */
	
	for(unsigned int i=0;i<xmlFiles.size();i++){
		if(strncmp(xmlFiles[i].c_str(),".",1) && strncmp(xmlFiles[i].c_str(),"..",2)){
			
			/*
			 * Read in the XML file.
			 */
			
			currentWorld = loadWorldFromXML(xmlFiles[i].c_str());
			break;
		}
	}

	/*
	 * Spawn the player and begin the game.
	 */
	
	player = new Player();
	player->spawn(200,100);

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
