/** @file world.h
 * @brief The world system.
 * 
 * This file contains the classes and variables necessary to create an in-game
 * world.
 */

#ifndef WORLD_H
#define WORLD_H

#include <ostream>
#include <istream>

#include <common.h>
#include <entities.h>

/**
 * Defines at what interval y values should be calculated for the array 'line'.
 */

#define GEN_INC 10


#define GEN_MIN  80
#define GEN_MAX  110


/**
 * Defines how many game ticks it takes for a day to elapse.
 */

#define DAY_CYCLE 12000

/**
 * The background type enum.
 * This enum contains all different possibilities for world backgrounds; used
 * in World::setBackground() to select the appropriate images.
 */

typedef enum {
	BG_FOREST,		/**< A forest theme. */
	BG_WOODHOUSE	/**< An indoor wooden house theme. */
} WORLD_BG_TYPE;

/**
 * The weather type enum.
 * This enum contains every type of weather currently implemented in the game.
 * Weather is set by the world somewhere.
 */

typedef enum {
	SUNNY = 0,	/**< Sunny/daytime */
	DARK,		/**< Nighttime */
	RAIN		/**< Rain (not implemented :) )*/
} WEATHER;


typedef struct{
	vec2 loc;
	Color color;
}Light;
/**
 * The line structure.
 * This structure is used to store the world's ground, stored in vertical
 * lines. Dirt color and grass properties are also kept track of here.
 */

struct line_t {
	float y;				/**< Height of this vertical line */
	bool gs;				/**< Show grass */
	float gh[2];			/**< Height of glass (2 blades per line) */
	unsigned char color;	/**< Lightness of dirt (brown) */
} __attribute__ ((packed));

/**
 * The world class. This class does everything a world should do.
 */

class World {
protected:
	
	/**
	 * The line array.
	 * This array is created through 'new' in World::generate(), with an amount
	 * of elements provided by the function.
	 */
	 
	struct line_t *line;
	
	/**
	 * Starting x coordinate.
	 * This x value is the point at which line[0] should reside, can be used to
	 * calculate the width of the world.
	 */
	
	int x_start;
	
	/**
	 * Handle physics for a single entity.
	 * This function handles gravity and death for an entity. The public version
	 * of this, World::detect(), handles all entities in the world as well as
	 * the player. World::singleDetect() should never be used outside of
	 * World::detect(), which is why it is declared private.
	 */
	
	void singleDetect(Entity *e);
	
	/**
	 * Empties all entity vectors.
	 * Each entity vector is iterated through, calling delete for each entry.
	 * Once all specific vectors are cleared, the general entity vector is
	 * emptied of the pointers to those other vectors. This function should only
	 * be called in World's destructor, as there shouldn't be another reason to
	 * call this function.
	 */
	
	void deleteEntities(void);
	
	/**
	 * Number of lines in the world.
	 * While this number is helpful for knowing the world's width, it is kept
	 * private for security reasons. To compensate for this,
	 * World::getTheWidth() is provided (see below).
	 */
	
	unsigned int lineCount;
	
	/**
	 * An array of star coordinates.
	 */
	
	vec2 *star;
	
	/**
	 * The Texturec object that holds the background sprites for this world.
	 */
	
	Texturec *bgTex;
	WORLD_BG_TYPE bgType;
	
	/**
	 * The Mix_Music object that holds the background soundtrack for the world.
	 */
	
	Mix_Music *bgmObj;
	
	/**
	 * The file path of the song wished to be loaded by bgmObj.
	 */
	
	char *bgm;
	
public:

	/**
	 * These pointers keep track of worlds that are adjacent to this one. Used in
	 * ui.cpp for world jumping.
	 */

	World **toLeft,
		  **toRight,
		  *behind,
		  *infront;
	
	/*
	 * These vectors contain the NPCs, Mobs, Structures and Objects that are
	 * loaded inside the world, with the Entity vector containing pointers to
	 * the contents of all the others.
	 */

	std::vector<NPC			*>	npc;
	std::vector<Structures	*>	build;
	std::vector<Mob			*>	mob;
	std::vector<Entity		*>	entity;
	std::vector<Object		*>	object;
	std::vector<Particles	*>	particles;
	std::vector<Light        >  light;
	
	void addStructure(BUILD_SUB sub,float x,float y,World **inside);//,World **outside);
	void addVillage(int bCount, int npcMin, int npcMax,World **inside);
	void addMob(int t,float x,float y);
	void addMob(int t,float x,float y,void (*hey)(Mob *));
	void addNPC(float x,float y);
	void addObject(ITEM_ID, bool, const char *, float, float);
	void addParticle(float, float, float, float, float, float, Color color, int);
	void addLight(vec2, Color);

	NPC *getAvailableNPC(void);
	
	/*
	 *	Update coordinates of all entities.
	 */
	
	void update(Player *p,unsigned int delta);
	
	/*
	 *	Constructor and deconstructor, these do what you would expect.
	*/
						
	World(void);
	virtual ~World(void);				// Frees the 'line' array.
	
	/*
	 *	Generate a world of width `width`. This function is virtual so that other world
	 *	classes that are based on this one can generate themselves their own way.
	*/
	
	virtual void generate(unsigned int width);
	void generateFunc(unsigned int width,float(*func)(float));
	
	/*
	 *	Adds images to using for the background.
	*/
	
	void setBackground(WORLD_BG_TYPE bgt);
	
	/*
	 *	Start/stop background music. 
	*/
	
	void setBGM(const char *path);
	void bgmPlay(World *prev);
	
	/*
	 *	Looks for the furthest back layer in this world and adds a new layer of width `width` behind it.
	*/
	
	void addLayer(unsigned int width);
	
	/*
	 *	Draw the world and entities based on the player's coordinates. Virtual for the same
	 *	reason generate() is.
	*/
														
	virtual void draw(Player *p);
	
	
	/*
	 *	Detect the player and any entities in the current world.
	*/
	
	void detect(Player *p);
	
	/*
	 *	These functions return the pointer to the world in the direction that is requested if it
	 *	exists and the player is in a condition that it can make the switch, otherwise they
	 *	return the current world.
	*/
	
	World *goWorldLeft(Player *p);
	World *goWorldRight(Player *p);					
	World *goWorldBack(Player *p);
	World *goWorldFront(Player *p);

	bool isWorldLeft(void);
	bool isWorldRight(void);
	
	/*
	 *	Called to enter/exit a structure.
	*/
	
	World *goInsideStructure(Player *p);
	
	/*
	 *	These functions add features to the world.
	*/
	
	void addHole(unsigned int start,unsigned int end);
	
	/*
	 *	Get's the world's width.
	*/
	
	int getTheWidth(void);
	
	void save(std::ofstream *);
	void load(std::ifstream *);
};

/*
 *	Gets a good base y value for background rendering.
*/

float worldGetYBase(World *w);

/*
 *	IndoorWorld - Indoor settings stored in a World class ;)
 */
 
class IndoorWorld : public World {
public:
	World **outside;
	IndoorWorld(void);
	~IndoorWorld(void);
	
	void generate(unsigned int width);	// Generates a flat world of width 'width'
	void draw(Player *p);				// Draws the world (ignores layers)
};

class Arena : public World {
private:
	vec2	 pxy;
	World	*exit;
public:
	Arena(World *leave,Player *p);
	~Arena(void);
	World *exitArena(Player *p);
};

extern int worldShade;

#endif // WORLD_H
