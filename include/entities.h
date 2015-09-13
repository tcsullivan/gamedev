#ifndef ENTITIES_H
#define ENTITIES_H

#include <common.h>

class Entity{
public:
	float width;
	float height;
	float speed;
	int type, subtype;
	vec2 loc;
	vec2 vel;
	bool right,left;

	void spawn(float, float);
	void draw(void);
};

class Player : public Entity{
public:
	Player();
};

class NPC : public Entity{
public:
	NPC();
};

extern Entity *entnpc[10];	//The NPC base
extern NPC npc[10];

class Structures : public Entity{
public:
	Structures();
	void spawn(int, float, float);
};

#endif // ENTITIES_H
