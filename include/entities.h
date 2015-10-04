#ifndef ENTITIES_H
#define ENTITIES_H

#include <common.h>
#include <inventory.h>

#define NPCp(n) ((NPC *)n)

#define PLAYER_INV_SIZE	30	// The size of the player's inventory
#define NPC_INV_SIZE	3	// Size of an NPC's inventory

extern FILE* names;

class Entity{
public:
	Inventory *inv;

	void *inWorld;
	
	float width;	//width and height of the player
	float height;
	float speed;	//speed of the play
	
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
	unsigned int texture;	  //TODO: ADD TEXTURES

	
	void spawn(float, float);
	void draw(void);
	void wander(int, vec2*);
	void getName();
	virtual void interact(){}
private:
	int ticksToUse;	//The variable for deciding how long an entity should do a certain task
};

class Player : public Entity{
public:
	QuestHandler qh;
	Player();
	void interact();
};

class NPC : public Entity{
private:
	std::vector<int (*)(NPC *)>aiFunc;
public:
	NPC();
	void addAIFunc(int (*func)(NPC *));
	void interact();
};
class Structures : public Entity{
public:
	void *inside;
	Structures();
	unsigned int spawn(_TYPE, float, float);
};

#endif // ENTITIES_H
