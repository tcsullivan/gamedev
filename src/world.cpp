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

// external variables
extern Player      *player;					// main.cpp?
extern World       *currentWorld;			// main.cpp
extern World       *currentWorldToLeft;		// main.cpp
extern World       *currentWorldToRight;	// main.cpp
extern bool         inBattle;               // ui.cpp?
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

// keeps track of information of worlds the player has left to enter arenas
static std::vector<WorldSwitchInfo> arenaNest;

// pathnames of images for world themes
static const std::string bgPaths[] = {
    "bg.png",					// Daytime background
    "bgn.png",					// Nighttime background
    "bgFarMountain.png",		// Furthest layer
    "forestTileFar.png",		// Furthest away Tree Layer
    "forestTileBack.png",		// Closer layer
    "forestTileMid.png",		// Near layer
    "forestTileFront.png",		// Closest layer
    "dirt.png",					// Dirt
    "grass.png",				// Grass
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

/**
 * Creates a world object.
 * Note that all this does is nullify a pointer...
 */
World::World(bool indoor)
	: m_Indoor(indoor), lineCount(0), worldStart(0) 
{
}

/**
 * The world destructor.
 * This will free objects used by the world itself, then free the vectors of
 * entity-related objects.
 */
World::
~World(void)
{
	deleteEntities();
}

/**
 * The entity vector destroyer.
 * This function will free all memory used by all entities, and then empty the
 * vectors they were stored in.
 */
template<class T>
void clearPointerVector(T &vec)
{
    while (!vec.empty()) {
        delete vec.back();
        vec.pop_back();
     }
}

void World::
deleteEntities(void)
{
    // free particles
	particles.clear();
    // clear light array
	light.clear();
    // free villages
	village.clear();
    // clear entity array
	clearPointerVector(entity);
}

/**
 * Generates a world of the specified width.
 * This will mainly populate the WorldData array, preparing most of the world
 * object for usage.
 */
void World::
generate(int width)
{
    float geninc = 0;

    // check for valid width
    if (width <= 0)
        UserError("Invalid world dimensions");

    // allocate space for world
    worldData = std::vector<WorldData> (width + GROUND_HILLINESS, WorldData { false, {0, 0}, 0, 0 });
    lineCount = worldData.size();

    // prepare for generation
    worldData.front().groundHeight = GROUND_HEIGHT_INITIAL;
    auto wditer = std::begin(worldData) + GROUND_HILLINESS;

	if (m_Indoor) {
		for(wditer = std::begin(worldData); wditer < std::end(worldData); wditer++) {
			auto w = &*(wditer);
			w->groundHeight = GROUND_HEIGHT_MINIMUM + 5;
			w->groundColor = 4;
		}
	} else {
	    // give every GROUND_HILLINESSth entry a groundHeight value
	    for (; wditer < std::end(worldData); wditer += GROUND_HILLINESS)
	        wditer[-static_cast<int>(GROUND_HILLINESS)].groundHeight = wditer[0].groundHeight + (randGet() % 8 - 4);

    	// create slopes from the points that were just defined, populate the rest of the WorldData structure
	    for (wditer = std::begin(worldData) + 1; wditer < std::end(worldData); wditer++){
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
    worldStart = (width - GROUND_HILLINESS) * game::HLINE / 2 * -1;

    // create empty star array, should be filled here as well...
	star = std::vector<vec2> (100, vec2 { 0, 400 });
	for (auto &s : star) {
		s.x = (randGet() % (static_cast<int>(-worldStart) * 2)) + worldStart;
		s.y = (randGet() % game::SCREEN_HEIGHT) + 100;
	}
}

static Color ambient;

void World::draw(Player *p)
{
	auto HLINE = game::HLINE;

	uint ls = light.size();

	GLfloat *lightCoords = new GLfloat[ls * 4];
	GLfloat *lightColors = new GLfloat[ls * 4];

	uint lpIndex = 0;
	uint lcIndex = 0;

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

	for (uint i = 0; i < ls; i++) {
       	auto &l = light[i];
 	   	if (l.belongsTo) {
            l.loc.x = l.following->loc.x;
            l.loc.y = l.following->loc.y;
        }
        if (l.flame) {
            l.fireFlicker = 0.9f + ((rand()% 2 ) / 10.0f);
            l.fireLoc.x = l.loc.x + (rand() % 2 - 1) * 3;
            l.fireLoc.y = l.loc.y + (rand() % 2 - 1) * 3;
        } else {
            l.fireFlicker = 1;
        }

		lightCoords[lpIndex++] = l.loc.x;
		lightCoords[lpIndex++] = l.loc.y;
		lightCoords[lpIndex++] = 0.0;
		lightCoords[lpIndex++] = l.radius;

		lightColors[lcIndex++] = l.color.red;
		lightColors[lcIndex++] = l.color.green;
		lightColors[lcIndex++] = l.color.blue;
		lightColors[lcIndex++] = 1.0;
	}

	Render::worldShader.use();

	glUniform4fv(Render::worldShader.uniform[WU_light], ls, lightCoords);
	glUniform4fv(Render::worldShader.uniform[WU_light_color], ls, lightColors);
	glUniform1i(Render::worldShader.uniform[WU_light_size], ls);

	Render::worldShader.unuse();

	for (auto &e :entity)
        e->draw();

	// flatten grass under the player if the player is on the ground
	if (p->ground) {
		int pOffset = (p->loc.x + p->width / 2 - worldStart) / HLINE;

		for (unsigned int i = 0; i < worldData.size(); i++)
			worldData[i].grassUnpressed = !(i < static_cast<unsigned int>(pOffset + 6) && i > static_cast<unsigned int>(pOffset - 6));
	} else {
		for (auto &wd : worldData)
			wd.grassUnpressed = true;
	}

    // draw the player
	//p->draw();

	// draw particles like a MASTAH
    glBindTexture(GL_TEXTURE_2D, colorIndex);
    glUniform1i(Render::worldShader.uniform[WU_texture], 0);
    Render::worldShader.use();

	glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, .8);

    Render::worldShader.enable();

	partMutex.lock();
	uint ps = particles.size();
    uint pss = ps * 6 * 5;
	uint pc = 0;

	std::vector<GLfloat> partVec(pss);
	auto *pIndex = &partVec[0];
	for (uint i = 0; i < ps; i++) {
        pc += 30;
		if (pc > pss) {
			// TODO resize the vector or something better than breaking
			break;
		}
		particles[i].draw(pIndex);
    }
	partMutex.unlock();

    glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &partVec[0]);
    glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &partVec[3]);
    glDrawArrays(GL_TRIANGLES, 0, ps * 6);

    Render::worldShader.disable();

	glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.0);

    Render::worldShader.unuse();
}

