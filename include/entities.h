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
	STRUCTURET = -1,
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
	MS_TRIGGER
};

class Entity{
public:
	Inventory *inv;

	float width;	//width and height of the player
	float height;
	float speed;	//speed of the play

	float health;
	float maxHealth;

	int subtype;
	_TYPE type;
			//example:
			//type 	1(NPC)
			//		|(subtype)
			//		|->  0 Base NPC
			//		|->  1 Merchant

	vec2 loc; //location and velocity of the entity
	vec2 vel;

	bool near;
	bool right,left, canMove; //movement variables
	bool alive;				  //the flag for whether or not the entity is alive
	unsigned char ground;	  //variable for testing what ground the entity is on to apply certain traits

	char* name;
	GENDER gender;
	//GLuint texture[3];	  //TODO: ADD TEXTURES
	Texturec* tex;


	void spawn(float, float);
	void draw(void);
	virtual void wander(int){}
	void getName();
	virtual void interact(){}
	int ticksToUse;	//The variable for deciding how long an entity should do a certain task
private:
};

class Player : public Entity {
public:
	QuestHandler qh;
	Player();
	void interact();
	bool light = false;
};

class NPC : public Entity{
public:
	std::vector<int (*)(NPC *)>aiFunc;
	NPC();
	void addAIFunc(int (*func)(NPC *),bool preload);
	void interact();
	void wander(int);
};

class Structures : public Entity{
public:
	void *inWorld;
	void *inside;
	Structures();
	unsigned int spawn(_TYPE, float, float);
};

class Mob : public Entity{
public:
	double init_y;
	void (*hey)();
	Mob(int);
	Mob(int,unsigned int);
	void wander(int);
};

class Object : public Entity{
public:
	Object(int);
	Object(int, bool, char*);
	void interact();
	bool questObject = false;
	char *pickupDialog;
private:
	int identifier;
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
