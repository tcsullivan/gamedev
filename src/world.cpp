#include <world.hpp>

// standard library headers
#include <algorithm>
#include <fstream>
#include <memory>
#include <sstream>

#include <chrono>
using namespace std::literals::chrono_literals;

// local library headers
#include <tinyxml2.h>
using namespace tinyxml2;

// game headers
#include <common.hpp>
#include <components.hpp>
#include <debug.hpp>
#include <engine.hpp>
#include <error.hpp>
#include <fileio.hpp>
#include <gametime.hpp>
#include <player.hpp>
#include <particle.hpp>
#include <render.hpp>
#include <ui.hpp>
#include <vector3.hpp>
#include <weather.hpp>
#include <systems/lua.hpp>

WorldData         WorldSystem::world;
Mix_Music*        WorldSystem::bgmObj;
std::string       WorldSystem::bgmCurrent;
TextureIterator   WorldSystem::bgTex;
XMLDocument       WorldSystem::xmlDoc;
std::string       WorldSystem::currentXMLFile;
std::thread       WorldSystem::thAmbient;
std::vector<vec2> WorldSystem::stars;
std::string       WorldSystem::toLoad;

// wait
static bool waitToSwap = false;

// externally referenced in main.cpp
int worldShade = 0;

WorldSystem::WorldSystem(void)
{
	bgmObj = nullptr;
}

WorldSystem::~WorldSystem(void)
{
}

void WorldSystem::generate(const char *file)
{
	world.ground = ObjectTexture(file);
	auto width = world.ground.getDim().x;
	world.startX = width / -2;

	// gen. star coordinates
	if (stars.empty()) {
		stars.resize(game::SCREEN_WIDTH / 30);
		for (auto& s : stars) {
			s.x = world.startX + (randGet() % (int)width);
			s.y = game::SCREEN_HEIGHT - (randGet() % (int)HLINES(game::SCREEN_HEIGHT / 1.3f));
		}
	}
}

float WorldSystem::getGroundHeight(float x)
{
	return world.ground.getHeight((int)(x - world.startX));
}

float WorldSystem::isAboveGround(const vec2& p) 
{
	auto gh = getGroundHeight(p.x);
	return p.y >= gh ? 0 : gh;
}

bool WorldSystem::save(void)
{	
	std::ofstream save (game::config::xmlFolder + currentXMLFile + ".dat");

	// signature?
	save << "831998 ";

	game::entities.each<Position>([&](entityx::Entity entity, Position& pos) {
		// save position
		save << "p " << pos.x << ' ' << pos.y << ' ';

		// save dialog, if exists
		if (entity.has_component<Dialog>())
			save << "d " << entity.component<Dialog>()->index << ' ';

		if (entity.has_component<Health>())
			save << "h " << entity.component<Health>()->health << ' ';
	}, true);

	save.close();
	return true;
}

static std::vector<entityx::Entity::Id> savedEntities;

void WorldSystem::fight(entityx::Entity entity)
{
	UserError("fights unimplemented?");
	std::string exit = currentXMLFile;

	savedEntities.emplace_back(entity.id());
	//load(entity.component<Aggro>()->arena);
	savedEntities.clear();

	entity.component<Health>()->health = entity.component<Health>()->maxHealth;
	entity.remove<Aggro>();

	auto door = game::entities.create();
	door.assign<Position>(0, 100);
	door.assign<Grounded>();
	door.assign<Visible>(-5);
	door.assign<Portal>(exit);

	auto sprite = door.assign<Sprite>();
	auto dtex = RenderSystem::loadTexture("assets/style/classic/door.png");
	sprite->addSpriteSegment(SpriteData(dtex), 0);

	auto dim = HLINES(sprite->getSpriteSize());
	door.assign<Solid>(dim.x, dim.y);
}

void WorldSystem::load(const std::string& file)
{
	toLoad = file;
}