/**
 * Get's the world's width in pixels.
 */
int World::
getTheWidth(void) const
{
	return (worldStart * -2);
}

float World::
getWorldStart(void) const
{
    return static_cast<float>(worldStart);
}

/**
 * Get a pointer to the most recently created light.
 * Meant to be non-constant.
 */
Light& World::
getLastLight(void)
{
    return light.back();
}

/**
 * Get a pointer to the most recently created mob.
 * Meant to be non-constant.
 */
Mob* World::
getLastMob(void)
{
	for (auto e = entity.rbegin(); e != entity.rend(); ++e) {
		if ((*e)->type == MOBT)
			return dynamic_cast<Mob *>(*e);
	}

    return nullptr;
}

/**
 * Get the interactable entity that is closest to the entity provided.
 */
Entity* World::
getNearInteractable(Entity &e)
{
    auto n = std::find_if(std::begin(entity), std::end(entity), [&](Entity *&a) {
        return ((a->type == MOBT) || (a->type == NPCT) || a->type == MERCHT) &&
               e.isNear(a) && (e.left ? (a->loc.x < e.loc.x) : (a->loc.x > e.loc.x));
    });

    return n == std::end(entity) ? nullptr : *n;
}

Mob* World::
getNearMob(Entity &e)
{
    auto n = std::find_if(std::begin(entity), std::end(entity), [&](Entity *a) {
        return (a->type == MOBT && e.isNear(a) && (e.left ? (a->loc.x < e.loc.x + e.width / 2) : (a->loc.x + a->width > e.loc.x + e.width / 2)));
    });

    return (n == std::end(entity)) ? nullptr : dynamic_cast<Mob *>(*n);
}


/**
 * Get the file path for the `index`th building.
 */
std::string World::
getSTextureLocation(unsigned int index) const
{
    return index < sTexLoc.size() ? sTexLoc[ index ] : "";
}

/**
 * Get the coordinates of the `index`th building, with -1 meaning the last building.
 */
vec2 World::
getStructurePos(int index)
{
    if (index < 0) {
		for (auto e = entity.rbegin(); e != entity.rend(); ++e) {
			if ((*e)->type == STRUCTURET)
				return (*e)->loc;
		}

		return vec2 {0, 0};
	}

	int nth = 0;
	for (const auto &e : entity) {
		if (e->type == STRUCTURET) {
			if (index == nth)
				return e->loc;
			else
				++nth;
		}
	}

    return vec2 {0, 0};
}

/**
 * Saves world data to a file.
 */
void World::save(const std::string& s)
{
	for (const auto &e : entity)
		e->saveToXML();
	currentXMLDoc.SaveFile((s.empty() ? currentXML : xmlFolder + s).c_str(), false);
}

