#include <world.hpp>

/* ----------------------------------------------------------------------------
** Includes section
** --------------------------------------------------------------------------*/

// standard library headers
#include <algorithm>
#include <sstream>
#include <fstream>
#include <memory>
#include <mutex>

// local game headers
#include <ui.hpp>
#include <gametime.hpp>

#include <render.hpp>
#include <engine.hpp>
#include <components.hpp>
#include <player.hpp>

// local library headers
#include <tinyxml2.h>
using namespace tinyxml2;

void makeWorldDrawingSimplerEvenThoughAndyDoesntThinkWeCanMakeItIntoFunctions(
		unsigned size, void *coordAddr, void *texAddr, unsigned triCount
	)
{
	glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, size, coordAddr);
	glVertexAttribPointer(Render::worldShader.tex  , 2, GL_FLOAT, GL_FALSE, size, texAddr  );
	glDrawArrays(GL_TRIANGLES, 0, triCount);
}

void makeWorldDrawingSimplerEvenThoughAndyDoesntThinkWeCanMakeItIntoFunctions_JustDrawThis(
		unsigned size, void *coordAddr, void *texAddr, unsigned triCount
	)
{
	Render::worldShader.enable();

	makeWorldDrawingSimplerEvenThoughAndyDoesntThinkWeCanMakeItIntoFunctions(size, coordAddr, texAddr, triCount);

	Render::worldShader.disable();
}

/* ----------------------------------------------------------------------------
** Variables section
** --------------------------------------------------------------------------*/

extern std::string  xmlFolder;

// particle mutex
std::mutex partMutex;

// externally referenced in main.cpp
int worldShade = 0;

// ground-generating constants
constexpr const float GROUND_HEIGHT_INITIAL =  80.0f;
constexpr const float GROUND_HEIGHT_MINIMUM =  60.0f;
constexpr const float GROUND_HEIGHT_MAXIMUM = 110.0f;
constexpr const float GROUND_HILLINESS      =  10.0f;

// defines grass height in HLINEs
constexpr const unsigned int GRASS_HEIGHT = 4;

// the path of the currently loaded XML file, externally referenced in places
std::string currentXML;

// pathnames of images for world themes
using StyleList = std::array<std::string, 9>;

static const std::vector<StyleList> bgPaths = {
	{ // Forest
		"bg.png", 				// daytime background
		"bgn.png",				// nighttime background
		"bgFarMountain.png",	// layer 1 (furthest)
		"forestTileFar.png",	// layer 2
		"forestTileBack.png",	// layer 3
		"forestTileMid.png",	// layer 4
		"forestTileFront.png",	// layer 5 (closest)
		"dirt.png",				// ground texture
		"grass.png"				// grass (ground-top) texture
	}
};

// pathnames of structure textures
static const std::string buildPaths[] = {
    "townhall.png",
	"house1.png",
    "house2.png",
    "house1.png",
    "house1.png",
    "fountain1.png",
    "lampPost1.png",
	"brazzier.png"
};

// alpha-related values used for world drawing? nobody knows...
static const float bgDraw[4][3]={
	{ 100, 240, 0.6  },
	{ 150, 250, 0.4  },
	{ 200, 255, 0.25 },
	{ 255, 255, 0.1  }
};

std::string currentXMLRaw;
XMLDocument currentXMLDoc;

/* ----------------------------------------------------------------------------
** Functions section
** --------------------------------------------------------------------------*/

void WorldSystem::generate(unsigned int width)
{
    float geninc = 0;

    // allocate space for world
    world.data = std::vector<WorldData> (width + GROUND_HILLINESS, WorldData { false, {0, 0}, 0, 0 });

    // prepare for generation
    world.data[0].groundHeight = GROUND_HEIGHT_INITIAL;
    auto wditer = std::begin(world.data) + GROUND_HILLINESS;

	if (world.indoor) {
		for (auto &l : world.data) {
			l.groundHeight = GROUND_HEIGHT_MINIMUM + 5;
			l.groundColor = 4;
		}
	} else {
	    // give every GROUND_HILLINESSth entry a groundHeight value
	    for (; wditer < std::end(world.data); wditer += GROUND_HILLINESS)
	        wditer[-static_cast<int>(GROUND_HILLINESS)].groundHeight = wditer[0].groundHeight + (randGet() % 8 - 4);

    	// create slopes from the points that were just defined, populate the rest of the WorldData structure
	    for (wditer = std::begin(world.data) + 1; wditer < std::end(world.data); wditer++){
	        auto w = &*(wditer);

    	    if (w->groundHeight != 0)
	            geninc = (w[static_cast<int>(GROUND_HILLINESS)].groundHeight - w->groundHeight) / GROUND_HILLINESS;

    	    w->groundHeight   = std::clamp(w[-1].groundHeight + geninc, GROUND_HEIGHT_MINIMUM, GROUND_HEIGHT_MAXIMUM);
	        w->groundColor    = randGet() % 32 / 8;
	        w->grassUnpressed = true;
	        w->grassHeight[0] = (randGet() % 16) / 3 + 2;
	        w->grassHeight[1] = (randGet() % 16) / 3 + 2;
	    }
	}

    // define x-coordinate of world's leftmost 'line'
    world.startX = (width - GROUND_HILLINESS) * game::HLINE / 2 * -1;
}