void WorldSystem::loader(void)
{
	entityx::Entity entity;

	// check for empty file name
	if (toLoad.empty())
		return;

	// save the current world's data
	if (!currentXMLFile.empty())
		save();

	LightSystem::clear();

	// load file data to string
	auto xmlPath = game::config::xmlFolder + toLoad;
	auto xmlRaw = readFile(xmlPath);

	// let tinyxml parse the file
	UserAssert(xmlDoc.Parse(xmlRaw.data()) == XML_NO_ERROR,
		"XML Error: Failed to parse file (not your fault though..?)");

	// include headers
	std::vector<std::string> toAdd;
	auto ixml = xmlDoc.FirstChildElement("include");
	while (ixml != nullptr) {
		auto file = ixml->Attribute("file");
		UserAssert(file != nullptr, "XML Error: <include> tag file not given");

		if (file != nullptr) {
			DEBUG_printf("Including file: %s\n", file);
			toAdd.emplace_back(game::config::xmlFolder + file);
			//xmlRaw.append(readFile(xmlFolder + file));
		} else {
			UserError("XML Error: <include> tag file not given");
		}

		ixml = ixml->NextSiblingElement("include");
	}

	for (const auto& f : toAdd)
		xmlRaw.append(readFile(f)); 

	UserAssert(xmlDoc.Parse(xmlRaw.data()) == XML_NO_ERROR,
		"XML Error: failed to append includes");

	// look for an opening world tag
	auto wxml = xmlDoc.FirstChildElement("World");
	if (wxml != nullptr) {
		wxml = wxml->FirstChildElement();
	} else {
		UserAssert(0, "XML Error: Cannot find a <World> or <IndoorWorld> tag in " + xmlPath);
	}

	world.toLeft = world.toRight = world.outdoor = "";
	currentXMLFile = toLoad;

	//game::entities.reset();
	if (!savedEntities.empty()) {
		savedEntities.emplace_back(PlayerSystem::getId());
		game::entities.each<Position>(
			[&](entityx::Entity entity, Position& p) {
			(void)p;
			if (std::find(savedEntities.begin(), savedEntities.end(), entity.id()) == savedEntities.end())
				entity.destroy();
		}, true);
	} else {
		game::entities.reset();
		PlayerSystem::create();
	}

	// iterate through tags
	while (wxml != nullptr) {
		std::string tagName = wxml->Name();

		// style tag
		if (tagName == "style") {
			world.styleFolder = wxml->StrAttribute("folder");
			world.bgm = wxml->StrAttribute("bgm");

			std::vector<std::string> bgFiles;
			auto layerTag = wxml->FirstChildElement("layer");
			while (layerTag != nullptr) {
				bgFiles.emplace_back(world.styleFolder + layerTag->StrAttribute("path"));
				layerTag = layerTag->NextSiblingElement("layer");
			}

			bgTex = TextureIterator(bgFiles);
		}

		// world generation
		else if (tagName == "ground") {
			generate(wxml->Attribute("path"));
		}

		// weather tag
		else if (tagName == "weather") {
			WeatherSystem::setWeather(wxml->GetText());
		}

		// link tags
		else if (tagName == "link") {
			if (auto linkTo = wxml->Attribute("left"); linkTo != nullptr)
				world.toLeft = linkTo;
			else if (auto linkTo = wxml->Attribute("right"); linkTo != nullptr)
				world.toRight = linkTo;
			else if (auto linkTo = wxml->Attribute("outside"); linkTo != nullptr)
				world.outdoor = linkTo;
			else
				UserError("<link> tag with bad attribute");
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
						entity.assign<Position>(wxml, abcd);
						auto pos = entity.component<Position>();
						if (pos->x == 0 && pos->y == 0) {
							pos->x = (randGet() % static_cast<int>(world.startX * 1.9f)) + world.startX;
							pos->y = 150;
						}
					} else if (tname == "Visible")
						entity.assign<Visible>(wxml, abcd);
					else if (tname == "Sprite")
						entity.assign<Sprite>(wxml, abcd);
					else if (tname == "Portal")
						entity.assign<Portal>(wxml, abcd);
					else if (tname == "Solid") {
						auto solid = entity.assign<Solid>(wxml, abcd);
						if (solid->width == 0 && solid->height == 0) {
							auto dim = entity.component<Sprite>()->getSpriteSize();
							entity.remove<Solid>();
							entity.assign<Solid>(dim.x, dim.y);
						}
					} else if (tname == "Direction")
						entity.assign<Direction>(wxml, abcd);
					else if (tname == "Physics")
						entity.assign<Physics>(wxml, abcd);
					else if (tname == "Name")
						entity.assign<Name>(wxml, abcd);
					else if (tname == "Dialog")
						entity.assign<Dialog>(wxml, abcd);
					else if (tname == "Grounded")
						entity.assign<Grounded>(); // no need to pass xmls...
					else if (tname == "Wander") {
						auto script = abcd->GetText();
						entity.assign<Wander>(script != nullptr ? script : "");
					} else if (tname == "Hop")
						entity.assign<Hop>();
					else if (tname == "Health")
						entity.assign<Health>(wxml, abcd);
					else if (tname == "Aggro")
						entity.assign<Aggro>(wxml, abcd);
					else if (tname == "Animation")
						entity.assign<Animate>(wxml, abcd);
					else if (tname == "Trigger")
						entity.assign<Trigger>(wxml, abcd);
					else if (tname == "Drop")
						entity.assign<Drop>(wxml, abcd);
					else if (tname == "Illuminate")
						entity.assign<Illuminate>(wxml, abcd);

					abcd = abcd->NextSiblingElement();
				}

			} else {
				UserError("Unknown tag <" + tagName + "> in file " + currentXMLFile);
			}
		}

		wxml = wxml->NextSiblingElement();
	}

	// attempt to load data
	std::ifstream save (game::config::xmlFolder + currentXMLFile + ".dat");
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

			while (!save.eof() && id != 'p') {
				switch (id) {
				case 'd':
					save >> entity.component<Dialog>()->index;
					break;
				case 'h':
					save >> entity.component<Health>()->health;
					break;
				}

				save >> id;
			}
		}

		save.close();
	}

	toLoad.clear();
	game::events.emit<BGMToggleEvent>();
}

