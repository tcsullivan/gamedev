<<<<<<< HEAD
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
	vec2 loci;
	vec2 vel;
	vec2 velg;

	void spawn(float, float);


};

class Player : public Entities{
public:
	Player();
	~Player();
};

=======
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

>>>>>>> origin/master
#endif //ENTITIES_H