static Color ambient;

bool WorldSystem::save(const std::string& s)
{
	(void)s;
	/*for (const auto &e : entity)
		e->saveToXML();

	currentXMLDoc.SaveFile((s.empty() ? currentXML : xmlFolder + s).c_str(), false);*/
	return false;
}

/*static bool loadedLeft = false;
static bool loadedRight = false;

World *loadWorldFromXML(std::string path) {
	if (!currentXML.empty())
		currentWorld->save();

	return loadWorldFromXMLNoSave(path);
}

World *loadWorldFromPtr(World *ptr)
{
	currentWorld->save(); // save the current world to the current xml path

	if (ptr->getToLeft() == currentXML) {
		currentWorldToLeft = currentWorld;
		loadedRight = true;
		currentWorldToRight = loadWorldFromXMLNoSave(ptr->getToRight());
		loadedRight = false;
	} else if (ptr->getToRight() == currentXML) {
		currentWorldToRight = currentWorld;
		loadedLeft = true;
		currentWorldToLeft = loadWorldFromXMLNoSave(ptr->getToLeft());
		loadedLeft = false;
	}

    return ptr;
}

World *
loadWorldFromXMLNoTakeover(std::string path)
{
	loadedLeft = true, loadedRight = true;
	auto ret = loadWorldFromXMLNoSave(path);
	loadedLeft = false, loadedRight = false;
	return ret;
}
*/

void WorldSystem::load(const std::string& file)
{
	auto str2coord = [](std::string s) -> vec2 {
		auto cpos = s.find(',');
		s[cpos] = '\0';
		return vec2 (std::stof(s), std::stof(s.substr(cpos + 1)));
	};

	entityx::Entity entity;

	std::string xmlRaw;
	std::string xmlPath;

	// check for empty file name
	if (file.empty())
		return;

	// load file data to string
	xmlPath = xmlFolder + file;
	auto xmlRawData = readFile(xmlPath.c_str());
	xmlRaw = xmlRawData;
	delete[] xmlRawData;

	// let tinyxml parse the file
	if (xmlDoc.Parse(xmlRaw.data()) != XML_NO_ERROR)
		UserError("XML Error: Failed to parse file (not your fault though..?)");

	// include headers
	auto ixml = xmlDoc.FirstChildElement("include");
	while (ixml) {
		auto file = ixml->Attribute("file");
		if (file != nullptr) {
			DEBUG_printf("Including file: %s\n", file);
			xmlRaw.append(readFile((xmlFolder + file).c_str()));
		}
		ixml = ixml->NextSiblingElement();
	}

	xmlDoc.Parse(xmlRaw.data());

	// look for an opening world tag
	auto wxml = xmlDoc.FirstChildElement("World");
	if (wxml != nullptr) {
		wxml = wxml->FirstChildElement();
		world.indoor = false;
	} else {
		wxml = xmlDoc.FirstChildElement("IndoorWorld");
		if (wxml != nullptr) {
			wxml = wxml->FirstChildElement();
			world.indoor = true;
		} else {
			UserError("XML Error: Cannot find a <World> or <IndoorWorld> tag in " + xmlPath);
		}
	}

	world.toLeft = world.toRight = "";
	currentXMLFile = file;

	

	// iterate through tags
	while (wxml) {
		std::string tagName = wxml->Name();

		// style tag
		if (tagName == "style") {
			world.styleFolder = wxml->StrAttribute("folder");

			unsigned int styleNo;
			if (wxml->QueryUnsignedAttribute("background", &styleNo) != XML_NO_ERROR)
				UserError("XML Error: No background given in <style> in " + xmlPath);

			world.style = static_cast<WorldBGType>(styleNo);
			world.bgm = wxml->StrAttribute("bgm");

			bgFiles.clear();

			const auto& files = bgPaths[(int)world.style];

			for (const auto& f : files)
				bgFiles.push_back(world.styleFolder + "bg/" + f);

			bgTex = TextureIterator(bgFiles);
		}

        // world generation
        else if (tagName == "generation") {
			generate(wxml->UnsignedAttribute("width") / game::HLINE);
		}

		// indoor stuff
		else if (tagName == "house") {
			if (!world.indoor)
				UserError("<house> can only be used inside <IndoorWorld>");

			world.indoorWidth = wxml->FloatAttribute("width");
			world.indoorTex = Texture::loadTexture(wxml->Attribute("texture"));
		}

		// weather tag
		else if (tagName == "weather") {
			setWeather(wxml->GetText());
		}

		// link tags
		else if (tagName == "link") {
			auto linkTo = wxml->Attribute("left");
			if (linkTo != nullptr) {
				world.toLeft = linkTo;
			} else {
				linkTo = wxml->Attribute("right");
				if (linkTo != nullptr)
					world.toRight = linkTo;
				else
					UserError("<link> doesn't handle left or right... huh");
			}
		}

		// time setting
		else if (tagName == "time") {
            game::time::setTickCount(std::stoi(wxml->GetText()));
        }

		// custom entity tags
		else {
			auto cxml = xmlDoc.FirstChildElement(tagName.c_str());
			if (cxml != nullptr) {
				DEBUG_printf("Using custom tag <%s>\n", tagName.c_str());

				entity = game::entities.create();
				auto abcd = cxml->FirstChildElement();

				while (abcd) {
					std::string tname = abcd->Name();

					if (tname == "Position") {
						vec2 coords;

						if (wxml->Attribute("position") != nullptr) {
							coords = str2coord(wxml->StrAttribute("position"));
						} else {
							coords = str2coord(abcd->StrAttribute("value"));
						}

						float cdat[2] = {coords.x, coords.y};
						entity.assign<Position>(cdat[0], cdat[1]);
					} else if (tname == "Visible") {
						entity.assign<Visible>(abcd->FloatAttribute("value"));
					} else if (tname == "Sprite") {
						auto sprite = entity.assign<Sprite>();
						auto tex = abcd->Attribute("image");
						auto dim = Texture::imageDim(tex);
						sprite->addSpriteSegment(SpriteData(game::sprite_l.loadSprite(tex),
						                                    vec2(0, 0),
						                                    vec2(dim.x, dim.y) * 2),
						                         vec2(0, 0));
					}

					abcd = abcd->NextSiblingElement();
				}

			} else {
				UserError("Unknown tag <" + tagName + "> in file " + currentXMLFile);
			}
		}

		// hill creation
		/*else if (tagName == "hill") {
			addHill(ivec2 { wxml->IntAttribute("peakx"), wxml->IntAttribute("peaky") }, wxml->UnsignedAttribute("width"));
		}*/

		wxml = wxml->NextSiblingElement();
	}

	game::events.emit<BGMToggleEvent>();
}