/**
 * Sets the desired theme for the world's background.
 * The images chosen for the background layers are selected depending on the
 * world's background type.
 */
void World::setBackground(WorldBGType bgt)
{
	bgType = bgt;
}

/**
 * Sets the world's style.
 * The world's style will determine what sprites are used for things like\
 * generic structures.
 */
void World::setStyle(std::string pre)
{
    // get folder prefix
	std::string prefix = pre.empty() ? "assets/style/classic/" : pre;
	styleFolder = prefix + "bg/";

    for (const auto &s : buildPaths)
        sTexLoc.push_back(prefix + s);
}

/**
 * Pretty self-explanatory.
 */
std::string World::setToLeft(std::string file)
{
    return (toLeft = file);
}

/**
 * Pretty self-explanatory.
 */
std::string World::setToRight(std::string file)
{
	return (toRight = file);
}

/**
 * Pretty self-explanatory.
 */
std::string World::getToLeft(void) const
{
    return toLeft;
}

/**
 * Pretty self-explanatory.
 */
std::string World::getToRight(void) const
{
    return toRight;
}

/**
 * Attempts to go to the left world, returning either that world or itself.
 */
WorldSwitchInfo World::goWorldLeft(Player *p)
{
	World *tmp;
    // check if player is at world edge
	if (!toLeft.empty() && p->loc.x < worldStart + HLINES(15)) {
        // load world (`toLeft` conditional confirms existance)
	    tmp = loadWorldFromPtr(currentWorldToLeft);

        // return pointer and new player coords
        return std::make_pair(tmp, vec2 {tmp->worldStart + tmp->getTheWidth() - (float)HLINES(15),
                              tmp->worldData[tmp->lineCount - 1].groundHeight});
	}

	return std::make_pair(this, vec2 {0, 0});
}

/**
 * Attempts to go to the right world, returning either that world or itself.
 */
WorldSwitchInfo World::goWorldRight(Player *p)
{
	World *tmp;
	if (!toRight.empty() && p->loc.x + p->width > -worldStart - HLINES(15)) {
        tmp = loadWorldFromPtr(currentWorldToRight);
        return std::make_pair(tmp, vec2 {tmp->worldStart + (float)HLINES(15.0), GROUND_HEIGHT_MINIMUM} );
	}

	return std::make_pair(this, vec2 {0, 0});
}

void World::adoptNPC(NPC *e)
{
	entity.push_back(e);
}

void World::adoptMob(Mob* e)
{
	entity.push_back(e);
}

/**
 * Acts like goWorldLeft(), but takes an NPC; returning true on success.
 */
bool World::goWorldLeft(NPC *e)
{
	// check if entity is at world edge
	if (!toLeft.empty() && e->loc.x < worldStart + HLINES(15)) {
		currentWorldToLeft->adoptNPC(e);

		entity.erase(std::find(std::begin(entity), std::end(entity), e));

        e->loc.x = currentWorldToLeft->worldStart + currentWorldToLeft->getTheWidth() - HLINES(15);
		e->loc.y = GROUND_HEIGHT_MAXIMUM;
		++e->outnabout;

		return true;
	}

	return false;
}

bool World::goWorldRight(NPC *e)
{
	if (!toRight.empty() && e->loc.x + e->width > -worldStart - HLINES(15)) {
		currentWorldToRight->adoptNPC(e);

		entity.erase(std::find(std::begin(entity), std::end(entity), e));

		e->loc.x = currentWorldToRight->worldStart + HLINES(15);
		e->loc.y = GROUND_HEIGHT_MINIMUM;
		--e->outnabout;

		return true;
	}

	return false;
}

/**
 * Attempts to enter a building that the player is standing in front of.
 */
WorldSwitchInfo World::goInsideStructure(Player *p)
{
	World *tmp;
	static std::string outdoorData, outdoorName;

	// enter a building
	if (outdoorName.empty()) {
        auto d = std::find_if(std::begin(entity), std::end(entity), [p](const Entity *s) {
            return ((p->loc.x > s->loc.x) && (p->loc.x + p->width < s->loc.x + s->width));
        });

        if ((d == std::end(entity)) || dynamic_cast<Structures *>(*d)->inside.empty())
            return std::make_pair(this, vec2 {0, 0});

		outdoorData = currentXMLRaw;
		outdoorName = currentXML;
		currentXML = xmlFolder + dynamic_cast<Structures *>(*d)->inside;
		const char *buf = readFile(currentXML.c_str());
		currentXMLRaw = buf;
		delete[] buf;

		tmp = dynamic_cast<Structures *>(*d)->insideWorld;

		return std::make_pair(tmp, vec2 {0, 100});
	}

	// exit the building
	else {
        std::string current = &currentXML[xmlFolder.size()];
		currentXML = outdoorName;
		currentXMLRaw = outdoorData;
		outdoorName.clear();
		outdoorData.clear();

		/*tmp = dynamic_cast<IndoorWorld *>(currentWorld)->outside; //loadWorldFromXML(inside.back());

        Structures *b = nullptr;
        for (auto &s : tmp->entity) {
            if (s->type == STRUCTURET && dynamic_cast<Structures *>(s)->inside == current) {
                b = dynamic_cast<Structures *>(s);
                break;
            }
        }

        if (b == nullptr)*/
            return std::make_pair(this, vec2 {0, 0});

		//return std::make_pair(tmp, vec2 {b->loc.x + (b->width / 2), 0});
	}

	return std::make_pair(this, vec2 {0, 0});
}

