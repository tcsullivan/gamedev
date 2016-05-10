/* ----------------------------------------------------------------------------
** The entity stuffs.
**
** Entities.
** --------------------------------------------------------------------------*/
#ifndef ENTITIES_H
#define ENTITIES_H
#define DEBUG

/* ----------------------------------------------------------------------------
** Includes section
** --------------------------------------------------------------------------*/

// local game includes
#include <common.hpp>
#include <quest.hpp>
#include <inventory.hpp>
#include <texture.hpp>

/* ----------------------------------------------------------------------------
** Structures section
** --------------------------------------------------------------------------*/

/**
 * An entity type enumerator for identifying entities.
 */
enum _TYPE {
	UNKNOWNT = -999, /**< who knows? */
	OBJECTT = -2,	 /**< an object (Object) */
	STRUCTURET,		 /**< a structure (Structures *) */
	PLAYERT,		 /**< the player (Player *) */
	NPCT,			 /**< an NPC (NPC *) */
	MERCHT,			 /**< a merchant (Merchant *) */
	MOBT			 /**< A mob (Mob *) */
};

/**
 * An enumerator for entity gender.
 */
enum GENDER{
	MALE,	/**< male */
	FEMALE  /**< female */
};

/**
 * An enumerator for strcture types.
 * The subtype of a structure will affect how it is drawn and how it functions.
 */
enum BUILD_SUB{
	TOWN_HALL = 0,		/**< a town hall */
	HOUSE,				/**< a generic house */
	HOUSE2,				/**< a generic house of a different style */
	HOUSE3,				/**< a generic house of a different style */
	HOUSE4,				/**< a generic house of a different style */
	FOUNTAIN,			/**< a fountain, creates water particles */
	LAMP_POST,			/**< a lamppost, creates light */
	FIRE_PIT,			/**< a firepit, creates fire particles / light */
	STALL_MARKET = 70,	/**< a stall for a merchant */
	STALL_TRADER
};

/**
 * A structure for tracking potential trades between the player and a merchant.
 */
struct Trade {
	// the names of the items up for trade
	std::string item[2];
	// how much of each item to trade
	int quantity[2];

	// constructs a trade with the given values
	Trade(int qo, std::string o, int qt, std::string t) {
		item[0] = o;
		item[1] = t;
		quantity[0] = qo;
		quantity[1] = qt;
	}

	// creates an empty trade item
	Trade(void) {
		item[0] = "";
		item[1] = "";
		quantity[0] = 0;
		quantity[1] = 0;
	}
};
typedef struct Trade Trade;

/* ----------------------------------------------------------------------------
** Variables section
** --------------------------------------------------------------------------*/

// the size of the player's inventory
extern const unsigned int PLAYER_INV_SIZE;
// the size of an NPC's inventory
extern const unsigned int NPC_INV_SIZE;

/* ----------------------------------------------------------------------------
** Classes / function prototypes section
** --------------------------------------------------------------------------*/

// a prototype of the world class, necessary for some function prototypes
class World;

/**
 * The particle class, handles a single particle.
 */
class Particles{
public:
	// the location of the particle
	vec2 loc;

	// the width of the particle, in pixels
	float width;

	// the height of the particle, in pixels
	float height;

	// the velocity of the particle, in pixels
	vec2 vel;

	// the color of the particle
	Color color;

	// TODO
	vec2 index;

	// the amount of milliseconds left for the particle to live
	float duration;

	// when true, the particle will move
	bool canMove;

	// TODO
	bool fountain;

	// when true, the particle will be affected by gravity
	bool gravity;

	// when true, draws the particle behind structures
	bool behind;

	// when true, the particle will bounce on impact with ground
	bool bounce;

	// creates a particle with the desired characteristics
	Particles(float x, float y, float w, float h, float vx, float vy, Color c, float d);

	// allows the particle to be destroyed
	~Particles(void){}

	// draws the particle
	std::vector<std::pair<vec2, vec3>> draw(void) const;

	// updates a particle
	void update(float _gravity, float ground_y);

	// returns true if the particle should be killed
	bool kill(float delta);
};

/**
 * The entity class.
 * This class contains common functions and variables for all types of
 * entities, i.e. a common structure.
 */
class Entity{
protected:
	// an incrementer for invincibility after a hit
	unsigned int hitCooldown;

	// an incrementer for triggering change of movement with wander()
	int ticksToUse;

	// entity handles an applied hit (sword) if set true
	bool forcedMove;

	// if set false, entity will be destroyed
	bool alive;

	// TODO
	float targetx;

public:
	// contains the entity's coordinates, in pixels
	vec2 loc;
	float z;