/*
World *
loadWorldFromXMLNoSave(std::string path) {
	XMLDocument *_currentXMLDoc;
	static std::string _currentXML,
	                   _currentXMLRaw;

	XMLElement *wxml;
	XMLElement *vil;

	World *tmp;
	Entity *newEntity;
	bool Indoor;

	const char *ptr;
	std::string name, sptr;

    // iterate through world tags
	while (wxml) {
		newEntity = nullptr;
		name = wxml->Name();

   		// set spawn x for player
		else if (name == "spawnx" && !(loadedLeft | loadedRight)) {
			player->loc.x = std::stoi(wxml->GetText());
		}

        // mob creation
        else if (name == "rabbit") {
            newEntity = new Rabbit();
        } else if (name == "bird") {
            newEntity = new Bird();
        } else if (name == "trigger") {
        	newEntity = new Trigger();
        } else if (name == "door") {
            newEntity = new Door();
        } else if (name == "page") {
            newEntity = new Page();
        } else if (name == "cat") {
            newEntity = new Cat();
        } else if (name == "chest") {
			newEntity = new Chest();
		}

        // npc creation
        else if (name == "npc") {
			newEntity = new NPC();
		}

        // structure creation
        else if (name == "structure") {
			newEntity = new Structures();
		}

		// hill creation
		else if (name == "hill") {
			tmp->addHill(ivec2 { wxml->IntAttribute("peakx"), wxml->IntAttribute("peaky") }, wxml->UnsignedAttribute("width"));
		}

		if (newEntity != nullptr) {
			//bool alive = true;
			//if (wxml->QueryBoolAttribute("alive", &alive) != XML_NO_ERROR || alive) {
				switch (newEntity->type) {
				case NPCT:
					tmp->addNPC(dynamic_cast<NPC *>(newEntity));
					break;
				case MOBT:
					tmp->addMob(dynamic_cast<Mob *>(newEntity), vec2 {0, 0});
					break;
				case STRUCTURET:
					tmp->addStructure(dynamic_cast<Structures *>(newEntity));
					break;
				default:
					break;
				}

				std::swap(currentXML, _currentXML);
				std::swap(currentXMLRaw, _currentXMLRaw);
				newEntity->createFromXML(wxml, tmp);
				std::swap(currentXML, _currentXML);
				std::swap(currentXMLRaw, _currentXMLRaw);
			//}
		}

		wxml = wxml->NextSiblingElement();
	}

	Village *vptr;
	Structures *s;

	if (vil) {
		vptr = tmp->addVillage(vil->StrAttribute("name"), tmp);
		vil = vil->FirstChildElement();
	}

	while(vil) {
		name = vil->Name();

		 //READS DATA ABOUT STRUCTURE CONTAINED IN VILLAGE

		if (name == "structure") {
			s = new Structures();
			s->createFromXML(vil, tmp);
			tmp->addStructure(s);
		} else if (name == "stall") {
            sptr = vil->StrAttribute("type");

            // handle markets
            if (sptr == "market") {

                // create a structure and a merchant, and pair them
				s = new Structures();
				s->createFromXML(vil, tmp);
				tmp->addStructure(s);
				tmp->addMerchant(0, 100, true);
            }

            // handle traders
            else if (sptr == "trader") {
				s = new Structures();
				s->createFromXML(vil, tmp);
				tmp->addStructure(s);
			}

            // loop through buy/sell/trade tags
            XMLElement *sxml = vil->FirstChildElement();
            std::string tag;

			Merchant *merch = dynamic_cast<Merchant *>(*std::find_if(tmp->entity.rbegin(), tmp->entity.rend(), [&](Entity *e) {
				return (e->type == MERCHT);
			}));

            while (sxml) {
                tag = sxml->Name();

                if (tag == "buy") { //converts price to the currencies determined in items.xml
                    // TODO
                } else if (tag == "sell") { //converts price so the player can sell
                    // TODO
                } else if (tag == "trade") { //doesn't have to convert anything, we just trade multiple items
                	merch->trade.push_back(Trade(sxml->IntAttribute("quantity"),
										   sxml->StrAttribute("item"),
										   sxml->IntAttribute("quantity1"),
										   sxml->StrAttribute("item1")));
				} else if (tag == "text") { //this is what the merchant says
                    XMLElement *txml = sxml->FirstChildElement();
                    std::string textOption;

                    while (txml) {
                        textOption = txml->Name();
                        const char* buf = txml->GetText();

                        if (textOption == "greet") { //when you talk to him
                            merch->text[0] = std::string(buf, strlen(buf));
                            merch->toSay = &merch->text[0];
                        } else if (textOption == "accept") { //when he accepts the trade
                            merch->text[1] = std::string(buf, strlen(buf));
                        } else if (textOption == "deny") { //when you don't have enough money
                            merch->text[2] = std::string(buf, strlen(buf));
                        } else if (textOption == "leave") { //when you leave the merchant
                            merch->text[3] = std::string(buf, strlen(buf));
                        }
                        txml = txml->NextSiblingElement();
                    }
                }

                sxml = sxml->NextSiblingElement();
			}
		}

        float buildx = tmp->getStructurePos(-1).x;

		if (buildx < vptr->start.x)
			vptr->start.x = buildx;

		if (buildx > vptr->end.x)
			vptr->end.x = buildx;

		//go to the next element in the village block
		vil = vil->NextSiblingElement();
	}

	if (!loadedLeft && !loadedRight) {
		currentXML = _currentXML;
		currentXMLRaw = _currentXMLRaw;
	} else {
		delete _currentXMLDoc;
	}

	return tmp;
}*/

