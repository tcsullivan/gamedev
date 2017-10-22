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

WorldData2        WorldSystem::world;
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

int WorldSystem::getLineIndex(float x)
{
	return std::clamp(static_cast<int>((x - world.startX) / game::HLINE),
		0, static_cast<int>(world.data.size()));
}

void WorldSystem::generate(LuaScript& script)
{
	int i = 0;
	world.data.clear();
	do {
		float h, g[2];
		script("ground", {LuaVariable("height", h)});
		if (h == -1.0f)
			break;
		script("grass", {LuaVariable("height", g[0])});
		script("grass", {LuaVariable("height", g[1])});
		world.data.push_back(WorldData {true, {g[0], g[1]}, h,
			static_cast<unsigned char>(randGet() % 32 / 8)});
	} while (++i);

	// define x-coordinate of world's leftmost 'line'
	world.startX = HLINES(i * -0.5);

	// gen. star coordinates
	if (stars.empty()) {
		stars.resize(game::SCREEN_WIDTH / 30);
		for (auto& s : stars) {
			s.x = world.startX + (randGet() % (int)HLINES(i));
			s.y = game::SCREEN_HEIGHT - (randGet() % (int)HLINES(game::SCREEN_HEIGHT / 1.3f));
		}
	}
}

float WorldSystem::isAboveGround(const vec2& p) 
{
	const auto& gh = world.data[getLineIndex(p.x)].groundHeight;
	return p.y >= gh ? 0 : gh;
}

