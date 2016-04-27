#include <world.hpp>

/* ----------------------------------------------------------------------------
** Includes section
** --------------------------------------------------------------------------*/

// standard library headers
#include <algorithm>
#include <sstream>

// local game headers
#include <ui.hpp>
#include <gametime.hpp>

// local library headers
#include <tinyxml2.h>
using namespace tinyxml2;

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

// externally referenced in main.cpp
const unsigned int DAY_CYCLE = 12000;
int worldShade = 0;

// externally referenced in entities.cpp
const float PLAYER_SPEED_CONSTANT = 0.150f;
const float GRAVITY_CONSTANT      = 0.001f;

// ground-generating constants
static const float GROUND_HEIGHT_INITIAL =  80.0f;
static const float GROUND_HEIGHT_MINIMUM =  60.0f;
static const float GROUND_HEIGHT_MAXIMUM = 110.0f;
static const float GROUND_HILLINESS      =  10.0f;

// defines grass height in HLINEs
static const unsigned int GRASS_HEIGHT = 4;

// indoor world constants
static const unsigned int INDOOR_FLOOR_THICKNESS = 50;
static const unsigned int INDOOR_FLOOR_HEIGHTT = 400;

// the path of the currently loaded XML file, externally referenced in places
std::string currentXML;

// contains the current world's weather, extern'd in ui.cpp, main.cpp, ..?
WorldWeather weather = WorldWeather::Sunny;

// keeps track of pathnames of XML file'd worlds the player has left to enter structures
static std::vector<std::string> inside;

// keeps track of information of worlds the player has left to enter arenas
static std::vector<World *> battleNest;
static std::vector<vec2>    battleNestLoc;

