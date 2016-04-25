/* ----------------------------------------------------------------------------
** The world stuffs.
**
** This file contains the classes and variables necessary to create an in-game
** world... "and stuffs".
** --------------------------------------------------------------------------*/
#ifndef WORLD_H
#define WORLD_H

/* ----------------------------------------------------------------------------
** Includes section
** --------------------------------------------------------------------------*/

// local game includes
#include <common.hpp>
#include <entities.hpp>

/* ----------------------------------------------------------------------------
** Structures section
** --------------------------------------------------------------------------*/

/**
 * The background type enum.
 * This enum contains all different possibilities for world backgrounds; used
 * in World::setBackground() to select the appropriate images.
 */
enum class WorldBGType : unsigned char {
	Forest,		/**< A forest theme. */
	WoodHouse	/**< An indoor wooden house theme. */
};

/**
 * The weather type enum.
 * This enum contains every type of weather currently implemented in the game.
 * Weather is set by the world somewhere.
 */
enum class WorldWeather : unsigned char {
	Sunny = 0,	/**< Sunny/daytime */
	Dark,		/**< Nighttime */
	Rain,		/**< Rain */
	Snowy		/**< Snow */
};

/**
 * The line structure.
 * This structure is used to store the world's ground, stored in vertical
 * lines. Dirt color and grass properties are also kept track of here.
 */
typedef struct {
    bool          grassUnpressed;
    float         grassHeight[2];
    float         groundHeight;
    unsigned char groundColor;
} WorldData;

/* ----------------------------------------------------------------------------
** Variables section
** --------------------------------------------------------------------------*/

// affects brightness of world elements when drawn
extern int worldShade;

// the path to the currently loaded XML file.
extern std::string currentXML;

// defines how many game ticks it takes for a day to elapse
extern const unsigned int DAY_CYCLE;

// velocity of player when moved by user
extern const float PLAYER_SPEED_CONSTANT;

// maximum pull of gravity in one game tick
extern const float GRAVITY_CONSTANT;

/* ----------------------------------------------------------------------------
** Classes / function prototypes section
** --------------------------------------------------------------------------*/

/**
 * The village class, used to group structures into villages.
 */
class Village {
public:
	std::string name;
	vec2 start, end;
	bool in;

	Village(std::string meme, World *w);
	~Village(void){}
};

/**
 * The world class. This class does everything a world should do.
 */
class World {
protected:

	// an array of all the world's ground data
	std::vector<WorldData> worldData;

	// the size of `worldData`
	unsigned int lineCount;

	// the left-most (negative) coordinate of the worldStart
	int worldStart;

	// holds / handles textures for background elements
	Texturec *bgTex;

	// defines what type of background is being used
	WorldBGType bgType;

	// an SDL_mixer object for the world's BGM
	Mix_Music *bgmObj;

	// the pathname of the loaded BGM
	std::string bgm;

	// XML file names of worlds to the left and right, empty if nonexistant
	std::string toLeft;
	std::string toRight;

	// structure texture file paths
	std::vector<std::string> sTexLoc;

	// TODO
	std::vector<std::string> bgFiles;
	std::vector<std::string> bgFilesIndoors;

	// an array of star coordinates
	std::vector<vec2> star;

	// entity vectors
	std::vector<Light>        light;
	std::vector<Mob *>        mob;
	std::vector<Object>	      object;
	std::vector<Particles>    particles;
	std::vector<Structures *> build;
	std::vector<Village>      village;

	// handles death, gravity, etc. for a single entity
	virtual void singleDetect(Entity *e);

	// frees entities and clears vectors that contain them
	void deleteEntities(void);

public:

	// entity vectors that need to be public because we're based
	std::vector<Entity *> entity;

	std::vector<NPC	*>      npc;
	std::vector<Merchant *> merchant;

	// the world constructor, prepares variables
	World(void);

	// destructor, frees used memory
	virtual ~World(void);

	// generates a world of the specified width
	void generate(unsigned int width);

	// draws everything to the screen
	virtual void draw(Player *p);

	// handles collisions/death of player and all entities
	void detect(Player *p);

	// updates entities, moving them and such
	void update(Player *p, unsigned int delta);

	// gets the world's width in TODO
	int getTheWidth(void) const;

	// gets a pointer to the most recently added light
	Light *getLastLight(void);

	// gets a pointer to the most recently added mob
	Mob *getLastMob(void);

	// gets the nearest interactable entity to the given one
	Entity *getNearInteractable(Entity &e);

