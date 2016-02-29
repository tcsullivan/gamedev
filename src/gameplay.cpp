#include <common.h>
#include <entities.h>
#include <world.h>
#include <ui.h>

#include <tinyxml2.h>

using namespace tinyxml2;

extern World	*currentWorld;
extern Player	*player;

extern float shit;
extern Menu* currentMenu;
extern Menu pauseMenu;
extern Menu optionsMenu;

extern void mainLoop(void);

void segFault(){
	(*((int *)NULL))++;
}


typedef struct {
	NPC *npc;
	unsigned int index;
} NPCDialog;

std::vector<XMLElement *> dopt;

int commonAIFunc(NPC *speaker){
	XMLDocument xml;
	XMLElement *exml,*oxml;
	
	static unsigned int oldidx = 9999;
	
	const char *name;
	unsigned int idx = 0;
	bool stop = false;
	
	/*
	 * Load the current world's XML file into memory for reading.
	 */
	
	xml.LoadFile(currentXML.c_str());
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
				 * Handle any quest tags
				 */
	
				if((oxml = exml->FirstChildElement("quest"))){
					const char *qname;
					
					while(oxml){
						if((qname = oxml->Attribute("assign")))
							player->qh.assign(qname,"None",(std::string)oxml->GetText());
						else if((qname = oxml->Attribute("check"))){
							if(player->qh.hasQuest(qname) && player->qh.finish(qname)){
								goto CONT;
							}else{
								oldidx = speaker->dialogIndex;
								speaker->dialogIndex = oxml->UnsignedAttribute("fail");
								return commonAIFunc(speaker);
							}
						}	
						
						oxml = oxml->NextSiblingElement();
					}
				}

CONT:

				/*
				 * Handle any 'give' requests.
				 */
				
				if((oxml = exml->FirstChildElement("give"))){
					while(oxml){
						player->inv->addItem(oxml->Attribute("id"),oxml->UnsignedAttribute("count"));
						oxml = oxml->NextSiblingElement();
					}
				}
				
				/*
				 * Handle any 'take' requests.
				 */
				
				if((oxml = exml->FirstChildElement("take"))){
					while(oxml){
						player->inv->takeItem(oxml->Attribute("id"),oxml->UnsignedAttribute("count"));
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
					
					ui::dialogBox(speaker->name,optstr.c_str(),false,exml->GetText()+1);
					ui::waitForDialog();
					
					if(ui::dialogOptChosen)
						exml = dopt[ui::dialogOptChosen-1];
					
					while(!dopt.empty())
						dopt.pop_back();
				}else{
										
					/*
					 * No options - simply print the text.
					 */

					ui::dialogBox(speaker->name,NULL,false,exml->GetText());
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
						speaker->dialogIndex = 9999;
						return 0;
					}else if(exml->QueryBoolAttribute("pause",&stop) == XML_NO_ERROR && stop){
						//speaker->dialogIndex = 9999;
						return 1;
					}else return commonAIFunc(speaker);
				}else{
					if(oldidx != 9999){
						speaker->dialogIndex = oldidx;
						oldidx = 9999;
						return 1;
					}else{
						speaker->dialogIndex = 9999;
						return 0;
					}
				}
				//return 1;
			}
		}
		
		exml = exml->NextSiblingElement();
		
	}while(exml);
	
	return 0;
}

void commonTriggerFunc(Mob *callee){
	static bool lock = false;
	XMLDocument xml;
	XMLElement *exml;
	
	char *text,*pch;
	
	if(!lock){
		lock = true;
	
		xml.LoadFile(currentXML.c_str());
		exml = xml.FirstChildElement("Trigger");
		
		while(strcmp(exml->Attribute("id"),callee->heyid.c_str()))
			exml = exml->NextSiblingElement();
		
		player->vel.x = 0;

		ui::toggleBlackFast();
		ui::waitForCover();
		
		text = new char[256];
		strcpy(text,exml->GetText());
		pch = strtok(text,"\n");
		
		while(pch){
			ui::importantText(pch);
			ui::waitForDialog();
			
			pch = strtok(NULL,"\n");
		}
		
		delete[] text;
		
		ui::toggleBlackFast();
		
		callee->alive = false;
		lock = false;
	}
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
		if(xmlFiles[i] != "." && xmlFiles[i] != ".." && strcmp(xmlFiles[i].c_str()+xmlFiles[i].size()-3,"dat")){
			
			/*
			 * Read in the XML file.
			 */
		
			currentWorld = loadWorldFromXML(xmlFiles[i].c_str());
			break;
		}
	}
	
	pauseMenu.items.push_back(ui::createParentButton({-256/2,0},{256,75},{0.0f,0.0f,0.0f}, "Resume"));
	pauseMenu.items.push_back(ui::createChildButton({-256/2,-100},{256,75},{0.0f,0.0f,0.0f}, "Options"));
	pauseMenu.items.push_back(ui::createButton({-256/2,-200},{256,75},{0.0f,0.0f,0.0f}, "Save and Quit", ui::quitGame));
	pauseMenu.items.push_back(ui::createButton({-256/2,-300},{256,75},{0.0f,0.0f,0.0f}, "Segfault", segFault));
	pauseMenu.child = &optionsMenu;
	pauseMenu.parent = NULL;


	optionsMenu.items.push_back(ui::createSlider({0-(float)SCREEN_WIDTH/4,0-(512/2)}, {50,512}, {0.0f, 0.0f, 0.0f}, 0, 100, "Master", &VOLUME_MASTER));
	optionsMenu.items.push_back(ui::createSlider({-200,100}, {512,50}, {0.0f, 0.0f, 0.0f}, 0, 100, "Music", &VOLUME_MUSIC));
	optionsMenu.items.push_back(ui::createSlider({-200,000}, {512,50}, {0.0f, 0.0f, 0.0f}, 0, 100, "SFX", &VOLUME_SFX));
	optionsMenu.child = NULL;
	optionsMenu.parent = &pauseMenu;
	// optionsMenu.push_back(ui::createButton({-256/2,-200},{256,75},{0.0f,0.0f,0.0f}, (const char*)("Save and Quit"), );
	
	/*
	 * Spawn the player and begin the game.
	 */

	player = new Player();
	player->sspawn(0,100);

	currentWorld->bgmPlay(NULL);
	atexit(destroyEverything);
}

extern std::vector<int (*)(NPC *)> AIpreload;
extern std::vector<NPC *> AIpreaddr;

void destroyEverything(void){
	currentWorld->save();
	delete currentWorld;
	//delete[] currentXML;
	
	while(!AIpreload.empty())
		AIpreload.pop_back();
	while(!AIpreaddr.empty())
		AIpreaddr.pop_back();
}