// pathnames of images for world themes
static const unsigned int BG_PATHS_ENTRY_SIZE = 9;
static const std::string bgPaths[][BG_PATHS_ENTRY_SIZE] = {
    {"bg.png",					// Daytime background
     "bgn.png",					// Nighttime background
     "bgFarMountain.png",		// Furthest layer
     "forestTileFar.png",		// Furthest away Tree Layer
     "forestTileBack.png",		// Closer layer
     "forestTileMid.png",		// Near layer
     "forestTileFront.png",		// Closest layer
     "dirt.png",				// Dirt
     "grass.png"},				// Grass
    {"bgWoodTile.png",
     "bgWoodTile.png",
     "bgWoodTile.png",
     "bgWoodTile.png",
     "bgWoodTile.png",
     "bgWoodTile.png",
     "bgWoodTile.png",
     "bgWoodTile.png"}
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

// externs
extern int  commonAIFunc(NPC *);		// gameplay.cpp
extern void commonTriggerFunc(Mob *);	// gameplay.cpp
extern void commonPageFunc(Mob *);		// gameplay.cpp

/**
 * Creates a world object.
 * Note that all this does is nullify a pointer...
 */
World::
World(void)
{
    bgmObj = nullptr;
}

/**
 * The world destructor.
 * This will free objects used by the world itself, then free the vectors of
 * entity-related objects.
 */
World::
~World(void)
{
    // SDL2_mixer's object
	if (bgmObj != nullptr)
		Mix_FreeMusic(bgmObj);

	delete bgTex;
	deleteEntities();
}

/**
 * The entity vector destroyer.
 * This function will free all memory used by all entities, and then empty the
 * vectors they were stored in.
 */
void World::
deleteEntities(void)
{
    // free mobs
    while (!mob.empty()) {
		delete mob.back();
		mob.pop_back();
	}

    // free npcs
	merchant.clear(); // TODO
	while (!npc.empty()) {
		delete npc.back();
		npc.pop_back();
	}

    // free structures
	while (!build.empty()) {
		delete build.back();
		build.pop_back();
	}

    // free objects
	object.clear();
    // free particles
	particles.clear();
    // clear light array
	light.clear();
    // free villages
	village.clear();
    // clear entity array
	entity.clear();
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

    // give every GROUND_HILLINESSth entry a groundHeight value
    for (; wditer < std::end(worldData); wditer += GROUND_HILLINESS)
        (*(wditer - GROUND_HILLINESS)).groundHeight = (*wditer).groundHeight + (getRand() % 8 - 4);

    // create slopes from the points that were just defined, populate the rest of the WorldData structure
    for (wditer = std::begin(worldData) + 1; wditer < std::end(worldData); wditer++){
        auto w = &*(wditer);

        if (w->groundHeight != 0)
            geninc = ((w + static_cast<int>(GROUND_HILLINESS))->groundHeight - w->groundHeight) / GROUND_HILLINESS;

        w->groundHeight   = fmin(fmax((w - 1)->groundHeight + geninc, GROUND_HEIGHT_MINIMUM), GROUND_HEIGHT_MAXIMUM);
        w->groundColor    = getRand() % 32 / 8;
        w->grassUnpressed = true;
        w->grassHeight[0] = (getRand() % 16) / 3 + 2;
        w->grassHeight[1] = (getRand() % 16) / 3 + 2;
    }

    // define x-coordinate of world's leftmost 'line'
    worldStart = (width - GROUND_HILLINESS) * game::HLINE / 2 * -1;

    // create empty star array, should be filled here as well...
	star = std::vector<vec2> (100, vec2 { 0, 400 });
	for (auto &s : star) {
		s.x = (getRand() % (-worldStart * 2)) + worldStart;
		s.y = (getRand() % game::SCREEN_HEIGHT) + 100;
	}
}

/**
 * The world draw function.
 * This function will draw the background layers, entities, and player to the
 * screen.
 */
void World::
draw(Player *p)
{
    auto SCREEN_WIDTH = game::SCREEN_WIDTH;
	auto SCREEN_HEIGHT = game::SCREEN_HEIGHT;
    auto HLINE = game::HLINE;

    const ivec2 backgroundOffset = ivec2 {
        static_cast<int>(SCREEN_WIDTH) / 2, static_cast<int>(SCREEN_HEIGHT) / 2
    };

    // iterators ranges
    int iStart, iEnd;

    // shade value for draws -- may be unnecessary
	int shadeBackground = -worldShade;

    // player's offset in worldData[]
	int pOffset;

    // world width in pixels
	int width = worldData.size() * HLINE;

    // used for alpha values of background textures
    int alpha;

	// shade value for GLSL
	float shadeAmbient = std::max(0.0f, static_cast<float>(-worldShade) / 50 + 0.5f); // 0 to 1.5f

	if (shadeAmbient > 0.9f)
		shadeAmbient = 1;

	// draw background images.
	glEnable(GL_TEXTURE_2D);

	// the sunny wallpaper is faded with the night depending on tickCount
	bgTex->bind(0);
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

	safeSetColorA(255, 255, 255, alpha);
	glBegin(GL_QUADS);
		glTexCoord2i(0, 0); glVertex2i(offset.x - backgroundOffset.x - 5, offset.y + backgroundOffset.y);
		glTexCoord2i(1, 0); glVertex2i(offset.x + backgroundOffset.x + 5, offset.y + backgroundOffset.y);
		glTexCoord2i(1, 1); glVertex2i(offset.x + backgroundOffset.x + 5, offset.y - backgroundOffset.y);
		glTexCoord2i(0, 1); glVertex2i(offset.x - backgroundOffset.x - 5, offset.y - backgroundOffset.y);
	glEnd();

	bgTex->bindNext();
	safeSetColorA(255, 255, 255, !alpha ? 255 : worldShade * 4);
	glBegin(GL_QUADS);
        glTexCoord2i(0, 0); glVertex2i(offset.x - backgroundOffset.x - 5, offset.y + backgroundOffset.y);
        glTexCoord2i(1, 0); glVertex2i(offset.x + backgroundOffset.x + 5, offset.y + backgroundOffset.y);
        glTexCoord2i(1, 1); glVertex2i(offset.x + backgroundOffset.x + 5, offset.y - backgroundOffset.y);
        glTexCoord2i(0, 1); glVertex2i(offset.x - backgroundOffset.x - 5, offset.y - backgroundOffset.y);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	// draw the stars if the time deems it appropriate
	if (worldShade > 0) {
		safeSetColorA(255, 255, 255, 255 - (getRand() % 30 - 15));

        auto xcoord = offset.x * 0.9f;
		for (auto &s : star)
			glRectf(s.x + xcoord, s.y, s.x + xcoord + HLINE, s.y + HLINE);
	}

	// draw remaining background items
	glEnable(GL_TEXTURE_2D);

	bgTex->bindNext();
	safeSetColorA(150 + shadeBackground * 2, 150 + shadeBackground * 2, 150 + shadeBackground * 2, 255);
	glBegin(GL_QUADS); {
        auto xcoord = width / 2 * -1 + offset.x * 0.85f;
		for (unsigned int i = 0; i <= worldData.size() * HLINE / 1920; i++) {
			glTexCoord2i(0, 1); glVertex2i(1920 * i       + xcoord, GROUND_HEIGHT_MINIMUM);
            glTexCoord2i(1, 1); glVertex2i(1920 * (i + 1) + xcoord, GROUND_HEIGHT_MINIMUM);
			glTexCoord2i(1, 0); glVertex2i(1920 * (i + 1) + xcoord, GROUND_HEIGHT_MINIMUM + 1080);
			glTexCoord2i(0, 0); glVertex2i(1920 * i       + xcoord, GROUND_HEIGHT_MINIMUM + 1080);
		}
	} glEnd();

	for (unsigned int i = 0; i < 4; i++) {
		bgTex->bindNext();
		safeSetColorA(bgDraw[i][0] + shadeBackground * 2,
                      bgDraw[i][0] + shadeBackground * 2,
                      bgDraw[i][0] + shadeBackground * 2,
                      bgDraw[i][1]
                      );
		glBegin(GL_QUADS); {
            auto xcoord = offset.x * bgDraw[i][2];
			for (int j = worldStart; j <= -worldStart; j += 600) {
                glTexCoord2i(0, 1); glVertex2i(j       + xcoord, GROUND_HEIGHT_MINIMUM);
				glTexCoord2i(1, 1); glVertex2i(j + 600 + xcoord, GROUND_HEIGHT_MINIMUM);
				glTexCoord2i(1, 0); glVertex2i(j + 600 + xcoord, GROUND_HEIGHT_MINIMUM + 400);
				glTexCoord2i(0, 0); glVertex2i(j       + xcoord, GROUND_HEIGHT_MINIMUM + 400);
			}
		} glEnd();
	}

	glDisable(GL_TEXTURE_2D);

	// draw black under backgrounds (y-coordinate)
	glColor3ub(0, 0, 0);
	glRectf(worldStart, GROUND_HEIGHT_MINIMUM, -worldStart, 0);

	// draw particles and buildings
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorIndex);
    glUniform1i(glGetUniformLocation(shaderProgram, "sampler"), 0);
    glUseProgram(shaderProgram);

    for (auto &p : particles) {
        if (p.behind)
            p.draw();
    }

    glUseProgram(0);

	for (auto &b : build) {
        if (b->bsubtype == STALL_MARKET) {
            for (auto &n : npc) {
                if (n->type == MERCHT && static_cast<Merchant *>(n)->inside == b) {
                    n->draw();
                    break;
                }
            }
        }
        b->draw();
    }

    for (auto &l : light) {
        if (l.belongsTo) {
            l.loc.x = l.following->loc.x + SCREEN_WIDTH / 2;
            l.loc.y = l.following->loc.y;
        }
        if (l.flame) {
            l.fireFlicker = 0.9f + ((rand()% 2 ) / 10.0f);
            l.fireLoc.x = l.loc.x + (rand() % 2 - 1) * 3;
            l.fireLoc.y = l.loc.y + (rand() % 2 - 1) * 3;
        } else {
            l.fireFlicker = 1;
        }
    }

    // draw light elements
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    bgTex->bindNext();

    std::unique_ptr<GLfloat[]> pointArrayBuf = std::make_unique<GLfloat[]> (2 * (light.size()));
	auto pointArray = pointArrayBuf.get();
    GLfloat flameArray[64];

	for (unsigned int i = 0; i < light.size(); i++) {
        if (light[i].flame) {
    		pointArray[2 * i    ] = light[i].fireLoc.x - offset.x;
    		pointArray[2 * i + 1] = light[i].fireLoc.y;
        }else{
            pointArray[2 * i    ] = light[i].loc.x - offset.x;
            pointArray[2 * i + 1] = light[i].loc.y;
        }
	}

    for (unsigned int i = 0; i < light.size(); i++)
        flameArray[i] = light[i].fireFlicker;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "sampler"), 0);
	glUniform1f(glGetUniformLocation(shaderProgram, "amb"), shadeAmbient);

	if (light.empty())
		glUniform1i(glGetUniformLocation(shaderProgram, "numLight"), 0);
	else {
		glUniform1i (glGetUniformLocation(shaderProgram, "numLight"), light.size());
		glUniform2fv(glGetUniformLocation(shaderProgram, "lightLocation"), light.size(), pointArray);
		glUniform3f (glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform1fv(glGetUniformLocation(shaderProgram,"fireFlicker"), light.size(),flameArray);
	}

    // get the line that the player is currently standing on
    pOffset = (offset.x + p->width / 2 - worldStart) / HLINE;

    // only draw world within player vision
    iStart = static_cast<int>(fmax(pOffset - (SCREEN_WIDTH / 2 / HLINE) - GROUND_HILLINESS, 0));
    iEnd   = static_cast<int>(fmin(pOffset + (SCREEN_WIDTH / 2 / HLINE) + GROUND_HILLINESS + HLINE, worldData.size()));
    iEnd = static_cast<int>(fmax(iEnd, GROUND_HILLINESS));

    // draw the dirt
	glBegin(GL_QUADS);
        //std::for_each(std::begin(worldData) + iStart, std::begin(worldData) + iEnd, [&](WorldData wd) {
        for (int i = iStart; i < iEnd; i++) {
            if (worldData[i].groundHeight <= 0) {
                worldData[i].groundHeight = GROUND_HEIGHT_MINIMUM - 1;
                glColor4ub(0, 0, 0, 255);
            } else {
                safeSetColorA(150, 150, 150, 255);
            }

            int ty = worldData[i].groundHeight / 64 + worldData[i].groundColor;
            glTexCoord2i(0, 0);  glVertex2i(worldStart + i * HLINE         , worldData[i].groundHeight - GRASS_HEIGHT);
            glTexCoord2i(1, 0);  glVertex2i(worldStart + i * HLINE + HLINE , worldData[i].groundHeight - GRASS_HEIGHT);
            glTexCoord2i(1, ty); glVertex2i(worldStart + i * HLINE + HLINE, 0);
            glTexCoord2i(0, ty); glVertex2i(worldStart + i * HLINE	      , 0);

            if (worldData[i].groundHeight == GROUND_HEIGHT_MINIMUM - 1)
                worldData[i].groundHeight = 0;
        }//);
	glEnd();

	glUseProgram(0);
	glDisable(GL_TEXTURE_2D);

	// draw the grass
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	bgTex->bindNext();
	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "sampler"), 0);
    safeSetColorA(255, 255, 255, 255);

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
    		glBegin(GL_QUADS);
    			glTexCoord2i(0, 0); glVertex2i(worldStart + i * HLINE            , wd.groundHeight + gh[0]);
    			glTexCoord2i(1, 0); glVertex2i(worldStart + i * HLINE + HLINE / 2, wd.groundHeight + gh[0]);
    			glTexCoord2i(1, 1); glVertex2i(worldStart + i * HLINE + HLINE / 2, wd.groundHeight - GRASS_HEIGHT);
    			glTexCoord2i(0, 1); glVertex2i(worldStart + i * HLINE		     , wd.groundHeight - GRASS_HEIGHT);
    			glTexCoord2i(0, 0); glVertex2i(worldStart + i * HLINE + HLINE / 2, wd.groundHeight + gh[1]);
    			glTexCoord2i(1, 0); glVertex2i(worldStart + i * HLINE + HLINE    , wd.groundHeight + gh[1]);
    			glTexCoord2i(1, 1); glVertex2i(worldStart + i * HLINE + HLINE    , wd.groundHeight - GRASS_HEIGHT);
    			glTexCoord2i(0, 1); glVertex2i(worldStart + i * HLINE + HLINE / 2, wd.groundHeight - GRASS_HEIGHT);
    		glEnd();
        }
	}

	glUseProgram(0);
	glDisable(GL_TEXTURE_2D);

	// draw particles
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorIndex);
    glUniform1i(glGetUniformLocation(shaderProgram, "sampler"), 0);
    glUseProgram(shaderProgram);

	for (auto &p : particles) {
        if (!p.behind)
            p.draw();
    }

    glUseProgram(0);

    // draw remaining entities
	for (auto &n : npc) {
		if (n->type != MERCHT)
            n->draw();
    }

	for (auto &m : mob)
		m->draw();

	for (auto &o : object)
		o.draw();

	// flatten grass under the player if the player is on the ground
	if (p->ground) {
		for (unsigned int i = 0; i < worldData.size(); i++)
			worldData[i].grassUnpressed = !(i < static_cast<unsigned int>(pOffset + 6) && i > static_cast<unsigned int>(pOffset - 6));
	} else {
		for (auto &wd : worldData)
			wd.grassUnpressed = true;
	}

    // draw the player
	p->draw();
}