	// gets the coordinates of the `index`th structure
	vec2 getStructurePos(int index);

	// gets the texture path of the `index`th structure
	std::string getSTextureLocation(unsigned int index) const;

	// saves the world's data to an XML file
	void save(void);

	// attempts to load world data from an XML file
	void load(void);

	// plays/pauses the world's music, according to if a new world is being entered
	void bgmPlay(World *prev) const;

	// sets and loads the specified BGM
	void setBGM(std::string path);

	// sets the world's background theme
	void setBackground(WorldBGType bgt);

	// sets the folder to collect entity textures from
	void setStyle(std::string pre);

	// sets / gets pathnames of XML files for worlds to the left and right
	std::string setToLeft(std::string file);
	std::string setToRight(std::string file);
	std::string getToLeft(void) const;
	std::string getToRight(void) const;

	// attempts to enter the left/right adjacent world, returning either that world or this
	World *goWorldLeft(Player *p);
	World *goWorldRight(Player *p);

	// attempts to move an NPC to the left adjacent world, returning true on success
	bool goWorldLeft(NPC *e);

	// attempts to enter a structure that the player would be standing in front of
	std::pair<World *, float> goInsideStructure(Player *p);

	// adds a hole at the specified start and end x-coordinates
	void addHole(unsigned int start,unsigned int end);

	// adds a hill that peaks at the given coordinate and is `width` HLINEs wide
	void addHill(ivec2 peak, unsigned int width);

	// functions to add entities to the world
	void addLight(vec2 xy, Color color);

	void addMerchant(float x, float y, bool housed);

	void addMob(int type, float x, float y);
	void addMob(int type, float x, float y, void (*hey)(Mob *));

	void addNPC(float x, float y);

	void addObject(std::string in, std::string pickupDialog, float x, float y);

	void addParticle(float x, float y, float w, float h, float vx, float vy, Color color, int dur);
	void addParticle(float x, float y, float w, float h, float vx, float vy, Color color, int dur, unsigned char flags);

	void addStructure(BUILD_SUB subtype, float x, float y, std::string tex, std::string inside);

	Village *addVillage(std::string name, World *world);
};

/**
 * IndoorWorld - Indoor settings stored in a World class
 */
class IndoorWorld : public World {
private:

	// like lines, but split into floors
	std::vector<std::vector<float>> floor;

	// the x coordinate to start each floor at
	std::vector<float> fstart;

	// handles physics for a single entity
	void singleDetect(Entity *e);

public:

	// creates an IndoorWorld object
	IndoorWorld(void);

	// frees memory used by this object
	~IndoorWorld(void);

	// adds a floor of the desired width
	void addFloor(unsigned int width);

	// adds a floor at the desired x coordinate with the given width
	void addFloor(unsigned int width, unsigned int start);

	// attempts to move the entity provided to the given floor
	bool moveToFloor(Entity *e, unsigned int _floor);

	// checks for a floor above the given entity
	bool isFloorAbove(Entity *e);

	// checks for a floor below the given entity
	bool isFloorBelow(Entity *e);

	// draws the world about the player
	void draw(Player *p);
};

/**
 * The arena class - creates an arena.
 *
 * This world, when created, expects a pointer to a Mob. This mob will be
 * transported to a temporary world with the player, and the Mob will be
 * killed upon exiting the arena.
 */

class Arena : public World {
private:

	/**
	 * The mob that the player is fighting.
	 */

	Mob *mmob;

public:

	/**
	 * Creates a world with the player and mob, returning the player to the
	 * world `leave` upon exit.
	 */

	Arena(World *leave, Player *p, Mob *m);

	/**
	 * Frees resources taken by the arena.
	 */

	~Arena(void);

	/**
	 * Attempts to exit the world, returning the player to the world they were
	 * last in.
	 */

	World *exitArena(Player *p);
};

bool  isCurrentWorldIndoors(void);
float getIndoorWorldFloorHeight(void);

std::string getWorldWeatherStr(WorldWeather ww);

/**
 * Loads the player into the world created by the given XML file. If a world is
 * already loaded it will be saved before the transition is made.
 */

World *loadWorldFromXML(std::string path);

/**
 * Loads the player into the XML-scripted world, but does not save data from the
 * previous world if one was loaded.
 */

World *loadWorldFromXMLNoSave(std::string path);

World *loadWorldFromPtr(World *ptr);

constexpr IndoorWorld *Indoorp(World *w)
{
    return (IndoorWorld *)w;
}

#endif // WORLD_H
