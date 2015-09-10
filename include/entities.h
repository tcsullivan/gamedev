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

	void spawn(float, float);


};

class Player : public Entities{
public:
	Player();
	~Player();
};

#endif //ENTITIES_H