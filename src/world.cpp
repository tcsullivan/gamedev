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
#include <chrono>
using namespace std::literals::chrono_literals;

// local game headers
#include <ui.hpp>
#include <gametime.hpp>

#include <render.hpp>
#include <engine.hpp>
#include <components.hpp>
#include <player.hpp>
#include <weather.hpp>
#include <particle.hpp>

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

// wait
static bool waitToSwap = false;

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
const unsigned int GRASS_HEIGHT = HLINES(4);

// the path of the currently loaded XML file, externally referenced in places
std::string currentXML;

// pathnames of images for world themes
using StyleList = std::array<std::string, 8>;
static const std::vector<StyleList> bgPaths = {
	{ // Forest
		"bg.png", 				// sky/background
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

/* ----------------------------------------------------------------------------
** Functions section
** --------------------------------------------------------------------------*/

void WorldSystem::generate(int width)
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
	        w->grassHeight[0] = (randGet() % 16) / 3 + HLINES(2);
	        w->grassHeight[1] = (randGet() % 16) / 3 + HLINES(2);
	    }
	}

    // define x-coordinate of world's leftmost 'line'
    world.startX = HLINES(width * -0.5);
}

float WorldSystem::isAboveGround(const vec2& p) const
{
	int line = std::clamp(static_cast<int>((p.x - world.startX) / game::HLINE),
		0, static_cast<int>(world.data.size()));

	const auto& gh = world.data[line].groundHeight;
	return p.y >= gh ? 0 : gh;
}

static Color ambient;

bool WorldSystem::save(void)
{
	if (world.indoor)
		return false;

	std::ofstream save (xmlFolder + currentXMLFile + ".dat");

	// signature?
	save << "831998 ";

	game::entities.each<Position>([&](entityx::Entity entity, Position& pos) {
		// save position
		save << "p " << pos.x << ' ' << pos.y << ' ';

		// save dialog, if exists
		if (entity.has_component<Dialog>())
			save << "d " << entity.component<Dialog>()->index << ' ';
	});

	save.close();
	return true;
}

