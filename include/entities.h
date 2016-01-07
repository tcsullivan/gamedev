#ifndef ENTITIES_H
#define ENTITIES_H

#include <common.h>
#include <Quest.h>
#include <inventory.h>

#define DEBUG

#define NPCp(n)			((NPC *)n)
#define Structurep(n)	((Structures *)n)
#define Mobp(n)			((Mob *)n)

#define PLAYER_INV_SIZE	30	// The size of the player's inventory
#define NPC_INV_SIZE	3	// Size of an NPC's inventory

enum _TYPE { //these are the main types of entities
	OBJECTT = -2,
	STRUCTURET,
	PLAYERT,
	NPCT,
	MOBT
};

enum GENDER{
	MALE,
	FEMALE,
	NONE 
};

enum MOB_SUB {
	MS_RABBIT = 1,
	MS_BIRD,
	MS_TRIGGER,
	MS_DOOR,
	MS_PAGE
};

enum BUILD_SUB{
	TOWN_HALL = 1,
	HOUSE,
	HOUSE2,
	HOUSE3,
	HOUSE4,
	FOUNTAIN
};

typedef struct {
	InventorySavePacket isp;
	vec2 loc;
	vec2 vel;
	float width;
	float height;
	float speed;
	float health;
	float maxHealth;
	int subtype;
	int ticksToUse;
	unsigned int randDialog;
	unsigned char ground;
	bool near;
	bool canMove;
	bool right,left;
	bool alive;
	bool hit;
	_TYPE type;
	GENDER  gender;
	size_t nameSize;
	char   name[32];
	//Texturec *tex;
} __attribute__ ((packed)) EntitySavePacket;

class World;

class Particles{
public:
	vec2 loc;
	float width;
	float height;
	float velx;
	float vely;
	Color color;
	int duration;
	bool canMove;
	Particles(float x, float y, float w, float h, float vx, float vy, Color c, int d){
		loc.x = (x);
		loc.y = (y);
		width = (w);
		height = (h);
		velx = vx;
		vely = vy;
		color.red = (c.red);
		color.green = (c.green);
		color.blue = (c.blue);
		duration = d;
	}
	~Particles(){}
	void draw(){
		glColor3f(color.red,color.green,color.blue);
		glRectf(loc.x,loc.y,loc.x+width,loc.y+height);
	}
	bool kill(float delta){
		duration -= delta;
		if(duration <= 0)
			return true;
		else return false;
	}
};

class Entity{
public:
	Inventory *inv;

	/*
	 *	Movement variables
	*/

	vec2 loc;
	vec2 vel;
	
	float width;
	float height;
	
	float speed;	// A speed factor for X movement

	/*
	 *	Movement flags
	*/

	bool near;				// Causes name to display
	bool canMove;			// Enables movement
	bool right,left;		// Direction faced by Entity
	bool alive;
	bool hit;
	unsigned char ground;	// Shows how the Entity is grounded (if it is)

	/*
	 *	Health variables
	*/

	float health;
	float maxHealth;

	/*
	 *	Identification variables
	*/

	_TYPE type;
	int	  subtype;

	char   *name;
	GENDER  gender;
	
	Texturec *tex;
	Texturec *ntex;

	unsigned int randDialog;

	void draw(void);
	void spawn(float, float);
	
	int ticksToUse;				// Used by wander()
	
	virtual void wander(int){}
	virtual void interact(){}
	
	virtual ~Entity(){}
	
	char *baseSave(void);
	void baseLoad(char *);
};

class Player : public Entity{
public:
	QuestHandler qh;
	bool light = false;
	
	Player();
	~Player();
	void interact();
};

class NPC : public Entity{
public:
	std::vector<int (*)(NPC *)>aiFunc;
	
	NPC();
	~NPC();
	
	void addAIFunc(int (*func)(NPC *),bool preload);
	void interact();
	void wander(int);
	
	char *save(unsigned int *size);
	void load(unsigned int,char *b);
};

typedef struct {
	EntitySavePacket esp;
	World *inWorld;
	World *inside;
	BUILD_SUB bsubtype;
} __attribute__ ((packed)) StructuresSavePacket;

class Structures : public Entity{
public:
	World *inWorld;
	World *inside;
	BUILD_SUB bsubtype;
	
	Structures();
	~Structures();
	
	unsigned int spawn(_TYPE, BUILD_SUB, float, float, World *);
	
	char *save(void);
	void load(char *s);
};

class Mob : public Entity{
public:
	double init_y;
	void (*hey)(Mob *callee);
	
	Mob(int);
	Mob(int,unsigned int);
	~Mob();
	
	void wander(int);
	
	char *save(void);
	void load(char *);
};

typedef struct {
	EntitySavePacket esp;
	double init_y;
	//void (*hey)(Mob *callee);
} __attribute__ ((packed)) MobSavePacket;

typedef struct {
	EntitySavePacket esp;
	ITEM_ID identifier;
	bool questObject;
	char pickupDialog[256];
} __attribute__ ((packed)) ObjectSavePacket;

class Object : public Entity{
private:
	ITEM_ID identifier;
public:
	char *pickupDialog;
	bool questObject = false;
	
	Object();
	Object(ITEM_ID id, bool qo, const char *pd);
	~Object();
	
	void reloadTexture(void);
	
	void interact(void);
	
	char *save(void);
	void load(char *);
};
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