/**
 * Handles physics and such for a single entity.
 * This function is kept private, as World::detect() should be used instead to
 * handle stuffs for all entities at once.
 */
void World::
singleDetect(Entity *e)
{
	std::string killed;
	unsigned int i;
	int l;

    auto deltaTime = game::time::getDeltaTime();

	// kill dead entities
	if (!e->isAlive()) {
        return;
    } else if (e->health <= 0) {
        // die
        e->die();

        // delete the entity
		for (i = 0; i < entity.size(); i++) {
			if (entity[i] == e) {
				switch (e->type) {
				case STRUCTURET:
					killed = "structure";
                    build.erase(std::find(std::begin(build), std::end(build), e));
                    break;
                case NPCT:
					killed = "NPC";
					npc.erase(std::find(std::begin(npc), std::end(npc), e));
					break;
				case MOBT:
					killed = "mob";
                    // TODO
					break;
				case OBJECTT:
					killed = "object";
                    object.erase(std::find(std::begin(object), std::end(object), *Objectp(e)));
					break;
				default:
					break;
				}

				std::cout << "Killed a " << killed << "...\n";
				entity.erase(entity.begin() + i);
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
		l = static_cast<int>(fmax((e->loc.x + e->width / 2 - worldStart) / game::HLINE, 0));
		l = static_cast<int>(fmin(l, lineCount - 1));

		// if the entity is under the world/line, pop it back to the surface
		if (e->loc.y < worldData[l].groundHeight) {
            int dir = e->vel.x < 0 ? -1 : 1;
            if (l + (dir * 2) < static_cast<int>(worldData.size()) &&
                worldData[l + (dir * 2)].groundHeight - 30 > worldData[l + dir].groundHeight) {
                e->loc.x -= (PLAYER_SPEED_CONSTANT + 2.7f) * e->speed * 2 * dir;
                e->vel.x = 0;
            } else {
                e->loc.y = worldData[l].groundHeight - 0.001f * deltaTime;
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
                e->vel.y -= GRAVITY_CONSTANT * deltaTime;
            }
		}

		// insure that the entity doesn't fall off either edge of the world.
		if (e->loc.x < worldStart) {
			e->vel.x = 0;
			e->loc.x = worldStart + game::HLINE / 2;
		} else if (e->loc.x + e->width + game::HLINE > worldStart + worldStart * -2) {
			e->vel.x = 0;
			e->loc.x = worldStart + worldStart * -2 - e->width - game::HLINE;
		}
	}
}

/**
 * Handle entity logic for the world.
 *
 * This function runs World::singleDetect() for the player and every entity
 * currently in a vector of this world. Particles and village entrance/exiting
 * are also handled here.
 */
void World::
detect(Player *p)
{
	int l;

	// handle the player
	std::thread(&World::singleDetect, this, p).detach();

    // handle other entities
	for (auto &e : entity)
		std::thread(&World::singleDetect, this, e).detach();

    // handle particles
	for (auto &part : particles) {
		// get particle's current world line
		l = (int)fmax((part.loc.x + part.width / 2 - worldStart) / game::HLINE, 0);
        l = (int)fmin(lineCount - 1, l);
		part.update(GRAVITY_CONSTANT, worldData[l].groundHeight);
	}

	// handle particle creation
	for (auto &b : build) {
		switch (b->bsubtype) {
		case FOUNTAIN:
			for (unsigned int r = (randGet() % 25) + 11; r--;) {
				addParticle(randGet() % HLINES(3) + b->loc.x + b->width / 2,	// x
							b->loc.y + b->height,								// y
							HLINES(1.25),										// width
							HLINES(1.25),										// height
							randGet() % 7 * .01 * (randGet() % 2 == 0 ? -1 : 1),	// vel.x
							(4 + randGet() % 6) * .05,							// vel.y
							{ 0, 0, 255 },										// RGB color
							2500												// duration (ms)
							);
				particles.back().fountain = true;
			}
			break;
		case FIRE_PIT:
			for(unsigned int r = (randGet() % 20) + 11; r--;) {
				addParticle(randGet() % (int)(b->width / 2) + b->loc.x + b->width / 4,	// x
							b->loc.y + HLINES(3),										// y
							game::HLINE,       											// width
							game::HLINE,												// height
							randGet() % 3 * .01 * (randGet() % 2 == 0 ? -1 : 1),		// vel.x
							(4 + randGet() % 6) * .005,									// vel.y
							{ 255, 0, 0 },												// RGB color
							400															// duration (ms)
							);
				particles.back().gravity = false;
				particles.back().behind  = true;
			}
			break;
		default:
			break;
		}
	}

	// draws the village welcome message if the player enters the village bounds
	for (auto &v : village) {
		if (p->loc.x > v.start.x && p->loc.x < v.end.x) {
            if (!v.in) {
			    ui::passiveImportantText(5000, "Welcome to %s", v.name.c_str());
			    v.in = true;
		    }
        } else {
            v.in = false;
        }
	}
}

/**
 * Updates all entity and player coordinates with their velocities.
 * Also handles music fading, although that could probably be placed elsewhere.
 */
void World::
update(Player *p, unsigned int delta)
{
    // update player coords
	p->loc.y += p->vel.y			 * delta;
	p->loc.x +=(p->vel.x * p->speed) * delta;

    // handle high-ness
	if (p->loc.y > 5000)
        UserError("Too high for me m8.");

	// update entity coords
	for (auto &e : entity) {
        // dont let structures move?
		if (e->type != STRUCTURET && e->canMove) {
			e->loc.x += e->vel.x * delta;
            e->loc.y += e->vel.y * delta;

            // update boolean directions
            e->left = e->vel.x ? (e->vel.x < 0) : e->left;
		} else if (e->vel.y < 0) {
            e->loc.y += e->vel.y * delta;
        }
	}
    // iterate through particles
    particles.erase(std::remove_if(particles.begin(), particles.end(), [&delta](Particles &part){
                        return part.kill(delta);
                    }),
                    particles.end());

    for (auto part = particles.begin(); part != particles.end(); part++) {
        auto pa = *part;

		if (pa.canMove) {
			(*part).loc.y += pa.vel.y * delta;
			(*part).loc.x += pa.vel.x * delta;

            if (std::any_of(std::begin(build), std::end(build), [pa](Structures *s) {
                    return (s->bsubtype == FOUNTAIN) &&
                           (pa.loc.x >= s->loc.x) && (pa.loc.x <= s->loc.x + s->width) &&
                           (pa.loc.y <= s->loc.y + s->height * 0.25f);
                })) {
                particles.erase(part);
            }
		}
	}

    // handle music fades
	if (!Mix_PlayingMusic())
		Mix_FadeInMusic(bgmObj, -1, 2000);
}

/**
 * Get's the world's width in pixels.
 */
int World::
getTheWidth(void) const
{
	return (worldStart * -2);
}

/**
 * Get a pointer to the most recently created light.
 * Meant to be non-constant.
 */
Light *World::
getLastLight(void)
{
    return &light.back();
}

/**
 * Get a pointer to the most recently created mob.
 * Meant to be non-constant.
 */
Mob *World::
getLastMob(void)
{
    return mob.back();
}

/**
 * Get the interactable entity that is closest to the entity provided.
 */
Entity *World::
getNearInteractable(Entity &e)
{
    auto n = std::find_if(std::begin(entity), std::end(entity), [&](Entity *&a) {
        return ((a->type == MOBT) || (a->type == NPCT) || a->type == MERCHT) &&
               e.isNear(*a) && (e.left ? (a->loc.x < e.loc.x) : (a->loc.x > e.loc.x));
    });

    return n == std::end(entity) ? nullptr : *n;
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
    if (index < 0)
        return build.back()->loc;
    else if ((unsigned)index >= build.size())
        return vec2 {0, 0};

    return build[index]->loc;
}

/**
 * Saves world data to a file.
 */
void World::save(void){
	std::string data;
	std::string save = currentXML + ".dat";
	std::ofstream out (save, std::ios::out | std::ios::binary);

	std::cout << "Saving to " << save << " ..." << '\n';

    // save npcs
	for (auto &n : npc) {
		data.append(std::to_string(n->dialogIndex) + "\n");
		data.append(std::to_string((int)n->loc.x) + "\n");
		data.append(std::to_string((int)n->loc.y) + "\n");
	}

    // save structures
	for (auto &b : build) {
		data.append(std::to_string((int)b->loc.x) + "\n");
		data.append(std::to_string((int)b->loc.y) + "\n");
	}

    // save mobs
	for (auto &m : mob) {
		data.append(std::to_string((int)m->loc.x) + "\n");
		data.append(std::to_string((int)m->loc.y) + "\n");
		data.append(std::to_string((int)m->isAlive()) + "\n");
	}

    // wrap up
	data.append("dOnE\0");
	out.write(data.data(), data.size());
	out.close();
}

void World::load(void){
	std::string save, data, line;
	const char *filedata;

	save = currentXML + ".dat";
	filedata = readFile(save.c_str());
	data = filedata;
	std::istringstream iss (data);

	for(auto &n : npc){
		std::getline(iss,line);
		if(line == "dOnE")return;
		if ((n->dialogIndex = std::stoi(line)) != 9999)
			n->addAIFunc(false);

		std::getline(iss,line);
		if(line == "dOnE")return;
		n->loc.x = std::stoi(line);
		std::getline(iss,line);
		if(line == "dOnE")return;
		n->loc.y = std::stoi(line);
	}

	for(auto &b : build){
		std::getline(iss,line);
		if(line == "dOnE")return;
		b->loc.x = std::stoi(line);
		std::getline(iss,line);
		if(line == "dOnE")return;
		b->loc.y = std::stoi(line);
	}

	for(auto &m : mob){
		std::getline(iss,line);
		if(line == "dOnE")return;
		m->loc.x = std::stoi(line);
		std::getline(iss,line);
		if(line == "dOnE")return;
		m->loc.y = std::stoi(line);
		std::getline(iss,line);
		if(line == "dOnE")return;
        if (!std::stoi(line))
            m->die();
	}

	while(std::getline(iss,line)){
		if(line == "dOnE")
			break;
	}

	delete[] filedata;
}

/**
 * Toggle play/stop of the background music.
 * If new music is to be played a crossfade will occur, otherwise... uhm.
 */
void World::
bgmPlay(World *prev) const
{
	if (prev) {
        if (bgm != prev->bgm) {
		    // new world, new music
		    Mix_FadeOutMusic(800);
		    Mix_PlayMusic(bgmObj, -1);
        }
        // sucks to be here
	} else {
        // first call
		Mix_FadeOutMusic(800);
		Mix_PlayMusic(bgmObj, -1);
	}
}

/**
 * Set the world's BGM.
 * This will load a sound file to be played while the player is in this world.
 * If no file is found, no music should play.
 */
void World::
setBGM(std::string path)
{
	if (!path.empty())
		bgmObj = Mix_LoadMUS((bgm = path).c_str());
}

/**
 * Sets the desired theme for the world's background.
 * The images chosen for the background layers are selected depending on the
 * world's background type.
 */
void World::
setBackground(WorldBGType bgt)
{
    // load textures with a limit check
	switch ((bgType = bgt)) {
	case WorldBGType::Forest:
		bgTex = new Texturec(bgFiles);
		break;
	case WorldBGType::WoodHouse:
		bgTex = new Texturec(bgFilesIndoors);
		break;
    default:
        UserError("Invalid world background type");
        break;
	}
}

/**
 * Sets the world's style.
 * The world's style will determine what sprites are used for things like\
 * generic structures.
 */
void World::
setStyle(std::string pre)
{
    // get folder prefix
	std::string prefix = pre.empty() ? "assets/style/classic/" : pre;

    for (s : buildPaths)
        sTexLoc.push_back(prefix + s);

    prefix += "bg/";

    for (s : bgPaths[0])
        bgFiles.push_back(prefix + s);
    for (s : bgPaths[1])
        bgFilesIndoors.push_back(prefix + s);
}

/**
 * Pretty self-explanatory.
 */
std::string World::
setToLeft(std::string file)
{
    return (toLeft = file);
}

/**
 * Pretty self-explanatory.
 */
std::string World::
setToRight(std::string file)
{
	return (toRight = file);
}

/**
 * Pretty self-explanatory.
 */
std::string World::
getToLeft(void) const
{
    return toLeft;
}

/**
 * Pretty self-explanatory.
 */
std::string World::
getToRight(void) const
{
    return toRight;
}

/**
 * Attempts to go to the left world, returning either that world or itself.
 */
World *World::
goWorldLeft(Player *p)
{
	World *tmp;

    // check if player is at world edge
	if (!toLeft.empty() && p->loc.x < worldStart + HLINES(15)) {
        // load world (`toLeft` conditional confirms existance)
	    tmp = loadWorldFromPtr(currentWorldToLeft);

        // adjust player location
		p->loc.x = tmp->worldStart + HLINES(20);
		p->loc.y = tmp->worldData[tmp->lineCount - 1].groundHeight;

		return tmp;
	}

	return this;
}

/**
 * Attempts to go to the right world, returning either that world or itself.
 */
World *World::
goWorldRight(Player *p)
{
	World *tmp;

	if (!toRight.empty() && p->loc.x + p->width > -worldStart - HLINES(15)) {
		tmp = loadWorldFromPtr(currentWorldToRight);

		p->loc.x = tmp->worldStart - HLINES(-15.0);
		p->loc.y = GROUND_HEIGHT_MINIMUM;

		return tmp;
	}

	return this;
}

/**
 * Acts like goWorldLeft(), but takes an NPC; returning true on success.
 */
bool World::
goWorldLeft(NPC *e)
{
	// check if entity is at world edge
	if(!toLeft.empty() && e->loc.x < worldStart + HLINES(15)) {
        currentWorldToLeft->addNPC(e->loc.x,e->loc.y);
        e->die();

		currentWorldToLeft->npc.back()->loc.x = 0;
		currentWorldToLeft->npc.back()->loc.y = GROUND_HEIGHT_MAXIMUM;

		return true;
	}

	return false;
}

/**
 * Attempts to enter a building that the player is standing in front of.
 */
std::pair<World *, float> World::
goInsideStructure(Player *p)
{
	World *tmp;

	if (inside.empty()) {
        auto d = std::find_if(std::begin(build), std::end(build), [p](const Structures *s) {
            return ((p->loc.x > s->loc.x) && (p->loc.x + p->width < s->loc.x + s->width));
        });
        auto b = *d;

        if ((d == std::end(build)) || b->inside.empty())
            return std::make_pair(this, 0);

        // +size cuts folder prefix
		inside.push_back(&currentXML[xmlFolder.size()]);
		tmp = loadWorldFromXML(b->inside);

		return std::make_pair(tmp, 0);
	} else {
        std::string current = &currentXML[xmlFolder.size()];
		tmp = loadWorldFromXML(inside.back());
        inside.clear();

        Structures *b = nullptr;
        for (auto &s : tmp->build) {
            if (s->inside == current) {
                b = s;
                break;
            }
        }

        if (b == nullptr)
            return std::make_pair(this, 0);

        return std::make_pair(tmp, b->loc.x + (b->width / 2));
	}

	return std::make_pair(this, 0);
}

void World::
addStructure(BUILD_SUB sub, float x,float y, std::string tex, std::string inside)
{
	build.push_back(new Structures());
	build.back()->inWorld = this;
	build.back()->textureLoc = tex;
	build.back()->spawn(sub, x, y);
	build.back()->inside = inside;
	entity.push_back(build.back());
}

Village *World::
addVillage(std::string name, World *world)
{
    village.emplace_back(name, world);
    return &village.back();
}

/*void World::
addMob(int t, float x, float y)
{
	mob.push_back(new new(auto *&t));
	mob.back()->spawn(x, y);

	entity.push_back(mob.back());
}*/

void World::addMob(Mob *m, vec2 coord)
{
    mob.push_back(m);
    mob.back()->spawn(coord.x, coord.y);

	entity.push_back(mob.back());
}

/*void World::
addMob(int t, float x, float y, void (*hey)(Mob *))
{
	mob.push_back(new Mob(t));
	mob.back()->spawn(x, y);
	mob.back()->hey = hey;

	entity.push_back(mob.back());
}*/

void World::
addNPC(float x, float y)
{
	npc.push_back(new NPC());
	npc.back()->spawn(x, y);

	entity.push_back(npc.back());
}

void World::
addMerchant(float x, float y, bool housed)
{
	merchant.push_back(new Merchant());
	merchant.back()->spawn(x, y);

    if (housed)
        merchant.back()->inside = build.back();

	npc.push_back(merchant.back());
	entity.push_back(npc.back());
}

void World::
addObject(std::string in, std::string p, float x, float y)
{
	object.emplace_back(in, p);
	object.back().spawn(x, y);

	entity.push_back(&object.back());
}

void World::
addParticle(float x, float y, float w, float h, float vx, float vy, Color color, int d)
{
	particles.emplace_back(x, y, w, h, vx, vy, color, d);
	particles.back().canMove = true;
}

void World::
addParticle(float x, float y, float w, float h, float vx, float vy, Color color, int d, unsigned char flags)
{
	particles.emplace_back(x, y, w, h, vx, vy, color, d);
	particles.back().canMove = true;
    particles.back().gravity = flags & (1 << 0);
    particles.back().bounce  = flags & (1 << 1);
}

void World::
addLight(vec2 loc, Color color)
{
	if (light.size() < 64)
        light.push_back(Light(loc, color, 1));
}

void World::
addHole(unsigned int start, unsigned int end)
{
    if (end > worldData.size())
        end = worldData.size();

	for (unsigned int i = start; i < end; i++)
		worldData[i].groundHeight = 0;
}

void World::
addHill(const ivec2 peak, const unsigned int width)
{
	int start  = peak.x - width / 2,
        end    = start + width,
        offset = 0;
	const float thing = peak.y - worldData[start].groundHeight;
    const float period = PI / width;

	if (start < 0) {
        offset = -start;
        start = 0;
    }

	if (end > (signed)worldData.size())
	  end = worldData.size();

	for (int i = start; i < end; i++) {
		worldData[i].groundHeight += thing * sin((i - start + offset) * period);
		if (worldData[i].groundHeight > peak.y)
			worldData[i].groundHeight = peak.y;
	}
}

float getIndoorWorldFloorHeight(void)
{
    return INDOOR_FLOOR_HEIGHTT + INDOOR_FLOOR_THICKNESS;
}

bool isCurrentWorldIndoors(void) {
    return !inside.empty();
}

IndoorWorld::IndoorWorld(void) {
}

IndoorWorld::~IndoorWorld(void) {
	delete bgTex;

	deleteEntities();
}

void IndoorWorld::
addFloor(unsigned int width)
{
    if (floor.empty())
        generate(width);
    floor.emplace_back(width, floor.size() * INDOOR_FLOOR_HEIGHTT + INDOOR_FLOOR_THICKNESS);
    fstart.push_back(0);
}


void IndoorWorld::
addFloor(unsigned int width, unsigned int start)
{
    if (floor.empty())
        generate(width);
    floor.emplace_back(width, floor.size() * INDOOR_FLOOR_HEIGHTT + INDOOR_FLOOR_THICKNESS);
    fstart.push_back(start);
}

bool IndoorWorld::
moveToFloor(Entity *e, unsigned int _floor)
{
    if (_floor > floor.size())
        return false;

    e->loc.y = floor[_floor - 1][0];
    return true;
}

bool IndoorWorld::
isFloorAbove(Entity *e)
{
    for (unsigned int i = 0; i < floor.size(); i++) {
        if (floor[i][0] + INDOOR_FLOOR_HEIGHTT - 100 > e->loc.y)
            return (i + 1) != floor.size();
    }
    return false;
}

bool IndoorWorld::
isFloorBelow(Entity *e)
{
    for (unsigned int i = 0; i < floor.size(); i++) {
        if (floor[i][0] + INDOOR_FLOOR_HEIGHTT - 100 > e->loc.y)
            return i > 0;
    }
    return false;
}

void IndoorWorld::
singleDetect(Entity *e)
{
    unsigned int floornum = 0;
    float start, end;

    if (!e->isAlive())
        return;

    for (; floornum < floor.size(); floornum++) {
        if (floor[floornum][0] + INDOOR_FLOOR_HEIGHTT - 100 > e->loc.y) {
            if (e->loc.y < floor[floornum][0]) {
                e->loc.y = floor[floornum][0];
                e->vel.y = 0;
                e->ground = true;
            }
            break;
        }
    }

    if (e->vel.y > -2)
        e->vel.y -= GRAVITY_CONSTANT * game::time::getDeltaTime();

    if (e->ground) {
        e->loc.y = ceil(e->loc.y);
        e->vel.y = 0;
    }

    start = worldStart + HLINES(fstart[floornum]);
    end = start + HLINES(floor[floornum].size());

    if (e->loc.x < start) {
        e->vel.x = 0;
        e->loc.x = start + game::HLINE / 2;
    } else if (e->loc.x + e->width + game::HLINE > end) {
        e->vel.x = 0;
        e->loc.x = end - e->width - game::HLINE;
    }

}

void IndoorWorld::
draw(Player *p)
{
	unsigned int i,f;
	int x;

    auto SCREEN_WIDTH = game::SCREEN_WIDTH;
    auto SCREEN_HEIGHT = game::SCREEN_HEIGHT;
    auto HLINE = game::HLINE;

    // draw lights
    for (auto &l : light) {
        if (l.belongsTo) {
            l.loc.x = l.following->loc.x + SCREEN_WIDTH / 2;
            l.loc.y = (l.following->loc.y > SCREEN_HEIGHT / 2) ? SCREEN_HEIGHT / 2 : l.following->loc.y;
        }
        if (l.flame) {
            l.fireFlicker = .9 + ((rand() % 2) / 10.0f);
            l.fireLoc.x = l.loc.x + (rand() % 2 - 1) * 3;
            l.fireLoc.y = l.loc.y + (rand() % 2 - 1) * 3;
        } else
            l.fireFlicker = 1.0f;
    }

    std::unique_ptr<GLfloat[]> pointArrayBuf = std::make_unique<GLfloat[]> (2 * (light.size()));
	auto pointArray = pointArrayBuf.get();
    GLfloat flameArray[64];

	for (i = 0; i < light.size(); i++) {
        if (light[i].flame) {
    		pointArray[2 * i    ] = light[i].fireLoc.x - offset.x;
    		pointArray[2 * i + 1] = light[i].fireLoc.y;
        }else{
            pointArray[2 * i    ] = light[i].loc.x - offset.x;
            pointArray[2 * i + 1] = light[i].loc.y;
        }
	}

    for(i = 0; i < light.size(); i++)
        flameArray[i] = light[i].fireFlicker;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "sampler"), 0);
	glUniform1f(glGetUniformLocation(shaderProgram, "amb"), 0.02f + light.size()/50.0f);

	if (light.empty())
		glUniform1i(glGetUniformLocation(shaderProgram, "numLight"), 0);
	else {
		glUniform1i (glGetUniformLocation(shaderProgram, "numLight"), light.size());
		glUniform2fv(glGetUniformLocation(shaderProgram, "lightLocation"), light.size(), pointArray);
		glUniform3f (glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform1fv(glGetUniformLocation(shaderProgram, "fireFlicker"), light.size(), flameArray);
	}

	bgTex->bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //for the s direction
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //for the t direction
	glColor4ub(255,255,255,255);

    glBegin(GL_QUADS);
        glTexCoord2i(0,1);							     glVertex2i(worldStart - SCREEN_WIDTH / 2,0);
		glTexCoord2i((-worldStart*2+SCREEN_WIDTH)/512,1);glVertex2i(-worldStart + SCREEN_WIDTH / 2,0);
		glTexCoord2i((-worldStart*2+SCREEN_WIDTH)/512,0);glVertex2i(-worldStart + SCREEN_WIDTH / 2,SCREEN_HEIGHT);
		glTexCoord2i(0,0);							     glVertex2i(worldStart - SCREEN_WIDTH / 2,SCREEN_HEIGHT);
    glEnd();

    glUseProgram(0);

	/*
	 *	Draw the ground.
	*/

	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "sampler"), 0);
	glBegin(GL_QUADS);
        safeSetColor(150, 100, 50);
        for (f = 0; f < floor.size(); f++) {
            i = 0;
    		for (h : floor[f]) {
    			x = worldStart + fstart[f] * HLINE + HLINES(i);
    			glVertex2i(x        , h            );
    			glVertex2i(x + HLINE, h            );
    			glVertex2i(x + HLINE, h - INDOOR_FLOOR_THICKNESS);
    			glVertex2i(x        , h - INDOOR_FLOOR_THICKNESS);
                i++;
    		}
        }
	glEnd();
	glUseProgram(0);

	/*
	 *	Draw all entities.
	*/
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorIndex);

    glUniform1i(glGetUniformLocation(shaderProgram, "sampler"), 0);
    glUseProgram(shaderProgram);

    std::for_each(particles.begin(), particles.end(), [](Particles &part) { part.draw(); });

    glUseProgram(0);

	/*for (auto &part : particles)
		part.draw();*/

	for (auto &e : entity)
		e->draw();

	p->draw();
}