void WorldSystem::load(const std::string& file)
{
	auto& render = *game::engine.getSystem<RenderSystem>();

	entityx::Entity entity;

	// check for empty file name
	if (file.empty())
		return;

	// load file data to string
	auto xmlPath = xmlFolder + file;
	auto xmlRaw = readFile(xmlPath);

	// let tinyxml parse the file
	if (xmlDoc.Parse(xmlRaw.data()) != XML_NO_ERROR)
		UserError("XML Error: Failed to parse file (not your fault though..?)");

	// include headers
	auto ixml = xmlDoc.FirstChildElement("include");
	while (ixml != nullptr) {
		auto file = ixml->Attribute("file");

		if (file != nullptr) {
			DEBUG_printf("Including file: %s\n", file);
			xmlRaw.append(readFile(xmlFolder + file));
		} else {
			UserError("XML Error: <include> tag file not given");
		}

		break;//ixml = ixml->NextSiblingElement();
	}

	if (xmlDoc.Parse(xmlRaw.data()) != XML_NO_ERROR)
		UserError("XML Error:");

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
	game::entities.reset();
	game::engine.getSystem<PlayerSystem>()->create();

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

			const auto& files = bgPaths[static_cast<int>(world.style)];

			for (const auto& f : files)
				bgFiles.push_back(world.styleFolder + "bg/" + f);

			bgTex = TextureIterator(bgFiles);
		}

        // world generation
        else if (tagName == "generation") {
			generate(wxml->IntAttribute("width"));
		}

		// indoor stuff
		else if (tagName == "house") {
			if (!world.indoor)
				UserError("<house> can only be used inside <IndoorWorld>");

			//world.indoorWidth = wxml->FloatAttribute("width");
			world.indoorTex = render.loadTexture(wxml->StrAttribute("texture"));
			//auto str = wxml->StrAttribute("texture");
			//auto tex = render.loadTexture(str);
			//world.indoorTex = tex;
		}

		// weather tag
		else if (tagName == "weather") {
			game::engine.getSystem<WeatherSystem>()->setWeather(wxml->GetText());
			//setWeather(wxml->GetText());
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
						auto sprx = abcd;
						auto frames = developFrame(sprx);
						if (frames.size() > 0)
							sprite->sprite = frames.at(0);
					} else if (tname == "Portal") {
						entity.assign<Portal>(wxml->StrAttribute("inside"));
					} else if (tname == "Solid") {
						vec2 dim;

						if (abcd->Attribute("value") != nullptr)
							dim = str2coord(abcd->StrAttribute("value"));
						else
							dim = entity.component<Sprite>()->getSpriteSize();

						float cdat[2] = {dim.x, dim.y};
						entity.assign<Solid>(cdat[0], cdat[1]);
					} else if (tname == "Direction") {
						vec2 dir;

						if (wxml->Attribute("direction") != nullptr) {
							dir = str2coord(wxml->StrAttribute("direction"));
						} else if (wxml->Attribute("value") != nullptr) {
							dir = str2coord(wxml->StrAttribute("value"));
						} else {
							dir = vec2(0,0);
						}

						float cdat[2] = {dir.x, dir.y};
						entity.assign<Direction>(cdat[0], cdat[1]);
					} else if (tname == "Physics") {
						float g;

						if (wxml->Attribute("gravity") != nullptr) {
							g = wxml->FloatAttribute("gravity");
						} else if (wxml->Attribute("value") != nullptr) {
							g = wxml->FloatAttribute("value");
						} else {
							g = 1.0f;
						}

						entity.assign<Physics>(g);
					} else if (tname == "Name") {
						entity.assign<Name>(coalesce(wxml->Attribute("name"), abcd->Attribute("value")));
					} else if (tname == "Dialog") {
						entity.assign<Dialog>((wxml->BoolAttribute("hasDialog") ? 0 : 9999));
					} else if (tname == "Grounded") {
						entity.assign<Grounded>();
					} else if (tname == "Wander") {
						entity.assign<Wander>();
					} else if (tname == "Hop" ) {
						entity.assign<Hop>();
					} else if (tname == "Animation") {
						auto entan = entity.assign<Animate>();
						auto animx = abcd->FirstChildElement();
						
						uint limbid 		= 0;
						float limbupdate 	= 0;
						uint limbupdatetype = 0;
						
						while (animx) {	
							std::string animType = animx->Name();
							if (animType == "movement") {
								limbupdatetype = 1;
								auto limbx = animx->FirstChildElement();
								while (limbx) {
									std::string limbHopefully = limbx->Name();
									if (limbHopefully == "limb") {
										auto frames = developFrame(limbx);
										
										entan->limb.push_back(Limb());
										entan->limb.back().updateType = limbupdatetype;

										if (limbx->QueryUnsignedAttribute("id", &limbid) == XML_NO_ERROR) {
											entan->limb.back().limbID = limbid;
										}
										if (limbx->QueryFloatAttribute("update", &limbupdate) == XML_NO_ERROR) {
											entan->limb.back().updateRate = limbupdate;
										}
										
										// place our newly developed frames in the entities animation stack
										for (auto &f : frames) {
											entan->limb.back().addFrame(f);
											for (auto &fr : entan->limb.back().frame) {
												for (auto &sd : fr)
													sd.first.limb = limbid;
											}
										}
									}
								limbx = limbx->NextSiblingElement();
								}
							}
							
							animx = animx->NextSiblingElement();
						}
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

	// attempt to load data
	std::ifstream save (xmlFolder + currentXMLFile + ".dat");
	if (save.good()) {
		// check signature
		int signature;
		save >> signature;
		if (signature != 831998)
			UserError("Save file signature is invalid... (delete it)");

		char id;
		save >> id;

		entityx::ComponentHandle<Position> pos;
		for (entityx::Entity entity : game::entities.entities_with_components(pos)) {
			save >> pos->x >> pos->y;
			save >> id;

			while (id != 'p') {
				switch (id) {
				case 'd':
					save >> entity.component<Dialog>()->index;
					break;
				}

				save >> id;
			}
		}

		save.close();
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
	: bgmObj(nullptr) {}

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

	const ivec2 backgroundOffset = ivec2 {
        static_cast<int>(SCREEN_WIDTH) / 2, static_cast<int>(SCREEN_HEIGHT) / 2
    };

	int iStart, iEnd, pOffset;

    // world width in pixels
	int width = HLINES(world.data.size());

	static bool ambientUpdaterStarted = false;
	if (!ambientUpdaterStarted) {
		ambientUpdaterStarted = true;
		thAmbient = std::thread([&](void) {
			const bool &run = game::engine.shouldRun;
			while (run) {
				float v = 75 * sin((game::time::getTickCount() + (DAY_CYCLE / 2)) / (DAY_CYCLE / PI));
				float rg = std::clamp(.5f + (-v / 100.0f), 0.01f, .9f);
				float b  = std::clamp(.5f + (-v / 80.0f), 0.03f, .9f);

				ambient = Color(rg, rg, b, 1.0f);

				std::this_thread::sleep_for(1ms);
			}
		});
		thAmbient.detach();
	}

	// shade value for GLSL
	float shadeAmbient = std::max(0.0f, static_cast<float>(-worldShade) / 50 + 0.5f); // 0 to 1.5f

	if (shadeAmbient > 0.9f)
		shadeAmbient = 1;

	// TODO scroll backdrop
	GLfloat bgOff = game::time::getTickCount() / static_cast<float>(DAY_CYCLE * 2);

	GLfloat topS = .125f + bgOff;
	GLfloat bottomS = 0.0f + bgOff;

	if (topS < 0.0f)
		topS += 1.0f;
	if (bottomS < 0.0f)
		bottomS += 1.0f;

	GLfloat scrolling_tex_coord[] = {
		0.0f,  bottomS,
		1.0f,  bottomS,
		1.0f,  bottomS,

		1.0f,  bottomS,
		0.0f,  bottomS,
		0.0f,  bottomS
	};

   	static const vec2 bg_tex_coord[] = {
		vec2(0.0f, 0.0f),
        vec2(1.0f, 0.0f),
        vec2(1.0f, 1.0f),

        vec2(1.0f, 1.0f),
        vec2(0.0f, 1.0f),
        vec2(0.0f, 0.0f)
	};

    GLfloat back_tex_coord[] = {
		offset.x - backgroundOffset.x - 5, offset.y - backgroundOffset.y, 9.9f,
        offset.x + backgroundOffset.x + 5, offset.y - backgroundOffset.y, 9.9f,
        offset.x + backgroundOffset.x + 5, offset.y + backgroundOffset.y, 9.9f,

        offset.x + backgroundOffset.x + 5, offset.y + backgroundOffset.y, 9.9f,
        offset.x - backgroundOffset.x - 5, offset.y + backgroundOffset.y, 9.9f,
        offset.x - backgroundOffset.x - 5, offset.y - backgroundOffset.y, 9.9f
	};

	// rendering!!

    glActiveTexture(GL_TEXTURE0);
	Render::worldShader.use();
    glUniform1i(Render::worldShader.uniform[WU_texture], 0);
	glUniform1f(Render::worldShader.uniform[WU_light_impact], 0.0f);
	glUniform4f(Render::worldShader.uniform[WU_ambient], 1.0, 1.0, 1.0, 1.0);
    Render::worldShader.enable();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    bgTex(0);
	glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.0);

	makeWorldDrawingSimplerEvenThoughAndyDoesntThinkWeCanMakeItIntoFunctions(0, back_tex_coord, scrolling_tex_coord, 6);
	// no more night bg
	//bgTex++;
	//glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.3 - static_cast<float>(alpha) / 255.0f);
	//makeWorldDrawingSimplerEvenThoughAndyDoesntThinkWeCanMakeItIntoFunctions(0, fron_tex_coord, tex_coord, 6);

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
	auto mountainDim = bgTex.getTextureDim();
	mountainDim.x = HLINES(mountainDim.x);
	mountainDim.y = HLINES(mountainDim.y);
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
	for (int i = 0; i < 4; i++) {
		bgTex++;
		auto xcoord = offset.x * bgDraw[i][2];

		bg_items.clear();
		bg_tex.clear();

		vec2 dim = bgTex.getTextureDim() * game::HLINE;

		if (world.indoor && i == 3) {
			world.indoorTex.use();

			const auto& startx = world.startX;
			dim = world.indoorTex.getDim() * game::HLINE;

			bg_items.emplace_back(startx,         GROUND_HEIGHT_MINIMUM,         7 - (i * 0.1f));
	        bg_items.emplace_back(startx + dim.x, GROUND_HEIGHT_MINIMUM,	     7 - (i * 0.1f));
	        bg_items.emplace_back(startx + dim.x, GROUND_HEIGHT_MINIMUM + dim.y, 7 - (i * 0.1f));

	        bg_items.emplace_back(startx + dim.x, GROUND_HEIGHT_MINIMUM + dim.y, 7 - (i * 0.1f));
	        bg_items.emplace_back(startx,         GROUND_HEIGHT_MINIMUM + dim.y, 7 - (i * 0.1f));
	        bg_items.emplace_back(startx,         GROUND_HEIGHT_MINIMUM,         7 - (i * 0.1f));
		} else {
			for (int j = world.startX; j <= -world.startX; j += dim.x) {
	            bg_items.emplace_back(j         + xcoord, GROUND_HEIGHT_MINIMUM,         7 - (i * 0.1f));
	            bg_items.emplace_back(j + dim.x + xcoord, GROUND_HEIGHT_MINIMUM,         7 - (i * 0.1f));
	            bg_items.emplace_back(j + dim.x + xcoord, GROUND_HEIGHT_MINIMUM + dim.y, 7 - (i * 0.1f));

	            bg_items.emplace_back(j + dim.x + xcoord, GROUND_HEIGHT_MINIMUM + dim.y, 7 - (i * 0.1f));
	            bg_items.emplace_back(j         + xcoord, GROUND_HEIGHT_MINIMUM + dim.y, 7 - (i * 0.1f));
	            bg_items.emplace_back(j         + xcoord, GROUND_HEIGHT_MINIMUM,         7 - (i * 0.1f));
			}
		}

   	    for (uint i = 0; i < bg_items.size() / 6; i++) {
			for (auto &v : bg_tex_coord)
				bg_tex.push_back(v);
		}

        Render::worldShader.use();
		glUniform1f(Render::worldShader.uniform[WU_light_impact], 0.075f + (0.2f * i));

	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		makeWorldDrawingSimplerEvenThoughAndyDoesntThinkWeCanMakeItIntoFunctions_JustDrawThis(0, bg_items.data(), &bg_tex[0], bg_items.size());

        Render::worldShader.unuse();
	}

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // get the line that the player is currently standing on
    pOffset = (offset.x - world.startX) / game::HLINE;

    // only draw world within player vision
    iStart = std::clamp(static_cast<int>(pOffset - (SCREEN_WIDTH / 2 / game::HLINE) - GROUND_HILLINESS),
	                    0, static_cast<int>(world.data.size()));
	iEnd = std::clamp(static_cast<int>(pOffset + (SCREEN_WIDTH / 2 / game::HLINE) + 2),
                      0, static_cast<int>(world.data.size()));

    // draw the dirt
	waitToSwap = true;

    bgTex++;

	static std::vector<GLfloat> dirt;
	if (dirt.size() != world.data.size() * 30) {
		dirt.clear();
		dirt.resize(world.data.size() * 30);
	}

	auto push5 = [](GLfloat *&vec, GLfloat *five) {
		for (int i = 0; i < 5; i++)
			*vec++ = *five++;
	};

	GLfloat *dirtp = &dirt[0];
    for (int i = iStart; i < iEnd; i++) {
        if (world.data[i].groundHeight <= 0) { // TODO holes (andy) TODO TODO TODO
            world.data[i].groundHeight = GROUND_HEIGHT_MINIMUM - 1;
            //glColor4ub(0, 0, 0, 255);
        } else {
            //safeSetColorA(150, 150, 150, 255);
        }

        int ty = world.data[i].groundHeight / 64 + world.data[i].groundColor;

		GLfloat five[5] = {
			0, 0, world.startX + HLINES(i), world.data[i].groundHeight - GRASS_HEIGHT, -4
		};

		push5(dirtp, five);
		five[0]++, five[2] += game::HLINE;
		push5(dirtp, five);
		five[1] += ty, five[3] = 0;
		push5(dirtp, five);
		push5(dirtp, five);
		five[0]--, five[2] -= game::HLINE;
		push5(dirtp, five);
		five[1] = 0, five[3] = world.data[i].groundHeight - GRASS_HEIGHT;
		push5(dirtp, five);

        if (world.data[i].groundHeight == GROUND_HEIGHT_MINIMUM - 1)
            world.data[i].groundHeight = 0;
    }

    Render::worldShader.use();
	glUniform1f(Render::worldShader.uniform[WU_light_impact], 0.45f);

    Render::worldShader.enable();

    glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &dirt[2]);
    glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &dirt[0]);
    glDrawArrays(GL_TRIANGLES, 0 , dirt.size() / 5);

    Render::worldShader.disable();
	Render::worldShader.unuse();

	if (!world.indoor) {
		bgTex++;
	    //safeSetColorA(255, 255, 255, 255); TODO TODO TODO 

		static std::vector<GLfloat> grass;
		if (grass.size() != world.data.size() * 60) {
			grass.clear();
			grass.resize(world.data.size() * 60);
		}

		GLfloat *grassp = &grass[0];
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
				float five[5] = {
					0, 1, world.startX + HLINES(i), wd.groundHeight + gh[0], -3
				};

				push5(grassp, five);
				five[0]++, five[1]--, five[2] += HLINES(0.5f);
				push5(grassp, five);
				five[1]++, five[3] = wd.groundHeight - GRASS_HEIGHT;
				push5(grassp, five);
				push5(grassp, five);
				five[0]--, five[2] -= HLINES(0.5f);
				push5(grassp, five);
				five[1]--, five[3] = wd.groundHeight + gh[0];
				push5(grassp, five);
				five[1]++;

				five[2] = world.startX + HLINES(i + 0.5), five[3] = wd.groundHeight + gh[1];

				push5(grassp, five);
				five[0]++, five[1]--, five[2] += HLINES(0.5f) + 1;
				push5(grassp, five);
				five[1]++, five[3] = wd.groundHeight - GRASS_HEIGHT;
				push5(grassp, five);
				push5(grassp, five);
				five[0]--, five[2] -= HLINES(0.5f) + 1;
				push5(grassp, five);
				five[1]--, five[3] = wd.groundHeight + gh[1];
				push5(grassp, five);
	        }
		}

	    Render::worldShader.use();
		glUniform1f(Render::worldShader.uniform[WU_light_impact], 1.0f);

	    Render::worldShader.enable();

	    glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &grass[2]);
	    glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &grass[0]);
	    glDrawArrays(GL_TRIANGLES, 0 , grass.size() / 5);

		// the starting pixel of the world
		static const float s = -(static_cast<float>(SCREEN_WIDTH) / 2.0f);
		// the ending pixel of the world
		static const float e = static_cast<float>(SCREEN_WIDTH) / 2.0f;

		static const float sheight = static_cast<float>(SCREEN_HEIGHT);
			

		if (offset.x + world.startX > s) {
			Colors::black.use();
			glUniform1f(Render::worldShader.uniform[WU_light_impact], 0.0f);

			auto off = offset.y - static_cast<float>(SCREEN_HEIGHT) / 2.0f;

			GLfloat blackBarLeft[] = {
				s,            0.0f    + off,    -3.5f, 0.0f, 0.0f,
				world.startX, 0.0f    + off,    -3.5f, 1.0f, 0.0f,
				world.startX, sheight + off, -3.5f, 1.0f, 1.0f,

				world.startX, sheight + off, -3.5f, 1.0f, 1.0f,
    			s,            sheight + off, -3.5f, 0.0f, 1.0f,
				s,            0.0f 	  + off,    -3.5f, 0.0f, 0.0f
			};

	    	glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, &blackBarLeft[0]);
	    	glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, &blackBarLeft[3]);
	    	glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		if (offset.x - world.startX < e) {
			Colors::black.use();
			glUniform1f(Render::worldShader.uniform[WU_light_impact], 0.0f);
		
			auto off = offset.y - static_cast<float>(SCREEN_HEIGHT) / 2.0f;

			GLfloat blackBarRight[] = {
				-(world.startX), 0.0f    + off,    -3.5f, 0.0f, 0.0f,
				e,               0.0f    + off,    -3.5f, 1.0f, 0.0f,
				e,               sheight + off, -3.5f, 1.0f, 1.0f,

				e,               sheight + off, -3.5f, 1.0f, 1.0f,
    			-(world.startX), sheight + off, -3.5f, 0.0f, 1.0f,
				-(world.startX), 0.0f    + off,    -3.5f, 0.0f, 0.0f
			};

	    	glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, &blackBarRight[0]);
	    	glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, &blackBarRight[3]);
	    	glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		Render::worldShader.disable();
		Render::worldShader.unuse();

	} else {
		Render::useShader(&Render::worldShader);
		Render::worldShader.use();
		Colors::red.use();
		vec2 ll = vec2 {world.startX, GROUND_HEIGHT_MINIMUM};
		Render::drawRect(ll, ll + vec2(world.indoorTex.getDim().x, 4), -3);
		Render::worldShader.unuse();
	}

	waitToSwap = false;
}