WorldSystem::WorldSystem(void)
	: weather(WorldWeather::None), bgmObj(nullptr) {}

WorldSystem::~WorldSystem(void)
{
    // SDL2_mixer's object
	if (bgmObj != nullptr)
		Mix_FreeMusic(bgmObj);
}

void WorldSystem::render(void)
{
	const auto SCREEN_WIDTH = game::SCREEN_WIDTH;
	const auto SCREEN_HEIGHT = game::SCREEN_HEIGHT;
	const auto HLINE = game::HLINE;

	const ivec2 backgroundOffset = ivec2 {
        static_cast<int>(SCREEN_WIDTH) / 2, static_cast<int>(SCREEN_HEIGHT) / 2
    };

	int iStart, iEnd, pOffset;

    // world width in pixels
	int width = world.data.size() * HLINE;

    // used for alpha values of background textures
    int alpha;


	static bool ambientUpdaterStarted = false;
	if (!ambientUpdaterStarted) {
		ambientUpdaterStarted = true;
		std::thread([&](void) {
			while (true) {
				float v = 75 * sin((game::time::getTickCount() + (DAY_CYCLE / 2)) / (DAY_CYCLE / PI));
				float rg = std::clamp(.5f + (-v / 100.0f), 0.01f, .9f);
				float b  = std::clamp(.5f + (-v / 80.0f), 0.03f, .9f);

				ambient = Color(rg, rg, b, 1.0f);

				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}).detach();
	}


	switch (weather) {
	case WorldWeather::Snowy:
		alpha = 150;
		break;
	case WorldWeather::Rain:
		alpha = 0;
		break;
	default:
		alpha = 255 - worldShade * 4;
		break;
	}

	// shade value for GLSL
	float shadeAmbient = std::max(0.0f, static_cast<float>(-worldShade) / 50 + 0.5f); // 0 to 1.5f

	if (shadeAmbient > 0.9f)
		shadeAmbient = 1;

    // draw background images.
    GLfloat tex_coord[] = { 0.0f, 1.0f,
                            1.0f, 1.0f,
                            1.0f, 0.0f,

                            1.0f, 0.0f,
                            0.0f, 0.0f,
                            0.0f, 1.0f,};

	// TODO scroll backdrop
	GLfloat bgOff = game::time::getTickCount()/24000.0f;

	GLfloat topS = .125f + bgOff;
	GLfloat bottomS = 0.0f + bgOff;

	if (topS < 0.0f) topS += 1.0f;
	if (bottomS < 0.0f) bottomS += 1.0f;
	if(bgOff < 0)std::cout << bottomS << "," << topS << std::endl;

	GLfloat scrolling_tex_coord[] = {0.0f,  bottomS,
									 1.0f,  bottomS,
									 1.0f,  bottomS,

									 1.0f,  bottomS,
									 0.0f,  bottomS,
									 0.0f,  bottomS};

   	vec2 bg_tex_coord[] = { vec2(0.0f, 0.0f),
                            vec2(1.0f, 0.0f),
                            vec2(1.0f, 1.0f),

                            vec2(1.0f, 1.0f),
                            vec2(0.0f, 1.0f),
                            vec2(0.0f, 0.0f)};

    GLfloat back_tex_coord[] = {offset.x - backgroundOffset.x - 5, offset.y - backgroundOffset.y, 9.9f,
                                offset.x + backgroundOffset.x + 5, offset.y - backgroundOffset.y, 9.9f,
                                offset.x + backgroundOffset.x + 5, offset.y + backgroundOffset.y, 9.9f,

                                offset.x + backgroundOffset.x + 5, offset.y + backgroundOffset.y, 9.9f,
                                offset.x - backgroundOffset.x - 5, offset.y + backgroundOffset.y, 9.9f,
                                offset.x - backgroundOffset.x - 5, offset.y - backgroundOffset.y, 9.9f};

    GLfloat fron_tex_coord[] = {offset.x - backgroundOffset.x - 5, offset.y + backgroundOffset.y, 9.8f,
                                offset.x + backgroundOffset.x + 5, offset.y + backgroundOffset.y, 9.8f,
                                offset.x + backgroundOffset.x + 5, offset.y - backgroundOffset.y, 9.8f,

                                offset.x + backgroundOffset.x + 5, offset.y - backgroundOffset.y, 9.8f,
                                offset.x - backgroundOffset.x - 5, offset.y - backgroundOffset.y, 9.8f,
                                offset.x - backgroundOffset.x - 5, offset.y + backgroundOffset.y, 9.8f};

	// rendering!!

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(Render::worldShader.uniform[WU_texture], 0);

	Render::worldShader.use();
	glUniform1f(Render::worldShader.uniform[WU_light_impact], 0.0f);
	glUniform4f(Render::worldShader.uniform[WU_ambient], 1.0, 1.0, 1.0, 1.0);

    Render::worldShader.enable();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    bgTex(0);
	glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.0);


	makeWorldDrawingSimplerEvenThoughAndyDoesntThinkWeCanMakeItIntoFunctions(0, back_tex_coord, scrolling_tex_coord, 6);

	bgTex++;
	glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.3 - static_cast<float>(alpha)/255.0f);

	makeWorldDrawingSimplerEvenThoughAndyDoesntThinkWeCanMakeItIntoFunctions(0, fron_tex_coord, tex_coord, 6);

	// TODO make stars dynamic
	/*static GLuint starTex = Texture::loadTexture("assets/style/classic/bg/star.png");
	const static float stardim = 24;
	GLfloat star_coord[star.size() * 5 * 6 + 1];
    GLfloat *si = &star_coord[0];

	if (worldShade > 0) {

		auto xcoord = offset.x * 0.9f;

		for (auto &s : star) {
			float data[30] = {
				s.x + xcoord, s.y, 9.7, 0, 0,
				s.x + xcoord + stardim, s.y, 9.7, 1, 0,
				s.x + xcoord + stardim, s.y + stardim, 9.7, 1, 1,
				s.x + xcoord + stardim, s.y + stardim, 9.7, 1, 1,
				s.x + xcoord, s.y + stardim, 9.7, 0, 1,
				s.x + xcoord, s.y, 9.7, 0, 0
			};

			std::memcpy(si, data, sizeof(float) * 30);
			si += 30;
		}
		glBindTexture(GL_TEXTURE_2D, starTex);
		glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.3 - static_cast<float>(alpha)/255.0f);

		makeWorldDrawingSimplerEvenThoughAndyDoesntThinkWeCanMakeItIntoFunctions(5 * sizeof(GLfloat), &star_coord[0], &star_coord[3], star.size() * 6);
	}*/

	Render::worldShader.disable();

	glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.0);
	glUniform4f(Render::worldShader.uniform[WU_ambient], ambient.red, ambient.green, ambient.blue, 1.0);

	Render::worldShader.unuse();

    std::vector<vec3> bg_items;
	std::vector<vec2> bg_tex;

	bgTex++;
	dim2 mountainDim = bgTex.getTextureDim();
    auto xcoord = width / 2 * -1 + offset.x * 0.85f;
	for (int i = 0; i <= width / mountainDim.x; i++) {
        bg_items.emplace_back(mountainDim.x * i       + xcoord, GROUND_HEIGHT_MINIMUM, 				 8.0f);
        bg_items.emplace_back(mountainDim.x * (i + 1) + xcoord, GROUND_HEIGHT_MINIMUM, 				 8.0f);
        bg_items.emplace_back(mountainDim.x * (i + 1) + xcoord, GROUND_HEIGHT_MINIMUM + mountainDim.y, 8.0f);

        bg_items.emplace_back(mountainDim.x * (i + 1) + xcoord, GROUND_HEIGHT_MINIMUM + mountainDim.y, 8.0f);
        bg_items.emplace_back(mountainDim.x * i       + xcoord, GROUND_HEIGHT_MINIMUM + mountainDim.y, 8.0f);
        bg_items.emplace_back(mountainDim.x * i       + xcoord, GROUND_HEIGHT_MINIMUM, 				 8.0f);
	}

    for (uint i = 0; i < bg_items.size()/6; i++) {
        for (auto &v : bg_tex_coord)
            bg_tex.push_back(v);
    }

    Render::worldShader.use();
	glUniform1f(Render::worldShader.uniform[WU_light_impact], 0.01);

	makeWorldDrawingSimplerEvenThoughAndyDoesntThinkWeCanMakeItIntoFunctions_JustDrawThis(0, bg_items.data(), bg_tex.data(), bg_items.size());

    Render::worldShader.unuse();

	// draw the remaining layers
	for (unsigned int i = 0; i < 4; i++) {
		bgTex++;
		auto dim = bgTex.getTextureDim();
		auto xcoord = offset.x * bgDraw[i][2];

		bg_items.clear();
		bg_tex.clear();

		if (world.indoor && i == 3) {
			glBindTexture(GL_TEXTURE_2D, world.indoorTex);

			const auto& startx = world.startX;

			bg_items.emplace_back(startx, GROUND_HEIGHT_MINIMUM, 7-(i*.1));
	        bg_items.emplace_back(startx + world.indoorWidth, GROUND_HEIGHT_MINIMUM,	7-(i*.1));
	        bg_items.emplace_back(startx + world.indoorWidth, GROUND_HEIGHT_MINIMUM + dim.y, 7-(i*.1));

	        bg_items.emplace_back(startx + world.indoorWidth, GROUND_HEIGHT_MINIMUM + dim.y, 7-(i*.1));
	        bg_items.emplace_back(startx, GROUND_HEIGHT_MINIMUM + dim.y, 7-(i*.1));
	        bg_items.emplace_back(startx, GROUND_HEIGHT_MINIMUM,	7-(i*.1));
		} else {
			for (int j = world.startX; j <= -world.startX; j += dim.x) {
	            bg_items.emplace_back(j         + xcoord, GROUND_HEIGHT_MINIMUM, 		7-(i*.1));
	            bg_items.emplace_back(j + dim.x + xcoord, GROUND_HEIGHT_MINIMUM, 		7-(i*.1));
	            bg_items.emplace_back(j + dim.x + xcoord, GROUND_HEIGHT_MINIMUM + dim.y, 7-(i*.1));

	            bg_items.emplace_back(j + dim.x + xcoord, GROUND_HEIGHT_MINIMUM + dim.y, 7-(i*.1));
	            bg_items.emplace_back(j         + xcoord, GROUND_HEIGHT_MINIMUM + dim.y, 7-(i*.1));
	            bg_items.emplace_back(j         + xcoord, GROUND_HEIGHT_MINIMUM, 		7-(i*.1));
			}
		}

   	    for (uint i = 0; i < bg_items.size()/6; i++) {
			for (auto &v : bg_tex_coord)
				bg_tex.push_back(v);
		}

        Render::worldShader.use();
		glUniform1f(Render::worldShader.uniform[WU_light_impact], 0.075f + (0.2f*i));

	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		makeWorldDrawingSimplerEvenThoughAndyDoesntThinkWeCanMakeItIntoFunctions_JustDrawThis(0, bg_items.data(), &bg_tex[0], bg_items.size());

        Render::worldShader.unuse();
	}

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // get the line that the player is currently standing on
    pOffset = (offset.x /*+ player->width / 2*/ - world.startX) / HLINE;

    // only draw world within player vision
    iStart = std::clamp(static_cast<int>(pOffset - (SCREEN_WIDTH / 2 / HLINE) - GROUND_HILLINESS),
	                    0, static_cast<int>(world.data.size()));
	iEnd = std::clamp(static_cast<int>(pOffset + (SCREEN_WIDTH / 2 / HLINE)),
                      0, static_cast<int>(world.data.size())) + 1;

    // draw the dirt
    bgTex++;
    std::vector<std::pair<vec2,vec3>> c;

    for (int i = iStart; i < iEnd; i++) {
        if (world.data[i].groundHeight <= 0) { // TODO holes (andy)
            world.data[i].groundHeight = GROUND_HEIGHT_MINIMUM - 1;
            glColor4ub(0, 0, 0, 255);
        } else {
            safeSetColorA(150, 150, 150, 255);
        }

        int ty = world.data[i].groundHeight / 64 + world.data[i].groundColor;

		c.push_back(std::make_pair(vec2(0, 0), vec3(world.startX + HLINES(i),         world.data[i].groundHeight - GRASS_HEIGHT, -4.0f)));
        c.push_back(std::make_pair(vec2(1, 0), vec3(world.startX + HLINES(i) + HLINE, world.data[i].groundHeight - GRASS_HEIGHT, -4.0f)));
        c.push_back(std::make_pair(vec2(1, ty),vec3(world.startX + HLINES(i) + HLINE, 0,                                         -4.0f)));

        c.push_back(std::make_pair(vec2(1, ty),vec3(world.startX + HLINES(i) + HLINE, 0,                                         -4.0f)));
        c.push_back(std::make_pair(vec2(0, ty),vec3(world.startX + HLINES(i),         0,                                         -4.0f)));
        c.push_back(std::make_pair(vec2(0, 0), vec3(world.startX + HLINES(i),         world.data[i].groundHeight - GRASS_HEIGHT, -4.0f)));

        if (world.data[i].groundHeight == GROUND_HEIGHT_MINIMUM - 1)
            world.data[i].groundHeight = 0;
    }

    std::vector<GLfloat> dirtc;
    std::vector<GLfloat> dirtt;

    for (auto &v : c) {
        dirtc.push_back(v.second.x);
        dirtc.push_back(v.second.y);
        dirtc.push_back(v.second.z);

        dirtt.push_back(v.first.x);
        dirtt.push_back(v.first.y);
    }

    Render::worldShader.use();
	glUniform1f(Render::worldShader.uniform[WU_light_impact], 0.45f);

    Render::worldShader.enable();

    glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 0, &dirtc[0]);
    glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 0, &dirtt[0]);
    glDrawArrays(GL_TRIANGLES, 0 , c.size());

    Render::worldShader.disable();
	Render::worldShader.unuse();

	if (!world.indoor) {
		bgTex++;
	    safeSetColorA(255, 255, 255, 255);

	    c.clear();
	    std::vector<GLfloat> grassc;
	    std::vector<GLfloat> grasst;

		for (int i = iStart; i < iEnd; i++) {
        	auto wd = world.data[i];
	        auto gh = wd.grassHeight;

			// flatten the grass if the player is standing on it.
			if (!wd.grassUnpressed) {
				gh[0] /= 4;
				gh[1] /= 4;
			}

			// actually draw the grass.
	        if (wd.groundHeight) {
				const auto& worldStart = world.startX;

	            c.push_back(std::make_pair(vec2(0, 0),vec3(worldStart + HLINES(i)            , wd.groundHeight + gh[0], 		-3)));
	            c.push_back(std::make_pair(vec2(1, 0),vec3(worldStart + HLINES(i) + HLINE / 2, wd.groundHeight + gh[0], 		-3)));
	            c.push_back(std::make_pair(vec2(1, 1),vec3(worldStart + HLINES(i) + HLINE / 2, wd.groundHeight - GRASS_HEIGHT, 	-3)));

    	        c.push_back(std::make_pair(vec2(1, 1),vec3(worldStart + HLINES(i) + HLINE / 2, wd.groundHeight - GRASS_HEIGHT,	-3)));
	            c.push_back(std::make_pair(vec2(0, 1),vec3(worldStart + HLINES(i)		     , wd.groundHeight - GRASS_HEIGHT,	-3)));
	            c.push_back(std::make_pair(vec2(0, 0),vec3(worldStart + HLINES(i)            , wd.groundHeight + gh[0],			-3)));

	            c.push_back(std::make_pair(vec2(0, 0),vec3(worldStart + HLINES(i) + HLINE / 2, wd.groundHeight + gh[1],			-3)));
	            c.push_back(std::make_pair(vec2(1, 0),vec3(worldStart + HLINES(i) + HLINE    , wd.groundHeight + gh[1],			-3)));
	            c.push_back(std::make_pair(vec2(1, 1),vec3(worldStart + HLINES(i) + HLINE    , wd.groundHeight - GRASS_HEIGHT,	-3)));

    	        c.push_back(std::make_pair(vec2(1, 1),vec3(worldStart + HLINES(i) + HLINE    , wd.groundHeight - GRASS_HEIGHT,	-3)));
	            c.push_back(std::make_pair(vec2(0, 1),vec3(worldStart + HLINES(i) + HLINE / 2, wd.groundHeight - GRASS_HEIGHT,	-3)));
	            c.push_back(std::make_pair(vec2(0, 0),vec3(worldStart + HLINES(i) + HLINE / 2, wd.groundHeight + gh[1],			-3)));
	        }
		}

	    for (auto &v : c) {
	        grassc.push_back(v.second.x);
	        grassc.push_back(v.second.y);
	        grassc.push_back(v.second.z);

        	grasst.push_back(v.first.x);
    	    grasst.push_back(v.first.y);
	    }

	    Render::worldShader.use();
		glUniform1f(Render::worldShader.uniform[WU_light_impact], 1.0f);

	    Render::worldShader.enable();

	    glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 0, &grassc[0]);
	    glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 0, &grasst[0]);
	    glDrawArrays(GL_TRIANGLES, 0 , c.size());

    	Render::worldShader.disable();
		Render::worldShader.unuse();
	} else {
		Render::useShader(&Render::worldShader);
		Render::worldShader.use();
		static const GLuint rug = Texture::genColor(Color {255, 0, 0});
		glBindTexture(GL_TEXTURE_2D, rug);
		vec2 ll = vec2 {world.startX, GROUND_HEIGHT_MINIMUM};
		Render::drawRect(ll, vec2 {ll.x + world.indoorWidth, ll.y + 4}, -3);
		Render::worldShader.unuse();
	}

	//player->draw();
}