void World::
addStructure(Structures *s)
{
	entityPending.push_back(s);
}

Village *World::
addVillage(std::string name, World *world)
{
    village.emplace_back(name, world);
    return &village.back();
}

void World::addMob(Mob *m, vec2 coord)
{
	m->spawn(coord.x, coord.y);

	entityPending.push_back(m);
}

void World::
addNPC(NPC *n)
{
	entityPending.push_back(n);
}

void World::
addMerchant(float x, float y, bool housed)
{
	Merchant *tmp = new Merchant();

	tmp->spawn(x, y);

    if (housed) {
        tmp->inside = dynamic_cast<Structures *>(*std::find_if(entity.rbegin(), entity.rend(), [&](Entity *e){ return (e->type == STRUCTURET); }));
    	tmp->z = tmp->inside->z + 0.1f;
	}

	entityPending.push_back(tmp);
}

void World::
addObject(std::string in, std::string p, float x, float y)
{
	Object *tmp = new Object(in, p);
	tmp->spawn(x, y);

	entityPending.push_back(tmp);
}

void World::
addParticle(float x, float y, float w, float h, float vx, float vy, Color color, int d)
{
	particles.push_back(Particles(x, y, w, h, vx, vy, color, d));
	particles.back().canMove = true;
}

void World::
addParticle(float x, float y, float w, float h, float vx, float vy, Color color, int d, unsigned char flags)
{
	particles.push_back(Particles(x, y, w, h, vx, vy, color, d));
	particles.back().canMove = true;
    particles.back().gravity = flags & (1 << 0);
    particles.back().bounce  = flags & (1 << 1);
}

void World::
addLight(vec2 loc, float radius, Color color)
{
	if (light.size() < 128)
        light.emplace_back(loc, radius, color);
}

void World::
addHole(unsigned int start, unsigned int end)
{
    end = fmin(worldData.size(), end);

	for (unsigned int i = start; i < end; i++)
		worldData[i].groundHeight = 0;
}

void World::
addHill(const ivec2 peak, const unsigned int width)
{
	int start  = peak.x - width / 2,
        end    = start + width,
        offset = 0;
	const float thing = peak.y - worldData[std::clamp(start, 0, static_cast<int>(lineCount))].groundHeight;
    const float period = PI / width;

	if (start < 0) {
        offset = -start;
        start = 0;
    }

    end = fmin(worldData.size(), end);

	for (int i = start; i < end; i++) {
		worldData[i].groundHeight += thing * sin((i - start + offset) * period);
		if (worldData[i].groundHeight > peak.y)
			worldData[i].groundHeight = peak.y;
	}
}

Arena::Arena(void)
{
	generate(800);
	addMob(new Door(), vec2 {100, 100});
	mmob = nullptr;
}

Arena::~Arena(void)
{
    if (mmob != nullptr)
		mmob->die();
	deleteEntities();
}

void Arena::fight(World *leave, const Player *p, Mob *m)
{
    inBattle = true;

    entity.push_back((mmob = m));
	mmob->aggressive = false;

    arenaNest.emplace_back(leave, p->loc);
}

WorldSwitchInfo Arena::exitArena(Player *p)
{
	if (!mmob->isAlive() &&
        p->loc.x + p->width / 2 > mmob->loc.x &&
	    p->loc.x + p->width / 2 < mmob->loc.x + HLINES(12)) {
        auto ret = arenaNest.back();
        arenaNest.pop_back();
        inBattle = !(arenaNest.empty());

        return ret;
    }

    return std::make_pair(this, vec2 {0, 0});
}

static bool loadedLeft = false;
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

/**
 * Loads a world from the given XML file.
 */

