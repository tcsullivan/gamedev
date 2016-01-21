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
	
	/*
	 * Load the current world's XML file into memory for reading.
	 */
	
	xml.LoadFile(currentXML);
	exml = xml.FirstChildElement("Dialog");
	
	/*
	 * Search for the correct dialog block.
	 */
	
	while(strcmp(exml->Attribute("name"),speaker->name))
		exml = exml->NextSiblingElement();
	
	exml = exml->FirstChildElement();
	
	/*
	 * Search for which text block should be used.
	 */
	
	do{
		if(!strcmp(exml->Name(),"text")){
			if(exml->UnsignedAttribute("id") == (unsigned)speaker->dialogIndex){
				
				/*
				 * Handle any 'give' requests.
				 */
				
				if((oxml = exml->FirstChildElement("give"))){
					while(oxml){
						player->inv->addItem((ITEM_ID)oxml->UnsignedAttribute("id"),oxml->UnsignedAttribute("count"));
						oxml = oxml->NextSiblingElement();
					}
				}
				
				/*
				 * Handle any 'take' requests.
				 */
				
				if((oxml = exml->FirstChildElement("take"))){
					while(oxml){
						player->inv->takeItem((ITEM_ID)oxml->UnsignedAttribute("id"),oxml->UnsignedAttribute("count"));
						oxml = oxml->NextSiblingElement();
					}
				}
				
				/*
				 * Handle dialog options.
				 */
				
				if((oxml = exml->FirstChildElement("option"))){
					
					/*
					 * Convert the list of options into a single colon-separated string.
					 */
					
					std::string optstr;

					while(oxml){
						
						/*
						 * Create a buffer big enough for the next option.
						 */
						 
						optstr.append((std::string)":" + oxml->Attribute("text"));
						
						/*
						 * Append the next option.
						 */
						
						dopt.push_back(oxml);
						
						oxml = oxml->NextSiblingElement();
					}
					
					/*
					 * Get the player's choice, then set the XMLElement to the option's block.
					 */
					
					ui::dialogBox(speaker->name,optstr.c_str(),false,exml->GetText());
					ui::waitForDialog();
					
					if(ui::dialogOptChosen)
						exml = dopt[ui::dialogOptChosen-1];
					
					while(!dopt.empty())
						dopt.pop_back();
				}else{
					
					/*
					 * No options - simply print the text.
					 */
					
					ui::dialogBox(speaker->name,"",false,exml->GetText());
					ui::waitForDialog();
				}
				
				/*
				 * Give another NPC dialog if requested.
				 */
				
				if((name = exml->Attribute("call"))){
					for(auto &n : currentWorld->npc){
						if(!strcmp(n->name,name)){
							if(exml->QueryUnsignedAttribute("callid",&idx) == XML_NO_ERROR)
								n->dialogIndex = idx;
							n->addAIFunc(commonAIFunc,false);
							break;
						}
					}
				}
				
				/*
				 * Handle the next dialog block if this one leads to another.
				 */
				
				if(exml->QueryUnsignedAttribute("nextid",&idx) == XML_NO_ERROR){
					speaker->dialogIndex = idx;
					
					if(exml->QueryBoolAttribute("stop",&stop) == XML_NO_ERROR && stop){
						speaker->dialogIndex = -1;
						return 0;
					}else if(exml->QueryBoolAttribute("pause",&stop) == XML_NO_ERROR && stop){
						speaker->dialogIndex = -1;
						return 1;
					}else return commonAIFunc(speaker);
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
	currentWorld->save();
	delete currentWorld;
	
	while(!AIpreload.empty())
		AIpreload.pop_back();
	while(!AIpreaddr.empty())
		AIpreaddr.pop_back();
}