void WorldSystem::receive(const BGMToggleEvent &bte)
{
	if (bte.world == nullptr || world.bgm != bte.file) {
		if (bgmCurrent == world.bgm)
			return;

		Mix_FadeOutMusic(800);

		if (bgmObj != nullptr)
			Mix_FreeMusic(bgmObj);

		bgmCurrent = world.bgm;
		bgmObj = Mix_LoadMUS(world.bgm.c_str());
		Mix_PlayMusic(bgmObj, -1);
	}
}

void WorldSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)en;
	(void)ev;

    // fade in music if not playing
	if (!Mix_PlayingMusic() && bgmObj != nullptr)
		Mix_FadeInMusic(bgmObj, -1, 2000);

	// run detect stuff
	detect(dt);
}

void WorldSystem::detect(entityx::TimeDelta dt)
{
	game::entities.each<Grounded, Position, Solid>(
		[&](entityx::Entity e, Grounded &g, Position &loc, Solid &dim) {
			(void)e;
			if (!g.grounded) {
				// get the line the entity is on
				int line = std::clamp(static_cast<int>((loc.x + dim.width / 2 - world.startX) / game::HLINE),
									  0,
									  static_cast<int>(world.data.size()));

				// make sure entity is above ground
				const auto& data = world.data;
				if (loc.y != data[line].groundHeight) {
					loc.y = data[line].groundHeight;
					e.remove<Grounded>();
				}
			}

		});

	game::entities.each<Direction, Physics>(
		[&](entityx::Entity e, Direction &vel, Physics &phys) {
			(void)e;
			// handle gravity
        	if (vel.y > -2.0f)
				vel.y -= (GRAVITY_CONSTANT * phys.g) * dt;
		});

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
				if (!vel.grounded) {
					vel.grounded = true;
					game::engine.getSystem<ParticleSystem>()->addMultiple(20, ParticleType::SmallPoof,
						[&](){ return vec2(loc.x + randGet() % static_cast<int>(dim.width), loc.y);}, 500, 30);
				}
			}
		}


		// insure that the entity doesn't fall off either edge of the world.
        if (loc.x < world.startX) {
			vel.x = 0;
			loc.x = world.startX + HLINES(0.5f);
		} else if (loc.x + dim.width + game::HLINE > -(static_cast<int>(world.startX))) {
			vel.x = 0;
			loc.x = -(static_cast<int>(world.startX)) - dim.width - game::HLINE;
		}
	});
}