Arena::Arena(World *leave,Player *p,Mob *m) {
	generate(800);
	addMob(new Door(), vec2 {100, 100});

	inBattle = true;
	mmob = m;
	mmob->aggressive = false;

	mob.push_back(m);
	entity.push_back(mob.back());

	battleNest.push_back(leave);
	battleNestLoc.push_back(p->loc);
}

Arena::~Arena(void) {
	deleteEntities();
}

World *Arena::exitArena(Player *p) {
	World *tmp;
	if (!mmob->isAlive() &&
        p->loc.x + p->width / 2 > mob[0]->loc.x &&
	    p->loc.x + p->width / 2 < mob[0]->loc.x + HLINES(12)) {
	    tmp = battleNest.front();
		battleNest.erase(battleNest.begin());

		inBattle = !battleNest.empty();
		ui::toggleBlackFast();
		ui::waitForCover();

		p->loc = battleNestLoc.back();
		battleNestLoc.pop_back();

		mob.clear();
		mmob->die();

		return tmp;
	}

    return this;
}

std::string getWorldWeatherStr(WorldWeather ww)
{
    switch (ww) {
    case WorldWeather::Sunny:
        return "Sunny";
        break;
    case WorldWeather::Dark:
        return "Darky";
        break;
    case WorldWeather::Rain:
        return "Rainy";
        break;
    default:
        return "Snowy";
        break;
    }
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
    World *tmp = ptr;

    loadedLeft = true;
    currentWorldToLeft = loadWorldFromXML(tmp->getToLeft());
    loadedLeft = false;

    loadedRight = true;
    currentWorldToRight = loadWorldFromXML(tmp->getToRight());
    loadedRight = false;

    return tmp;
}

