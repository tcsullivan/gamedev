#include <common.hpp>
#include <entities.hpp>
#include <world.hpp>
#include <ui.hpp>

#include <tinyxml2.h>
using namespace tinyxml2;

extern Player	*player;						// main.cpp
extern World	*currentWorld;					// main.cpp

extern Menu   pauseMenu;
extern Menu   optionsMenu;

extern std::string xmlFolder;

extern std::vector<NPC *> aipreload;

extern void mainLoop(void);						// main.cpp

std::vector<XMLElement *> dopt;

void destroyEverything(void);

int commonAIFunc(NPC *speaker)
{
	XMLDocument xml;
	XMLElement *exml,*oxml;

	static unsigned int oldidx = 9999;

	std::string name;
	unsigned int idx = 0;
	bool stop = false;

	// load the XML file and find the dialog tags
	xml.LoadFile(currentXML.c_str());
	exml = xml.FirstChildElement("Dialog");

	// search for the dialog block associated with this npc
	while (exml->StrAttribute("name") != speaker->name)
		exml = exml->NextSiblingElement();

	// search for the desired text block
	exml = exml->FirstChildElement();
	std::cout << speaker->dialogIndex << '\n';
	do {
		if (std::string("text") == exml->Name() && exml->UnsignedAttribute("id") == (unsigned)speaker->dialogIndex)
			break;
	} while ((exml = exml->NextSiblingElement()));

	// handle quest tags
	if ((oxml = exml->FirstChildElement("quest"))) {
		std::string qname;

		// iterate through all quest tags
		do {
			// assign quest
			if (!(qname = oxml->StrAttribute("assign")).empty())
				player->qh.assign(qname, "None", std::string(oxml->GetText())); // TODO add descriptions

			// check / finish quest
			else if (!(qname = oxml->StrAttribute("check")).empty()) {
				if (player->qh.hasQuest(qname) && player->qh.finish(qname)) {
					// QuestHandler::finish() did all the work..
					break;
				} else {
					// run error dialog
					oldidx = speaker->dialogIndex;
					speaker->dialogIndex = oxml->UnsignedAttribute("fail");
					return commonAIFunc(speaker);
				}
			}
		} while((oxml = oxml->NextSiblingElement()));
	}

	// handle give tags
	if ((oxml = exml->FirstChildElement("give"))) {
		do player->inv->addItem(oxml->Attribute("id"), oxml->UnsignedAttribute("count"));
		while ((oxml = oxml->NextSiblingElement()));
	}

	// handle take tags
	if ((oxml = exml->FirstChildElement("take"))) {
		do player->inv->takeItem(oxml->Attribute("id"), oxml->UnsignedAttribute("count"));
		while ((oxml = oxml->NextSiblingElement()));
	}

	// handle movement directs
	if ((oxml = exml->FirstChildElement("gotox")))
		speaker->moveTo(std::stoi(oxml->GetText()));

	// handle dialog options
	if ((oxml = exml->FirstChildElement("option"))) {
		std::string optstr;

		// convert option strings to a colon-separated format
		do {
			// append the next option
			optstr.append(std::string(":") + oxml->Attribute("text"));

			// save the associated XMLElement
			dopt.push_back(oxml);
		} while ((oxml = oxml->NextSiblingElement()));

		// run the dialog stuff
		ui::dialogBox(speaker->name, optstr, false, exml->GetText() + 1);
		ui::waitForDialog();

		if (ui::dialogOptChosen)
			exml = dopt[ui::dialogOptChosen - 1];

		dopt.clear();
	}

	// optionless dialog
	else {
		ui::dialogBox(speaker->name, "", false, exml->GetText());
		ui::waitForDialog();
	}

	// trigger other npcs if desired
	if (!(name = exml->StrAttribute("call")).empty()) {
		NPC *n = *std::find_if(std::begin(currentWorld->npc), std::end(currentWorld->npc), [name](NPC *npc) {
			return (npc->name == name);
		});

		if (exml->QueryUnsignedAttribute("callid", &idx) == XML_NO_ERROR) {
			n->dialogIndex = idx;
			n->addAIFunc(false);
		}
	}

	// handle potential following dialogs
	if ((idx = exml->UnsignedAttribute("nextid"))) {
		speaker->dialogIndex = idx;

		// stop talking
		if (exml->QueryBoolAttribute("stop", &stop) == XML_NO_ERROR && stop) {
			speaker->dialogIndex = 9999;
			return 0;
		}

		// pause, allow player to click npc to continue
		else if (exml->QueryBoolAttribute("pause", &stop) == XML_NO_ERROR && stop) {
			return 1;
		}

		// instantly continue
		else {
			return commonAIFunc(speaker);
		}
	}

	// stop talking
	else {
		// error text?
		if (oldidx != 9999) {
			speaker->dialogIndex = oldidx;
			oldidx = 9999;
			return 1;
		} else {
			speaker->dialogIndex = 9999;
			return 0;
		}
	}

	return 0;
}

void commonPageFunc(Mob *callee)
{
	ui::drawPage(callee->heyid);
	ui::waitForDialog();
	callee->health = 0;
}

void commonTriggerFunc(Mob *callee) {
	static bool lock = false;
	XMLDocument xml;
	XMLElement *exml;

	char *text,*pch;
	if (!lock) {
		lock = true;

		xml.LoadFile(currentXML.c_str());
		exml = xml.FirstChildElement("Trigger");

		while(exml->StrAttribute("id") != callee->heyid)
			exml = exml->NextSiblingElement();

		player->vel.x = 0;

		ui::toggleBlackFast();
		ui::waitForCover();

		text = new char[256];
		strcpy(text,exml->GetText());
		pch = strtok(text,"\n");

		while(pch) {
			ui::importantText(pch);
			ui::waitForDialog();

			pch = strtok(NULL,"\n");
		}

		delete[] text;

		ui::toggleBlackFast();

		callee->health = 0;
		lock = false;
	}
}

void initEverything(void) {
	std::vector<std::string> xmlFiles;
	XMLDocument xml;

	/*
	 * Read the XML directory into an array.
	 */

	if (getdir(std::string("./" + xmlFolder).c_str(), xmlFiles))
		UserError("Error reading XML files!!!");

	/*
	 * Sort the files alphabetically.
	 */

	strVectorSortAlpha(&xmlFiles);

	/*
	 * Load the first file found as currentWorld.
	 */

	for (xf : xmlFiles) { //unsigned int i=0;i<xmlFiles.size();i++){
		if (xf[0] != '.' && strcmp(&xf[xf.size() - 3], "dat")){
			// read the xml file
			std::cout << "File to load: " << xf << std::endl;
			currentWorld = loadWorldFromXML(xf);
			break;
		}
	}

	/*
	 * Spawn the player and begin the game.
	 */

	player = new Player();
	player->sspawn(0,100);

	ui::menu::init();

	currentWorld->bgmPlay(NULL);
	atexit(destroyEverything);
}

void destroyEverything(void) {
	currentWorld->save();
	//delete currentWorld;
	//delete[] currentXML;

	aipreload.clear();
}