	// contains the entity's velocity, in pixels
	vec2 vel;

	// the entity's width, in pixels
	float width;

	// the entity's height, in pixels
	float height;

	// a speed multiplier, applied to velocity
	float speed;

	// when true player may interact, and the entity's name will be drawn
	bool near;

	// when true, the entity can move
	bool canMove;

	// tells direction entity is facing
	bool right, left;

	// set to 1 if entity is on the ground, 0 if in the air
	unsigned char ground;

	// the entity's inventory
	Inventory *inv;

	// the entity's health
	float health;

	// the most health the entity can have
	float maxHealth;

	// the type of the entity
	_TYPE type;

	// the entity's subtype, if applicable
	int	subtype;

	// the entity's name, randomly generated on spawn
	char *name;

	// the entity's gender
	GENDER  gender;

	// a texture handler for the entity
	TextureIterator tex;

	// draws the entity to the screen
	void draw(void);

	// spawns the entity at the given coordinates
	void spawn(float, float);

	// allows the entity to wander, according to what class is deriving this.
	virtual void wander(int){}
	virtual void wander(void){}

	// allows the entity to interact with the player
	virtual void interact(void){}

	// causes the entity to move to the given x coordinate
	void moveTo(float dest_x);

	// causes the entity to follow the one provided
	void follow(Entity *e);

	// causes the entity to take a player-inflicted hit
	void takeHit(unsigned int _health, unsigned int cooldown);

	// returns the amount of cool down before next hit
	unsigned int coolDown();

	// set the cool down
	void setCooldown(unsigned int c);

	// handles hits if they've been taken
	void handleHits(void);

	// insures that the entity is dead
	void die(void);

	// checks if the entity is alive
	bool isAlive(void) const;

	// checks if the entity is hit in some way
	bool isHit(void) const;

	// returns true if this entity is near the one provided
	bool isNear(Entity e);

	// returns true if the coordinate is within the entity
	bool isInside(vec2 coord) const;

	// a common constructor, clears variables
	Entity(void);

	// frees memory taken by the entity
	virtual ~Entity(){}
};

class Player : public Entity{
public:
	Entity *ride;
	QuestHandler qh;

	Player();
	~Player();
	void save(void);
	void sspawn(float x,float y);
};

class Structures : public Entity{
public:
	BUILD_SUB bsubtype;
	World *inWorld;
	std::string inside;
	std::string textureLoc;

	Structures();
	~Structures();

	unsigned int spawn(BUILD_SUB, float, float);
};


class NPC : public Entity {
private:
	// the number of the random dialog to use
	unsigned int randDialog;

	unsigned int dialogCount;

public:
	int dialogIndex;

	NPC();
	~NPC();

	void drawThingy(void) const;

	void addAIFunc(bool preload);

	virtual void interact();
	virtual void wander(int);
};

class Merchant : public NPC {
public:
	std::vector<Trade>trade;
	uint currTrade;

	std::string text[4];
	std::string *toSay;
	//greeting
	//accept
	//deny
	//farewell

	void interact();
	Structures *inside;

	Merchant();
	~Merchant();

	void wander(int);
};

class Object : public Entity{
private:
	std::string iname;
public:
	std::string pickupDialog;
	bool questObject = false;

	Object();
	Object(std::string in,std::string pd);
	~Object();

	void reloadTexture(void);

	void interact(void);

	bool operator==(const Object &o) {
		return !strcmp(name, o.name) && (loc == o.loc);
	}
};

/**
 * The light structure, used to store light coordinates and color.
 */

class Light{
public:
	vec2 loc;		/**< Light location */
	Color color;	/**< Light color */
	float radius;   /**< Light radius */

	bool belongsTo;
	Entity *following;

	bool flame;
	float fireFlicker;
	vec2 fireLoc;

	Light(vec2 l, Color c, float r){
		loc = l;
		color = c;
		radius = r;

		belongsTo = false;
		following = nullptr;

		flame = false;
	}

	void makeFlame(void){
		flame = true;
	}
};

#include <mob.hpp>

constexpr Object *Objectp(Entity *e) {
	return (Object *)e;
}

constexpr NPC *NPCp(Entity *e) {
	return (NPC *)e;
}

constexpr Structures *Structurep(Entity *e) {
	return (Structures *)e;
}

constexpr Merchant *Merchantp(Entity *e) {
	return (Merchant *)e;
}

#endif // ENTITIES_H

/**
ENTITY TYPES
-1 STRUCTURES
|->1 Village
|->2 Castle
|
0 PLAYERS
|->Player
|
1 NPCS
|->0 Base
|->1 Merchant
|
2 MOBS
|->1 Rabbit
|->2 Bird
**/