void WorldSystem::die(void)
{
    // SDL2_mixer's object
	if (bgmObj != nullptr)
		Mix_FreeMusic(bgmObj);
}

void WorldSystem::render(void)
{
	float z = Render::ZRange::World;

	static Color ambient;

	const auto SCREEN_WIDTH = game::SCREEN_WIDTH;
	const auto SCREEN_HEIGHT = game::SCREEN_HEIGHT;

	const vector2<int> backgroundOffset
		(static_cast<int>(SCREEN_WIDTH) / 2, static_cast<int>(SCREEN_HEIGHT) / 2);

    // world width in pixels
	int width = world.ground.getDim().x;

	static std::once_flag init;
	std::call_once(init, [&](void) {
		thAmbient = std::thread([&](void) {
			while (game::engine.shouldRun) {
				float v = 75 * sin((game::time::getTickCount() + (DAY_CYCLE / 2)) / (DAY_CYCLE / PI));
				float rg = std::clamp(.5f + (-v / 100.0f), 0.01f, .9f);
				float b  = std::clamp(.5f + (-v / 80.0f), 0.03f, .9f);

				ambient = Color(rg, rg, b, 1.0f);
				std::this_thread::sleep_for(1ms);
			}
		});
		thAmbient.detach();
	});

	auto push5 = [](GLfloat *&vec, GLfloat *five) {
		for (int i = 0; i < 5; i++)
			*vec++ = *five++;
	};

	// shade value for GLSL
	float shadeAmbient = std::max(0.0f, static_cast<float>(-worldShade) / 50 + 0.5f); // 0 to 1.5f

	if (shadeAmbient > 0.9f)
		shadeAmbient = 1;

	// TODO scroll backdrop
	GLfloat skyOffset = -0.5f * cos(PI / DAY_CYCLE * game::time::getTickCount()) + 0.5f;
	if (skyOffset < 0)
		skyOffset++;

	GLfloat skyverts[] = {
		offset.x - backgroundOffset.x - 5, offset.y - backgroundOffset.y, z, 0, skyOffset,
		offset.x + backgroundOffset.x + 5, offset.y - backgroundOffset.y, z, 1, skyOffset,
		offset.x + backgroundOffset.x + 5, offset.y + backgroundOffset.y, z, 1, skyOffset,
		offset.x + backgroundOffset.x + 5, offset.y + backgroundOffset.y, z, 1, skyOffset,
		offset.x - backgroundOffset.x - 5, offset.y + backgroundOffset.y, z, 0, skyOffset,
		offset.x - backgroundOffset.x - 5, offset.y - backgroundOffset.y, z, 0, skyOffset,
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

	glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), skyverts);
	glVertexAttribPointer(Render::worldShader.tex  , 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), skyverts + 3);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	if (worldShade > 0) {
		static const Texture starTex ("assets/style/classic/bg/star.png"); // TODO why in theme, not just std.?
		static const float stardim = 24;

		GLfloat* star_coord = new GLfloat[stars.size() * 5 * 6 + 1];
		auto si = star_coord;
		auto xcoord = offset.x * 0.9f;

		for (auto &s : stars) {
			float data[30] = {
				s.x + xcoord, s.y, z - 0.1f, 0, 0,
				s.x + xcoord + stardim, s.y, z - 0.1f, 1, 0,
				s.x + xcoord + stardim, s.y + stardim, z - 0.1f, 1, 1,
				s.x + xcoord + stardim, s.y + stardim, z - 0.1f, 1, 1,
				s.x + xcoord, s.y + stardim, z - 0.1f, 0, 1,
				s.x + xcoord, s.y, z - 0.1f, 0, 0
			};

			std::memcpy(si, data, sizeof(float) * 30);
			si += 30;
		}
		starTex.use();
		glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.3);

		glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &star_coord[0]);
		glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &star_coord[3]);
		glDrawArrays(GL_TRIANGLES, 0, stars.size() * 6);

		delete[] star_coord;
	}

	glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.0);
	glUniform4f(Render::worldShader.uniform[WU_ambient], ambient.red, ambient.green, ambient.blue, 1.0);

	int layerCount = bgTex.size() - 3;
	float parallax = 0.85f;
	float parallaxChange = 0.75f / layerCount;

	z -= 0.2f;
	for (int i = 0; i < layerCount; i++, z -= 0.1f, parallax -= parallaxChange) {
		bgTex++;
		auto mountainDim = bgTex.getTextureDim() * game::HLINE;
		auto xcoord = width / 2 * -1 + offset.x * parallax;

		int count = width / mountainDim.x + 1;
		GLfloat* bgItems = new GLfloat[count * 30];
		GLfloat* bgItemsFront = bgItems;

		for (int j = 0; j < count; j++) {
			GLfloat five[5] = {
				0, 0, mountainDim.x * j + xcoord, 0, z
			};

			push5(bgItemsFront, five);
			five[0]++, five[2] += mountainDim.x;
			push5(bgItemsFront, five);
			five[1]++, five[3] += mountainDim.y;
			push5(bgItemsFront, five);
			push5(bgItemsFront, five);
			five[0]--, five[2] -= mountainDim.x;
			push5(bgItemsFront, five);
			five[1]--, five[3] -= mountainDim.y;
			push5(bgItemsFront, five);
		}

		glUniform1f(Render::worldShader.uniform[WU_light_impact], 0.01);

		glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), bgItems + 2);
		glVertexAttribPointer(Render::worldShader.tex  , 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), bgItems);
		glDrawArrays(GL_TRIANGLES, 0, count * 6);

		delete[] bgItems;
	}

	world.ground.use();
	auto dim = world.ground.getDim();
	GLfloat verts[] = {
		world.startX,         0,         z, 0, 0,
		world.startX + dim.x, 0,         z, 1, 0,
		world.startX + dim.x, dim.y, z, 1, 1,
		world.startX + dim.x, dim.y, z, 1, 1,
		world.startX,         dim.y, z, 0, 1,
		world.startX,         0,         z, 0, 0,
	};

	glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), verts);
	glVertexAttribPointer(Render::worldShader.tex  , 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), verts + 3);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	Render::worldShader.disable();
	Render::worldShader.unuse();
}