bool WorldSystem::save(void)
{	
	if (world.indoor)
		return false;

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
		world.indoor = false;
	} else {
		wxml = xmlDoc.FirstChildElement("IndoorWorld");
		UserAssert(wxml != nullptr, "XML Error: Cannot find a <World> or <IndoorWorld> tag in " + xmlPath);
		wxml = wxml->FirstChildElement();
		world.indoor = true;
		if (world.outdoor.empty()) {
			world.outdoor = currentXMLFile;
			world.outdoorCoords = vec2(0, 100);
		}
	}

	world.toLeft = world.toRight = "";
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
		else if (tagName == "generation") {
			LuaScript script (wxml->GetText());
			generate(script);
		}

		// indoor stuff
		else if (tagName == "house") {
			if (!world.indoor)
				UserError("<house> can only be used inside <IndoorWorld>");

			//world.indoorWidth = wxml->FloatAttribute("width");
			world.indoorTex = Texture(wxml->StrAttribute("texture")); // TODO winbloze lol
			auto str = wxml->StrAttribute("texture");
			auto tex = Texture(str);
			world.indoorTex = tex;
		}

		// weather tag
		else if (tagName == "weather") {
			WeatherSystem::setWeather(wxml->GetText());
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

	int iStart, iEnd, pOffset;

    // world width in pixels
	int width = HLINES(world.data.size());

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

	if (world.indoor) {
		world.indoorTex.use();
		auto dim = world.indoorTex.getDim() * game::HLINE;
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
	}

	//glUniform1f(Render::worldShader.uniform[WU_light_impact], 0.075f + (0.2f * i));
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// get the line that the player is currently standing on
	pOffset = (offset.x - world.startX) / game::HLINE;

	// only draw world within player vision
	iStart = std::clamp(static_cast<int>(pOffset - (SCREEN_WIDTH / 2 / game::HLINE)),
	                    0, static_cast<int>(world.data.size()));
	iEnd = std::clamp(static_cast<int>(pOffset + (SCREEN_WIDTH / 2 / game::HLINE) + 2),
                      0, static_cast<int>(world.data.size()));

	// draw the dirt
	waitToSwap = true;

	bgTex++;
	z = Render::ZRange::Ground;

	static std::vector<GLfloat> dirt;
	if (dirt.size() != world.data.size() * 30) {
		dirt.clear();
		dirt.resize(world.data.size() * 30);
	}

	GLfloat *dirtp = &dirt[0];
	for (int i = iStart; i < iEnd; i++) {
		int ty = world.data[i].groundHeight / 64 + world.data[i].groundColor;
		GLfloat five[5] = {
			0, 0, world.startX + HLINES(i), world.data[i].groundHeight, z - 0.1f
		};

		push5(dirtp, five);
		five[0]++, five[2] += game::HLINE;
		push5(dirtp, five);
		five[1] += ty, five[3] = 0;
		push5(dirtp, five);
		push5(dirtp, five);
		five[0]--, five[2] -= game::HLINE;
		push5(dirtp, five);
		five[1] = 0, five[3] = world.data[i].groundHeight;
		push5(dirtp, five);
	}

	glUniform1f(Render::worldShader.uniform[WU_light_impact], 0.45f);

	glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &dirt[2]);
	glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &dirt[0]);
	glDrawArrays(GL_TRIANGLES, 0 , dirt.size() / 5);

	Render::worldShader.disable();
	Render::worldShader.unuse();

	bgTex++;

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
		if (!world.indoor && !wd.grassUnpressed) {
			gh[0] /= 4;
			gh[1] /= 4;
		}

		// actually draw the grass.
		if (wd.groundHeight) {
			float five[5] = {
				0, 1, world.startX + HLINES(i), wd.groundHeight + gh[0], z - 0.2f
			};

			push5(grassp, five);
			five[0]++, five[1]--, five[2] += HLINES(0.5f);
			push5(grassp, five);
			five[1]++, five[3] = wd.groundHeight;
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
			five[1]++, five[3] = wd.groundHeight;
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
			
	auto yOffset = offset.y - static_cast<float>(SCREEN_HEIGHT) / 2.0f;
	GLfloat blackBar[] = {
		s,            yOffset,           z - 0.3f, 0.0f, 0.0f,
		world.startX, yOffset,           z - 0.3f, 1.0f, 0.0f,
		world.startX, yOffset + sheight, z - 0.3f, 1.0f, 1.0f,
		world.startX, yOffset + sheight, z - 0.3f, 1.0f, 1.0f,
   		s,            yOffset + sheight, z - 0.3f, 0.0f, 1.0f,
		s,            yOffset,           z - 0.3f, 0.0f, 0.0f
	};

	if (offset.x + world.startX > s) {
		Colors::black.use();
		glUniform1f(Render::worldShader.uniform[WU_light_impact], 0.0f);
		glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, blackBar);
		glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, blackBar + 3);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	if (offset.x - world.startX < e) {
		blackBar[0] = blackBar[20] = blackBar[25] = -world.startX;
		blackBar[5] = blackBar[10] = blackBar[15] = e;

		Colors::black.use();
		glUniform1f(Render::worldShader.uniform[WU_light_impact], 0.0f);
		glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, blackBar);
		glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, blackBar + 3);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	Render::worldShader.disable();
	Render::worldShader.unuse();

	waitToSwap = false;
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
			auto height = world.data[getLineIndex(loc.x + dim.width / 2)].groundHeight;
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
		// get the line the entity is on
		auto line = getLineIndex(loc.x + dim.width / 2);

		// make sure entity is above ground
		if (loc.y < world.data[line].groundHeight) {
			int dir = vel.x < 0 ? -1 : 1;
			auto near = std::clamp(line + dir * 2, 0, static_cast<int>(world.data.size()));
			if (world.data[near].groundHeight - 30 > world.data[line + dir].groundHeight) {
				loc.x -= (PLAYER_SPEED_CONSTANT + 2.7f) * dir * 2;
				vel.x = 0;
			} else {
				loc.y = world.data[line].groundHeight - 0.001f * dt;
				vel.y = 0;
				if (!vel.grounded) {
					vel.grounded = true;
					ParticleSystem::addMultiple(20, ParticleType::SmallPoof,
						[&](){ return vec2(loc.x + randGet() % static_cast<int>(dim.width), loc.y); }, 500, 30);
				}
			}
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