/**
 * Loads a world from the given XML file.
 */
World *
loadWorldFromXMLNoSave(std::string path) {
	XMLDocument xml;
	XMLElement *wxml;
	XMLElement *vil;

	World *tmp;
	float spawnx, randx;
	bool dialog,Indoor;
    unsigned int flooor;

	const char *ptr;
	std::string name, sptr;

    // no file? -> no world
    if (path.empty())
        return NULL;

    currentXML = std::string(xmlFolder + path);
	xml.LoadFile(currentXML.c_str());

    // attempt to load a <World> tag
	if ((wxml = xml.FirstChildElement("World"))) {
		wxml = wxml->FirstChildElement();
		vil = xml.FirstChildElement("World")->FirstChildElement("village");
		tmp = new World();
        Indoor = false;
	}

    // attempt to load an <IndoorWorld> tag
    else if ((wxml = xml.FirstChildElement("IndoorWorld"))) {
		wxml = wxml->FirstChildElement();
		vil = NULL;
		tmp = new IndoorWorld();
        Indoor = true;
	}

    // error: can't load a world...
    else
        UserError("XML Error: Cannot find a <World> or <IndoorWorld> tag in " + currentXML + "!");

    // iterate through world tags
	while (wxml) {
		name = wxml->Name();

        // world linkage
		if (name == "link") {

            // links world to the left
			if ((ptr = wxml->Attribute("left"))) {
				tmp->setToLeft(ptr);

                // load the left world if it isn't
                if (!loadedLeft) {
                    loadedLeft = true;
                    currentWorldToLeft = loadWorldFromXMLNoSave(ptr);
                    loadedLeft = false;
                }
			}

            // links world to the right
            else if ((ptr = wxml->Attribute("right"))) {
				tmp->setToRight(ptr);

                // load the right world if it isn't
                if (!loadedRight) {
                    loadedRight = true;
                    currentWorldToRight = loadWorldFromXMLNoSave(ptr);
                    loadedRight = false;
                }
			}

			// tells what world is outside, if in a structure
			else if (Indoor && (ptr = wxml->Attribute("outside")))
				inside.push_back(ptr);

            // error, invalid link tag
            else
                UserError("XML Error: Invalid <link> tag in " + currentXML + "!");

		}

        // style tags
        else if (name == "style") {
            // set style folder
			tmp->setStyle(wxml->StrAttribute("folder"));

            // set background folder
            if (wxml->QueryUnsignedAttribute("background", &flooor) != XML_NO_ERROR)
                UserError("XML Error: No background given in <style> in " + currentXML + "!");
			tmp->setBackground((WorldBGType)flooor);

            // set BGM file
            tmp->setBGM(wxml->StrAttribute("bgm"));
		}

        // world generation (for outdoor areas)
        else if (name == "generation") {
            // random gen.
			if (!Indoor && wxml->StrAttribute("type") == "Random")
				tmp->generate(wxml->UnsignedAttribute("width"));
            else {
                if (Indoor)
                    UserError("XML Error: <generation> tags can't be in <IndoorWorld> tags (in " + currentXML + ")!");
                else
                    UserError("XML Error: Invalid <generation> tag in " + currentXML + "!");
            }
		}

        // mob creation
         else if (name == "rabbit") {
             tmp->addMob(new Rabbit(), vec2 {0, 0});
             tmp->getLastMob()->createFromXML(wxml);
         } else if (name == "bird") {
             tmp->addMob(new Bird(), vec2 {0, 0});
             tmp->getLastMob()->createFromXML(wxml);
         } else if (name == "trigger") {
             tmp->addMob(new Trigger(), vec2 {0, 0});
             tmp->getLastMob()->createFromXML(wxml);
         } else if (name == "door") {
             tmp->addMob(new Door(), vec2 {0, 0});
             tmp->getLastMob()->createFromXML(wxml);
         } else if (name == "page") {
             tmp->addMob(new Page(), vec2 {0, 0});
             tmp->getLastMob()->createFromXML(wxml);
         }

        // npc creation
        else if (name == "npc") {
			const char *npcname;

            // spawn at coordinates if desired
			if (wxml->QueryFloatAttribute("x", &spawnx) == XML_NO_ERROR)
				tmp->addNPC(spawnx, 100);
			else
				tmp->addNPC(0, 100);

            // name override
			if ((npcname = wxml->Attribute("name"))) {
                delete[] tmp->npc.back()->name;
				tmp->npc.back()->name = new char[strlen(npcname) + 1];
				strcpy(tmp->npc.back()->name, npcname);
			}

            // dialog enabling
			dialog = false;
			if (wxml->QueryBoolAttribute("hasDialog", &dialog) == XML_NO_ERROR && dialog) {
				tmp->npc.back()->addAIFunc(false);
			} else
                tmp->npc.back()->dialogIndex = 9999;

            if (Indoor && wxml->QueryUnsignedAttribute("floor", &flooor) == XML_NO_ERROR)
                Indoorp(tmp)->moveToFloor(tmp->npc.back(), flooor);

            // custom health value
            if (wxml->QueryFloatAttribute("health", &spawnx) == XML_NO_ERROR)
                tmp->npc.back()->health = tmp->npc.back()->maxHealth = spawnx;
		}

        // structure creation
        else if (name == "structure") {
			tmp->addStructure((BUILD_SUB) wxml->UnsignedAttribute("type"),
							   wxml->QueryFloatAttribute("x", &spawnx) != XML_NO_ERROR ?
							       getRand() % tmp->getTheWidth() / 2.0f : spawnx,
							   100,
							   wxml->StrAttribute("texture"),
							   wxml->StrAttribute("inside")
               );
		} else if (name == "hill") {
			tmp->addHill(ivec2 { wxml->IntAttribute("peakx"), wxml->IntAttribute("peaky") }, wxml->UnsignedAttribute("width"));
		} else if (name == "time") {
            game::time::setTickCount(std::stoi(wxml->GetText()));
        } else if (Indoor && name == "floor") {
            if (wxml->QueryFloatAttribute("start",&spawnx) == XML_NO_ERROR)
                Indoorp(tmp)->addFloor(wxml->UnsignedAttribute("width"), spawnx);
            else
                Indoorp(tmp)->addFloor(wxml->UnsignedAttribute("width"));
        }

        spawnx = 0;
		wxml = wxml->NextSiblingElement();
	}

	Village *vptr;

	if (vil) {
		vptr = tmp->addVillage(vil->Attribute("name"), tmp);
		vil = vil->FirstChildElement();
	}

	while(vil) {
		name = vil->Name();
		randx = 0;

		/**
		 * 	READS DATA ABOUT STRUCTURE CONTAINED IN VILLAGE
		 */

		if (name == "structure") {
			tmp->addStructure((BUILD_SUB)vil->UnsignedAttribute("type"),
							   vil->QueryFloatAttribute("x", &spawnx) != XML_NO_ERROR ? randx : spawnx,
							   100,
							   vil->StrAttribute("texture"),
							   vil->StrAttribute("inside"));
		} else if (name == "stall") {
            sptr = vil->StrAttribute("type");

            // handle markets
            if (sptr == "market") {

                // create a structure and a merchant, and pair them
                tmp->addStructure(STALL_MARKET,
                                   vil->QueryFloatAttribute("x", &spawnx) != XML_NO_ERROR ? randx : spawnx,
                                   100,
							       vil->StrAttribute("texture"),
							       vil->StrAttribute("inside")
                   );
				tmp->addMerchant(0, 100, true);
            }

            // handle traders
            else if (sptr == "trader") {
				tmp->addStructure(STALL_TRADER,
							       vil->QueryFloatAttribute("x", &spawnx) != XML_NO_ERROR ? randx : spawnx,
							       100,
							       vil->StrAttribute("texture"),
							       vil->StrAttribute("inside")
                   );
			}

            // loop through buy/sell/trade tags
            XMLElement *sxml = vil->FirstChildElement();
            std::string tag;
            while (sxml) {
                tag = sxml->Name();

                if (tag == "buy") { //converts price to the currencies determined in items.xml
                    // TODO
                } else if (tag == "sell") { //converts price so the player can sell
                    // TODO
                } else if (tag == "trade") { //doesn't have to convert anything, we just trade multiple items
                	tmp->merchant.back()->trade.push_back(Trade(sxml->IntAttribute("quantity"),
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
                            tmp->merchant.back()->text[0] = std::string(buf, strlen(buf));
                            tmp->merchant.back()->toSay = &tmp->merchant.back()->text[0];
                        } else if (textOption == "accept") { //when he accepts the trade
                            tmp->merchant.back()->text[1] = std::string(buf, strlen(buf));
                        } else if (textOption == "deny") { //when you don't have enough money
                            tmp->merchant.back()->text[2] = std::string(buf, strlen(buf));
                        } else if (textOption == "leave") { //when you leave the merchant
                            tmp->merchant.back()->text[3] = std::string(buf, strlen(buf));
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

	std::ifstream dat (((std::string)currentXML + ".dat").data());
	if (dat.good()) {
		dat.close();
		tmp->load();
	}

	return tmp;
}

Village::Village(std::string meme, World *w)
{
	name = meme;
	start.x = w->getTheWidth() / 2.0f;
	end.x = -start.x;
	in = false;
}