void WorldSystem::goWorldRight(Position& p, Solid &d)
{
	if (!(world.toRight.empty()) && (p.x + d.width > world.startX * -1 - HLINES(5))) {
		ui::toggleBlack();
		ui::waitForCover();
		while (waitToSwap)
			std::this_thread::sleep_for(1ms);
		load(world.toRight);
		game::engine.getSystem<PlayerSystem>()->setX(world.startX + HLINES(10));
		ui::toggleBlack();
	}
}

void WorldSystem::goWorldLeft(Position& p)
{
	if (!(world.toLeft.empty()) && (p.x < world.startX + HLINES(10))) {
		ui::toggleBlack();
		ui::waitForCover();
		while (waitToSwap)
			std::this_thread::sleep_for(1ms);
		load(world.toLeft);
		game::engine.getSystem<PlayerSystem>()->setX(world.startX * -1 - HLINES(15));
		ui::toggleBlack();
	}
}

void WorldSystem::goWorldPortal(Position& p)
{
	std::string file;

	if (world.indoor) {
		file = world.outdoor;
		p.x = world.outdoorCoords.x; // ineffective, player is regen'd
		p.y = world.outdoorCoords.y;
	} else {
		game::entities.each<Position, Solid, Portal>(
			[&](entityx::Entity entity, Position& loc, Solid &dim, Portal &portal) {
				(void)entity;
				if (!(portal.toFile.empty()) && p.x > loc.x && p.x < loc.x + dim.width)  {
					file = portal.toFile;
					world.outdoor = currentXMLFile;
					world.outdoorCoords = vec2(loc.x + dim.width / 2, 100);
					return;
				}
			}
		);
	}

	if (!file.empty()) {
		ui::toggleBlack();
		ui::waitForCover();
		while (waitToSwap)
			std::this_thread::sleep_for(1ms);
		load(file);
		ui::toggleBlack();
	}
}