bool WorldSystem::receive(const BGMToggleEvent &bte)
{
	if (bte.world == nullptr || world.bgm != bte.file) {
		if (bgmCurrent == world.bgm)
			return true;

		Mix_FadeOutMusic(800);

		if (bgmObj != nullptr)
			Mix_FreeMusic(bgmObj);

		bgmCurrent = world.bgm;
		bgmObj = Mix_LoadMUS(world.bgm.c_str());
		Mix_PlayMusic(bgmObj, -1);
	}
	return true;
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
	game::entities.each<Health>(
		[](entityx::Entity e, Health& h) {
		if (h.health <= 0) {
			if (e.has_component<Drop>()) {
				for (auto& d : e.component<Drop>()->items) {
					auto& pos = *e.component<Position>();
					InventorySystem::makeDrop(vec2(pos.x, pos.y), d.first, d.second);
				}
			}

			e.kill();
		}
	});

	game::entities.each<Grounded, Position, Solid>(
		[&](entityx::Entity e, Grounded &g, Position &loc, Solid &dim) {
		(void)e;
		if (!g.grounded) {
			// make sure entity is above ground
			auto height = getGroundHeight(loc.x + dim.width / 2);
			if (loc.y != height) {
				loc.y = height;
				e.remove<Grounded>();
			}
		}
	});

	game::entities.each<Direction, Physics>(
		[&](entityx::Entity e, Direction &vel, Physics &phys) {
		(void)e;
		// handle gravity
		if (vel.y > -2.0f)
			vel.y -= GRAVITY_CONSTANT * phys.g * dt;
	});

	game::entities.each<Position, Direction, Solid>(
	    [&](entityx::Entity e, Position &loc, Direction &vel, Solid &dim) {
		(void)e;

		// make sure entity is above ground
		auto height = getGroundHeight(loc.x + dim.width / 2);
		if (loc.y < height) {
			/*int dir = vel.x < 0 ? -1 : 1;
			auto near = std::clamp(line + dir * 2, 0, static_cast<int>(world.data.size()));
			if (world.data[near].groundHeight - 30 > world.data[line + dir].groundHeight) {
				loc.x -= (PLAYER_SPEED_CONSTANT + 2.7f) * dir * 2;
				vel.x = 0;
			} else {*/
				loc.y = height - 0.001f * dt;
				vel.y = 0;
				if (!vel.grounded) {
					vel.grounded = true;
					ParticleSystem::addMultiple(20, ParticleType::SmallPoof,
						[&](){ return vec2(loc.x + randGet() % static_cast<int>(dim.width), loc.y); }, 500, 30);
				}
			//}
		}


		// insure that the entity doesn't fall off either edge of the world.
		if (loc.x < world.startX) {
			vel.x = 0;
			loc.x = world.startX + HLINES(0.5f);
		} else if (loc.x + dim.width + game::HLINE > -static_cast<int>(world.startX)) {
			vel.x = 0;
			loc.x = -static_cast<int>(world.startX) - dim.width - game::HLINE;
		}
	});
}

