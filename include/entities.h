#ifndef ENTITIES_H
#define ENTITIES_H

#include <common.h>

extern int npcAmt;

class Entity{
public:
	float width;
	float height;
	float speed;
	int type, subtype;
	vec2 loc;
	vec2 vel;
	bool right,left, canMove;
	bool alive;

	unsigned int texture[];

	void spawn(float, float);
	void draw(void);
	void wander(int, vec2*);
	virtual void interact(){}
private:
	int ticksToUse;
};

class Player : public Entity{
public:
	Player();
	void interact();
};

class NPC : public Entity{
public:
	NPC();
	void interact();
};

extern Entity *entnpc[32];	//The NPC base
extern NPC npc[32];

class Structures : public Entity{
public:
	Structures();
	void spawn(int, float, float);
};

#endif // ENTITIES_H