World *
loadWorldFromXMLNoTakeover(std::string path)
{
	loadedLeft = true, loadedRight = true;
	auto ret = loadWorldFromXMLNoSave(path);
	loadedLeft = false, loadedRight = false;
	return ret;
}

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

    // no file? -> no world
    if (path.empty())
        return nullptr;

    _currentXML = xmlFolder + path;
	const char *worthless = readFile(_currentXML.c_str());
	_currentXMLRaw = worthless;
	delete[] worthless;

	// create a temporary XMLDocument if this isn't the main world
	if (!loadedLeft && !loadedRight)
		_currentXMLDoc = &currentXMLDoc;
	else
		_currentXMLDoc = new XMLDocument();

	// parse the file
	if (_currentXMLDoc->Parse(_currentXMLRaw.data()) != XML_NO_ERROR)
		UserError("XML Error: Failed to parse file (not your fault though..?)");

    // attempt to load a <World> tag
	if ((wxml = _currentXMLDoc->FirstChildElement("World"))) {
		wxml = wxml->FirstChildElement();
		vil = _currentXMLDoc->FirstChildElement("World")->FirstChildElement("village");
		tmp = new World();
        Indoor = false;
	}

    // attempt to load an <IndoorWorld> tag
    else if ((wxml = _currentXMLDoc->FirstChildElement("IndoorWorld"))) {
		wxml = wxml->FirstChildElement();
		vil = NULL;
		tmp = new World(true);
        Indoor = true;
	}

    // error: can't load a world...
    else
        UserError("XML Error: Cannot find a <World> or <IndoorWorld> tag in " + _currentXML + "!");

    // iterate through world tags
	while (wxml) {
		newEntity = nullptr;
		name = wxml->Name();

        // world linkage
		if (name == "link") {

            // links world to the left
			if ((ptr = wxml->Attribute("left"))) {
				tmp->setToLeft(ptr);

                // load the left world if it isn't
                if (!loadedLeft) {
                    loadedRight = true;
                    currentWorldToLeft = loadWorldFromXMLNoSave(ptr);
                    loadedRight = false;
                } else {
					currentWorldToLeft = nullptr;
				}
			}

            // links world to the right
            else if ((ptr = wxml->Attribute("right"))) {
				tmp->setToRight(ptr);

                // load the right world if it isn't
                if (!loadedRight) {
					loadedLeft = true;
                    currentWorldToRight = loadWorldFromXMLNoSave(ptr);
                    loadedLeft = false;
				} else {
					currentWorldToRight = nullptr;
				}
			}

			// tells what world is outside, if in a structure
			else if (Indoor && (ptr = wxml->Attribute("outside"))) {
//				if (!loadedLeft && !loadedRight)
//					inside.push_back(ptr);
			}

            // error, invalid link tag
            else {
                UserError("XML Error: Invalid <link> tag in " + _currentXML + "!");
			}

		}

        // style tags
        else if (name == "style") {
            // set style folder
			tmp->setStyle(wxml->StrAttribute("folder"));

            // set background folder
			unsigned int bgt;
            if (wxml->QueryUnsignedAttribute("background", &bgt) != XML_NO_ERROR)
                UserError("XML Error: No background given in <style> in " + _currentXML + "!");
			tmp->setBackground(static_cast<WorldBGType>(bgt));

            // set BGM file
            tmp->bgm = wxml->StrAttribute("bgm");
		}

        // world generation (for outdoor areas)
        else if (name == "generation") {
			tmp->generate(wxml->UnsignedAttribute("width") / game::HLINE);
		}

		else if (name == "house") {
			if (Indoor)
				tmp->HouseWidth = wxml->FloatAttribute("width");
			else
				UserError("<house> can only be used with indoor worlds");
		}

		// weather tags
		else if (name == "weather") {
			game::engine.getSystem<WorldSystem>()->setWeather(wxml->GetText());
		}

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

		// time setting
		else if (name == "time" && !(loadedLeft | loadedRight)) {
            game::time::setTickCount(std::stoi(wxml->GetText()));
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

		/**
		 * 	READS DATA ABOUT STRUCTURE CONTAINED IN VILLAGE
		 */

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
}

Village::Village(std::string meme, World *w)
	: name(meme)
{
	start.x = w->getTheWidth() / 2.0f;
	end.x = -start.x;
	in = false;
}



WorldSystem::WorldSystem(void)
	: weather(WorldWeather::None), bgmObj(nullptr) {}

WorldSystem::~WorldSystem(void)
{
    // SDL2_mixer's object
	if (bgmObj != nullptr)
		Mix_FreeMusic(bgmObj);
}

void WorldSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)en;
	(void)ev;
	(void)dt;

    // fade in music if not playing
	if (bgmObj != nullptr && !Mix_PlayingMusic())
		Mix_FadeInMusic(bgmObj, -1, 2000);

    // update player coords
	player->loc.y += player->vel.y * dt;
	player->loc.x += (player->vel.x * player->speed) * dt;

	// update entity coords
	for (auto &e : world->entity) {
        // dont let structures move?
		if (e->type != STRUCTURET && e->canMove) {
			e->loc.x += e->vel.x * dt;
            e->loc.y += e->vel.y * dt;

            // update boolean directions
            e->left = e->vel.x ? (e->vel.x < 0) : e->left;
		} else if (e->vel.y < 0) {
            e->loc.y += e->vel.y * dt;
        }
	}

	partMutex.lock();
	// iterate through particles
    world->particles.remove_if([](const Particles &part) {
		return part.duration <= 0;
    });

    for (auto &pa : world->particles) {
		if (pa.canMove) { // causes overhead
			pa.loc.y += pa.vel.y * dt;
			pa.loc.x += pa.vel.x * dt;

			if (pa.stu != nullptr) {
				if (pa.loc.x >= pa.stu->loc.x && pa.loc.x <= pa.stu->loc.x + pa.stu->width &&
				    pa.loc.y <= pa.stu->loc.y + pa.stu->height * 0.25f)
					pa.duration = 0;
			}
		}
	}
    partMutex.unlock();

	// add entities if need be
	auto& entityPending = world->entityPending;

	if (!entityPending.empty()) {
		while (entityPending.size() > 0) {
			world->entity.push_back(entityPending.back());
			entityPending.pop_back();
		}
	}

	// run detect stuff
	detect(dt);
}

