#ifndef ENTITIES_H
#define ENTITIES_H

#include <common.h>

class Entity{
public:
	void *inWorld;
	float width;
	float height;
	float speed;
	int type, subtype;
	vec2 loc;
	vec2 vel;
	bool right,left, canMove;
	bool alive;
	unsigned char ground;

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
class Structures : public Entity{
public:
	World *inside;
	Structures();
	unsigned int spawn(int, float, float);
};

#endif // ENTITIES_H