void WorldSystem::receive(const BGMToggleEvent &bte)
{
	if (bte.world == nullptr || world.bgm != bte.file) {
		Mix_FadeOutMusic(800);

		if (bgmObj != nullptr)
			Mix_FreeMusic(bgmObj);

		//worldBgmFile = bte.file;
		bgmObj = Mix_LoadMUS(world.bgm.c_str());
		Mix_PlayMusic(bgmObj, -1);
	}
}

void WorldSystem::setWeather(const std::string &s)
{
	for (unsigned int i = 3; i--;) {
		if (WorldWeatherString[i] == s) {
			weather = static_cast<WorldWeather>(i);
			return;
		}
	}

	weather = WorldWeather::None;
}

void WorldSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)en;
	(void)ev;
	(void)dt;

    // fade in music if not playing
	if (bgmObj != nullptr && !Mix_PlayingMusic())
		Mix_FadeInMusic(bgmObj, -1, 2000);

	// run detect stuff
	detect(dt);
}

void WorldSystem::detect(entityx::TimeDelta dt)
{
	game::entities.each<Position, Direction, Solid>(
	    [&](entityx::Entity e, Position &loc, Direction &vel, Solid &dim) {
		(void)e;

		//if (health.health <= 0)
		//	UserError("die mofo");

		// get the line the entity is on
		int line = std::clamp(static_cast<int>((loc.x + dim.width / 2 - world.startX) / game::HLINE),
		                      0,
		                      static_cast<int>(world.data.size()));

		// make sure entity is above ground
		const auto& data = world.data;
		if (loc.y < data[line].groundHeight) {
			int dir = vel.x < 0 ? -1 : 1;
			if (line + dir * 2 < static_cast<int>(data.size()) &&
			    data[line + dir * 2].groundHeight - 30 > data[line + dir].groundHeight) {
				loc.x -= (PLAYER_SPEED_CONSTANT + 2.7f) * dir * 2;
				vel.x = 0;
			} else {
				loc.y = data[line].groundHeight - 0.001f * dt;
				vel.y = 0;
				// TODO ground flag
			}
		}

        // handle gravity
        else if (vel.y > -2.0f) {
			vel.y -= GRAVITY_CONSTANT * dt;
		}

		// insure that the entity doesn't fall off either edge of the world.
        if (loc.x < world.startX) {
			vel.x = 0;
			loc.x = world.startX + HLINES(0.5f);
		} else if (loc.x + dim.width + game::HLINE > -((int)world.startX)) {
			vel.x = 0;
			loc.x = -((int)world.startX) - dim.width - game::HLINE;
		}
	});
}

void WorldSystem::goWorldRight(Position& p)
{
	if (!(world.toRight.empty()) && (p.x > world.startX * -1 - HLINES(10))) {
		ui::toggleBlack();
		ui::waitForCover();
		auto file = world.toRight;
		load(file);
		p.x = world.startX + HLINES(15);
		ui::toggleBlack();
	}
}

void WorldSystem::goWorldLeft(Position& p)
{
	if (!(world.toLeft.empty()) && (p.x < world.startX + HLINES(10))) {
		ui::toggleBlack();
		ui::waitForCover();
		auto file = world.toLeft;
		load(file);
		p.x = world.startX * -1 - HLINES(15);
		ui::toggleBlack();
	}
}