void WorldSystem::render(void)
{
	const auto SCREEN_WIDTH = game::SCREEN_WIDTH;
	const auto SCREEN_HEIGHT = game::SCREEN_HEIGHT;
	const auto HLINE = game::HLINE;

	const ivec2 backgroundOffset = ivec2 {
        static_cast<int>(SCREEN_WIDTH) / 2, static_cast<int>(SCREEN_HEIGHT) / 2
    };

	auto& worldData = currentWorld->worldData;
	auto& star = currentWorld->star;
	auto worldStart = currentWorld->worldStart;

	int iStart, iEnd, pOffset;

    // world width in pixels
	int width = worldData.size() * HLINE;

    // used for alpha values of background textures
    int alpha;

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
	static GLuint starTex = Texture::loadTexture("assets/style/classic/bg/star.png");
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
	}

	Render::worldShader.disable();

	glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.0);
	glUniform4f(Render::worldShader.uniform[WU_ambient], ambient.red, ambient.green, ambient.blue, 1.0);

	Render::worldShader.unuse();

    std::vector<vec3> bg_items;
	std::vector<vec2> bg_tex;

	bgTex++;
	dim2 mountainDim = bgTex.getTextureDim();
    auto xcoord = width / 2 * -1 + offset.x * 0.85f;
	for (unsigned int i = 0; i <= worldData.size() * HLINE / mountainDim.x; i++) {
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

		if (world->isIndoor() && i == 3) {
			static const GLuint tex = Texture::loadTexture(world->styleFolder + "insideWoodHouse.png");
			static const auto dimm = Texture::imageDim(world->styleFolder + "insideWoodHouse.png");
			glBindTexture(GL_TEXTURE_2D, tex);

			bg_items.emplace_back(worldStart, GROUND_HEIGHT_MINIMUM, 7-(i*.1));
	        bg_items.emplace_back(worldStart + world->HouseWidth, GROUND_HEIGHT_MINIMUM,	7-(i*.1));
	        bg_items.emplace_back(worldStart + world->HouseWidth, GROUND_HEIGHT_MINIMUM + dimm.y, 7-(i*.1));

	        bg_items.emplace_back(worldStart + world->HouseWidth, GROUND_HEIGHT_MINIMUM + dimm.y, 7-(i*.1));
	        bg_items.emplace_back(worldStart, GROUND_HEIGHT_MINIMUM + dimm.y, 7-(i*.1));
	        bg_items.emplace_back(worldStart, GROUND_HEIGHT_MINIMUM,	7-(i*.1));
		} else {
			for (int j = worldStart; j <= -worldStart; j += dim.x) {
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
    pOffset = (offset.x + player->width / 2 - worldStart) / HLINE;

    // only draw world within player vision
    iStart = std::clamp(static_cast<int>(pOffset - (SCREEN_WIDTH / 2 / HLINE) - GROUND_HILLINESS),
	                    0, static_cast<int>(world->lineCount));
	iEnd = std::clamp(static_cast<int>(pOffset + (SCREEN_WIDTH / 2 / HLINE)),
                      0, static_cast<int>(world->lineCount));

    // draw the dirt
    bgTex++;
    std::vector<std::pair<vec2,vec3>> c;

    for (int i = iStart; i < iEnd; i++) {
        if (worldData[i].groundHeight <= 0) { // TODO holes (andy)
            worldData[i].groundHeight = GROUND_HEIGHT_MINIMUM - 1;
            glColor4ub(0, 0, 0, 255);
        } else {
            safeSetColorA(150, 150, 150, 255);
        }

        int ty = worldData[i].groundHeight / 64 + worldData[i].groundColor;

		c.push_back(std::make_pair(vec2(0, 0), vec3(worldStart + HLINES(i),         worldData[i].groundHeight - GRASS_HEIGHT, -4.0f)));
        c.push_back(std::make_pair(vec2(1, 0), vec3(worldStart + HLINES(i) + HLINE, worldData[i].groundHeight - GRASS_HEIGHT, -4.0f)));
        c.push_back(std::make_pair(vec2(1, ty),vec3(worldStart + HLINES(i) + HLINE, 0,                                        -4.0f)));

        c.push_back(std::make_pair(vec2(1, ty),vec3(worldStart + HLINES(i) + HLINE, 0,                                        -4.0f)));
        c.push_back(std::make_pair(vec2(0, ty),vec3(worldStart + HLINES(i),         0,                                        -4.0f)));
        c.push_back(std::make_pair(vec2(0, 0), vec3(worldStart + HLINES(i),         worldData[i].groundHeight - GRASS_HEIGHT, -4.0f)));

        if (worldData[i].groundHeight == GROUND_HEIGHT_MINIMUM - 1)
            worldData[i].groundHeight = 0;
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

	if (!world->isIndoor()) {
		bgTex++;
	    safeSetColorA(255, 255, 255, 255);

	    c.clear();
	    std::vector<GLfloat> grassc;
	    std::vector<GLfloat> grasst;

		for (int i = iStart; i < iEnd; i++) {
        	auto wd = worldData[i];
	        auto gh = wd.grassHeight;

			// flatten the grass if the player is standing on it.
			if (!wd.grassUnpressed) {
				gh[0] /= 4;
				gh[1] /= 4;
			}

			// actually draw the grass.
	        if (wd.groundHeight) {
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
		vec2 ll = vec2 {worldStart, GROUND_HEIGHT_MINIMUM};
		Render::drawRect(ll, vec2 {ll.x + world->HouseWidth, ll.y + 4}, -3);
		Render::worldShader.unuse();
	}

	player->draw();
}

void WorldSystem::setWorld(World *w)
{
	world = w;

	bgFiles.clear();

	for (int i = 0; i < 9; i++) { 
		int idx = /*((int)w->bgType * 9) +*/ i;
		bgFiles.push_back(w->styleFolder + bgPaths[idx]);
	}

	bgTex = TextureIterator(bgFiles);
}


void WorldSystem::receive(const BGMToggleEvent &bte)
{
	if (bte.world == nullptr || bgmObjFile != bte.file) {
		Mix_FadeOutMusic(800);

		if (bgmObj != nullptr)
			Mix_FreeMusic(bgmObj);

		bgmObjFile = bte.file;
		bgmObj = Mix_LoadMUS(bgmObjFile.c_str());
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

void WorldSystem::enterWorld(World *w)
{
	if (w != nullptr) {
		outside = world;
		world = w;
	}
}

void WorldSystem::leaveWorld(void)
{
	world = outside;
} 

void WorldSystem::singleDetect(Entity *e, entityx::TimeDelta dt)
{
	std::string killed;
	unsigned int i;
	int l;

	auto& worldData = world->worldData;

	if (e == nullptr || !(e->isAlive()))
		return;

	// kill dead entities
	if (e->health <= 0) {
        // die
        e->die();
		if (inBattle && e->type == MOBT)
			Mobp(e)->onDeath();

        // delete the entity
		for (i = 0; i < world->entity.size(); i++) {
			if (world->entity[i] == e) {
				switch (e->type) {
				case STRUCTURET:
					killed = " structure";
                    break;
                case NPCT:
					killed = "n NPC";
					break;
				case MOBT:
					killed = " mob";
					break;
				case OBJECTT:
					killed = "n object";
					break;
				default:
					break;
				}

				std::cout << "Killed a" << killed << "...\n";
				world->entity.erase(world->entity.begin() + i);
				return;
			}
		}

        // exit on player death
		std::cout << "RIP " << e->name << ".\n";
		exit(0);
	}

	// collision / gravity: handle only living entities
	else {

        // forced movement gravity (sword hits)
        e->handleHits();

		// calculate the line that this entity is currently standing on
        l = std::clamp(static_cast<int>((e->loc.x + e->width / 2 - world->worldStart) / game::HLINE),
                       0,
                       static_cast<int>(world->lineCount));

		// if the entity is under the world/line, pop it back to the surface
		if (e->loc.y < worldData[l].groundHeight) {
            int dir = e->vel.x < 0 ? -1 : 1;
            if (l + (dir * 2) < static_cast<int>(worldData.size()) &&
                worldData[l + (dir * 2)].groundHeight - 30 > worldData[l + dir].groundHeight) {
                e->loc.x -= (PLAYER_SPEED_CONSTANT + 2.7f) * e->speed * 2 * dir;
                e->vel.x = 0;
            } else {
                e->loc.y = worldData[l].groundHeight - 0.001f * dt;
		        e->ground = true;
		        e->vel.y = 0;
            }

		}

        // handle gravity if the entity is above the line
        else {
			if (e->type == STRUCTURET) {
				e->loc.y = worldData[l].groundHeight;
				e->vel.y = 0;
				e->ground = true;
				return;
			} else if (e->vel.y > -2) {
                e->vel.y -= GRAVITY_CONSTANT * dt;
            }
		}

		// insure that the entity doesn't fall off either edge of the world.
        if (e->loc.x < world->worldStart) {
			e->vel.x = 0;
			e->loc.x = world->worldStart + HLINES(0.5f);
		} else if (e->loc.x + e->width + game::HLINE > -((int)world->worldStart)) {
			e->vel.x = 0;
			e->loc.x = -((int)world->worldStart) - e->width - game::HLINE;
		}
	}
}

void WorldSystem::detect2(entityx::TimeDelta dt)
{
	game::entities.each<Position, Direction, Health, Solid>(
	    [&](entityx::Entity e, Position &loc, Direction &vel, Health &health, Solid &dim) {
	
		(void)e;
		
		if (health.health <= 0)
			UserError("die mofo");

		// get the line the entity is on
		int line = std::clamp(static_cast<int>((loc.x + dim.width / 2 - world->worldStart) / game::HLINE),
		                      0,
		                      static_cast<int>(world->lineCount));

		// make sure entity is above ground
		auto& data = world->worldData;
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
        if (loc.x < world->worldStart) {
			vel.x = 0;
			loc.x = world->worldStart + HLINES(0.5f);
		} else if (loc.x + dim.width + game::HLINE > -((int)world->worldStart)) {
			vel.x = 0;
			loc.x = -((int)world->worldStart) - dim.width - game::HLINE;
		}
	});
}

void WorldSystem::detect(entityx::TimeDelta dt)
{
	int l;

	// handle the player
    singleDetect(player, dt);

    // handle other entities
	for (auto &e : world->entity)
		singleDetect(e, dt);

	partMutex.lock();
    // handle particles
	for (auto &part : world->particles) {
		// get particle's current world line
		l = std::clamp(static_cast<int>((part.loc.x + part.width / 2 - world->worldStart) / game::HLINE),
                       0,
                       static_cast<int>(world->lineCount - 1));
		part.update(GRAVITY_CONSTANT, world->worldData[l].groundHeight);
	}

	// handle particle creation
	for (auto &e : world->entity) {
		if (e->type == STRUCTURET) {
			auto b = dynamic_cast<Structures *>(e);
			switch (b->bsubtype) {
			case FOUNTAIN:
				for (unsigned int r = (randGet() % 25) + 11; r--;) {
					world->addParticle(randGet() % HLINES(3) + b->loc.x + b->width / 2,	// x
								b->loc.y + b->height,								// y
								HLINES(1.25),										// width
								HLINES(1.25),										// height
								randGet() % 7 * .01 * (randGet() % 2 == 0 ? -1 : 1),	// vel.x
								randGet() % 1 ? (8 + randGet() % 6) * .05 : (4 + randGet() % 6) * .05,// vel.y
								{ 0, 0, 255 },										// RGB color
								2500												// duration (ms)
								);
					world->particles.back().fountain = true;
					world->particles.back().stu = b;
				}
				break;
			case FIRE_PIT:
				for(unsigned int r = (randGet() % 20) + 11; r--;) {
					world->addParticle(randGet() % (int)(b->width / 2) + b->loc.x + b->width / 4,	// x
								b->loc.y + HLINES(3),										// y
								game::HLINE,       											// width
								game::HLINE,												// height
								randGet() % 3 * .01 * (randGet() % 2 == 0 ? -1 : 1),		// vel.x
								(4 + randGet() % 6) * .005,									// vel.y
								{ 255, 0, 0 },												// RGB color
								400															// duration (ms)
								);
					world->particles.back().gravity = false;
					world->particles.back().behind  = true;
					world->particles.back().stu = b;
				}
				break;
			default:
				break;
			}
		}
	}
	partMutex.unlock();

	// draws the village welcome message if the player enters the village bounds
	for (auto &v : world->village) {
		if (player->loc.x > v.start.x && player->loc.x < v.end.x) {
            if (!v.in) {
			    ui::passiveImportantText(5000, "Welcome to %s", v.name.c_str());
			    v.in = true;
		    }
        } else {
            v.in = false;
        }
	}
}

