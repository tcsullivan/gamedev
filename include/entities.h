#ifndef ENTITIES_H
#define ENTITIES_H

#include <common.h>

class Entity{
public:
	float width;
	float height;
	float speed;
	int type;
	vec2 loc;
	vec2 vel;
	bool right,left;

	void spawn(float, float);
	void draw(void);
};

class Player : public Entity{
public:
	Player();
	~Player();
};

class NPC : public Entity{
public:
	NPC();
};

#endif // ENTITIES_H