void WorldSystem::goWorldRight(Position& p, Solid &d)
{
	if (!(world.toRight.empty()) && (p.x + d.width > world.startX * -1 - HLINES(5))) {
		UISystem::fadeToggle();
		UISystem::waitForCover();
		while (waitToSwap)
			std::this_thread::sleep_for(1ms);
		load(world.toRight);
		PlayerSystem::setX(world.startX + HLINES(10));
		UISystem::fadeToggle();
	}
}

void WorldSystem::goWorldLeft(Position& p)
{
	if (!(world.toLeft.empty()) && (p.x < world.startX + HLINES(10))) {
		UISystem::fadeToggle();
		UISystem::waitForCover();
		while (waitToSwap)
			std::this_thread::sleep_for(1ms);
		load(world.toLeft);
		PlayerSystem::setX(world.startX * -1 - HLINES(15));
		UISystem::fadeToggle();
	}
}

void WorldSystem::goWorldPortal(Position& p)
{
	std::string file;

	if (!world.outdoor.empty()) {
		file = world.outdoor;
		world.outdoor = "";
	} else {
		game::entities.each<Position, Solid, Portal>(
			[&](entityx::Entity entity, Position& loc, Solid &dim, Portal &portal) {
			(void)entity;
			if (!(portal.toFile.empty()) && p.x > loc.x && p.x < loc.x + dim.width)  {
				file = portal.toFile;
				return;
			}
		});
	}

	if (!file.empty()) {
		UISystem::fadeToggle();
		UISystem::waitForCover();
		while (waitToSwap)
			std::this_thread::sleep_for(1ms);
		load(file);
		UISystem::fadeToggle();
	}
}
