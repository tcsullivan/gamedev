/** @file world.h
 * @brief The world system.
 *
 * This file contains the classes and variables necessary to create an in-game
 * world.
 */

#ifndef WORLD_H
#define WORLD_H

#include <common.h>
#include <entities.h>

#define GROUND_HEIGHT_INITIAL   80
#define GROUND_HEIGHT_MINIMUM   60
#define GROUND_HEIGHT_MAXIMUM   110

#define GROUND_HILLINESS        10

/**
 * Defines how many game ticks it takes for a day to elapse.
 */

#define DAY_CYCLE 12000

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
	Rain		/**< Rain (to be implemented)*/
};

/**
 * The light structure, used to store light coordinates and color.
 */

typedef struct {
	vec2 loc;		/**< Light location */
	Color color;	/**< Light color */
} Light;

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

/**
 * A value used by World::draw() for shading, ranges from -50 to 50 depending
 * on the current time of day.
 */

extern int worldShade;

/**
 * The path to the currently loaded XML file.
 */

extern std::string currentXML;

// prototype so Village can reference it
class World;

/**
 * The village class, used to group structures into villages.
 */

class Village {
public:
	std::string name;
	vec2 start;
	vec2 end;
	bool in;
	std::vector<Structures *> build;

	Village(const char *meme, World *w);
	~Village(void){}
};

/**
 * The world class. This class does everything a world should do.
 */

class World {
protected:

	/**
	 * The line array.
	 *
	 * This array is created through 'new' in World::generate(), with an amount
	 * of elements provided by the function.
	 */

	std::vector<WorldData> worldData;

	/**
	 * Starting x coordinate.
	 *
	 * This x value is the point at which line[0] should reside, can be used to
	 * calculate the width of the world.
	 */

	int worldStart;

	/**
	 * Handle physics for a single entity.
	 *
	 * This function handles gravity and death for an entity. The public version
	 * of this, World::detect(), handles all entities in the world as well as
	 * the player. World::singleDetect() should never be used outside of
	 * World::detect(), which is why it is declared private.
	 */

	void singleDetect( Entity *e );

	/**
	 * Empties all entity vectors.
	 *
	 * Each entity vector is iterated through, calling delete for each entry.
	 * Once all specific vectors are cleared, the general entity vector is
	 * emptied of the pointers to those other vectors. This function should only
	 * be called in World's destructor, as there shouldn't be another reason to
	 * call this function.
	 */

	void deleteEntities( void );

	/**
	 * Number of lines in the world.
	 *
	 * While this number is helpful for knowing the world's width, it is kept
	 * private for security reasons. To compensate for this,
	 * World::getTheWidth() is provided (see below).
	 */

	unsigned int lineCount;

	/**
	 * An array of star coordinates.
	 */

	std::vector<vec2> star;

	/**
	 * The Texturec object that holds the background sprites for this world.
	 */

	Texturec *bgTex;

	/**
	 * Defines the set of background images that should be used for this world.
	 */

	WorldBGType bgType;

	/**
	 * The Mix_Music object that holds the background soundtrack for the world.
	 */

	Mix_Music *bgmObj;

	/**
	 * The file path of the song wished to be loaded by bgmObj.
	 */

	std::string bgm;

	std::vector<std::string> bgFiles;
	std::vector<std::string> bgFilesIndoors;

public:

	/**
	 * The filename of the XML file for the world to the left; NULL if no world
	 * is present.
	 */

	std::string toLeft;

	/**
	 * The filename of the XML file for the world to the right; NULL if no world
	 * is present.
	 */

	std::string toRight;

	/**
	 * Sets what XML file to use for loading the world to the left.
	 */

	std::string setToLeft( std::string file );

	/**
	 * Sets what XML file to use for loading the world to the right.
	 */

	std::string setToRight( std::string file );

	/**
	 * A vector of pointers to every NPC, Structure, Mob, and Object in this
	 * world.
	 */

	std::vector<Entity		*>	entity;

	/**
	 * A vector of all NPCs in this world.
	 */

	std::vector<NPC			*>	npc;
	std::vector<Merchant    *>  merchant;

	/**
	 * A vector of all Structures in this world.
	 */

	std::vector<Structures	*>	build;

	/**
	 * A vector of all Mobs in this world.
	 */

	std::vector<Mob			*>	mob;

	/**
	 * A vector of all Objects in this world.
	 */

	std::vector<Object		*>	object;

	/**
	 * A vector of all particles in this world.
	 */

	std::vector<Particles> particles;


	std::vector<Village 	*>	village;

	/**
	 * A vector of all light elements in this world.
	 */

	std::vector<Light>  light;

	/**
	 * Vector of all building textures for the current world style
	 */

	std::vector<std::string> sTexLoc;

