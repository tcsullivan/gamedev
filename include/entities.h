#ifndef ENTITIES_H
#define ENTITIES_H

#include <common.h>

class Entities{
public:
	float width;
	float height;
	float speed;
	int type;
	vec2 loc;
	vec2 vel;
	bool right,left;

	void spawn(float, float);
};

class Player : public Entities{
public:
	Player();
	~Player();
};

class NPC : public Entities{
public:
	NPC();
};

#endif // ENTITIES_H