	/**
	 * NULLifies pointers and allocates necessary memory. This should be
	 * followed by some combination of setBackground(), setBGM(), or
	 * generate().
	 */

	World( void );

	/**
	 * Frees resources taken by the world.
	 */

	virtual ~World(void);

	/**
	 * Adds a structure to the world, with the specified subtype and
	 * coordinates. `inside` is a file name for the IndoorWorld XML file that
	 * this structure will lead to; if NULL the player won't be able to enter
	 * the structure.
	 */

	void addStructure(BUILD_SUB subtype,float x,float y, std::string tex, std::string inside);

	/**
	 * Adds a Mob to the world with the specified type and coordinates.
	 */

	void addMob(int type,float x,float y);

	/**
	 * Adds a Mob to the world with a handler function that can be called by
	 * certain mobs to trigger events.
	 */

	void addMob(int t,float x,float y,void (*hey)(Mob *));

	/**
	 * Adds an NPC to the world with the specified coordinates.
	 */

	void addNPC(float x,float y);

	/**
	 * Adds a Merchant to the world at the specified coordinates.
	 */

	void addMerchant(float x, float y);

	/**
	 * Adds an object to the world with the specified item id and coordinates.
	 * If `pickupDialog` is not NULL, that string will display in a dialog box
	 * upon object interaction.
	 */

	void addObject( std::string in, std::string pickupDialog, float x, float y);

	/**
	 * Adds a particle to the world with the specified coordinates, dimensions,
	 * velocity, color and duration (time to live).
	 */

	void addParticle(float x, float y, float w, float h, float vx, float vy, Color color, int duration);

	/**
	 * Adds a light to the world with the specified coordinates and color.
	 */

	void addLight(vec2 xy, Color color);

	/**
	 * Updates the coordinates of everything in the world that has coordinates
	 * and a velocity. The provided delta time is used for smoother updating.
	 */

	void update( Player *p, unsigned int delta );

	/**
	 * Generate a world of the provided width. Worlds are drawn centered on the
	 * y-axis, so the reachable coordinates on the world would be from negative
	 * half-width to positive half-width.
	 */

	virtual void generate(unsigned int width);

	/**
	 * Sets the background theme, collecting the required textures into a
	 * Texturec object.
	 */

	void setBackground(WorldBGType bgt);

	/**
	 * Sets the background music for the world, required for the world to be
	 * playable.
	 */

	void setBGM(std::string path);

	/**
	 *	Sets the worlds style folder
	 */

	void setStyle(std::string pre);

	/**
	 * Plays/stops this world's BGM. If `prev` is not NULL, that world's BGM
	 * will be faded out followed by the fading in of this world's BGM.
	 */

	void bgmPlay(World *prev) const;

	/**
	 * Draw the world and entities based on the player's coordinates.
	 */

	virtual void draw(Player *p);

	/**
	 * Handles collision between the entities and the world, as well as entity
	 * death.
	 */

	void detect(Player *p);

	/**
	 * Attempts to let the player enter the left-linked world specified by
	 * `toLeft`. Returns the world to the left if the movement is possible,
	 * otherwise returns this world.
	 */

	World *goWorldLeft(Player *p);

	/**
	 * Attempts to let the player enter the right-linked world specified by
	 * `toRight`. Returns the world to the right if the movement is possible,
	 * otherwise returns this world.
	 */

	World *goWorldRight(Player *p);

	/**
	 * This function looks for any structure the player is standing in front of
	 * that also have an inside world. Returns the inside world if those
	 * conditions are met, otherwise returns this world.
	 */

	World *goInsideStructure(Player *p);

	/**
	 * Adds a hole between the specified y coordinates. If the player falls in
	 * this hole the game will exit.
	 */

	void addHole(unsigned int start,unsigned int end);

	/**
	 * Adds a hill to the world, given the peak's coordinates and how wide the
	 * hill can be.
	 */

	void addHill( ivec2 peak, unsigned int width );

	/**
	 * Gets the world's width.
	 */

	int getTheWidth(void) const;

	void save(void);
	void load(void);
};

/*
 *	IndoorWorld - Indoor settings stored in a World class
 */

class IndoorWorld : public World {
public:
	IndoorWorld(void);
	~IndoorWorld(void);

	void generate(unsigned int width);	// Generates a flat world of width 'width'
	void draw(Player *p);				// Draws the world (ignores layers)
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

	Arena( World *leave, Player *p, Mob *m );

	/**
	 * Frees resources taken by the arena.
	 */

	~Arena( void );

	/**
	 * Attempts to exit the world, returning the player to the world they were
	 * last in.
	 */

	World *exitArena( Player *p );
};

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

World *loadWorldFromPtr( World *ptr );

#endif // WORLD_H
