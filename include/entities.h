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
	TOWN_HALL = 0,
	HOUSE = 1,
	HOUSE2 = 2,
	HOUSE3 = 3,
	HOUSE4 = 4,
	FOUNTAIN = 5,
	LAMP_POST = 6,
	FIRE_PIT = 7
};

class World;

class Particles{
public:
	vec2 loc;
	float width;
	float height;
	float velx;
	float vely;
	Color color;
	vec2 index;
	//GLuint tex;
	float duration;
	bool canMove;
	bool fountain;
	bool gravity;
	bool behind;
	Particles(float x, float y, float w, float h, float vx, float vy, Color c, float d){
		loc.x = x;
		loc.y = y;
		width = w;
		height = h;
		velx = vx;
		vely = vy;
		color = c;
		duration = d;
		fountain = false;
		gravity = true;
		behind = false;
		index = Texture::getIndex(c);
		//tex = text;
	}
	~Particles(){

	}
	void draw(){
		//glColor3f(color.red,color.green,color.blue);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorIndex);
		glUseProgram(shaderProgram);
		glUniform1i(glGetUniformLocation(shaderProgram, "sampler"), 0);
		glEnable(GL_TEXTURE_2D);
		glColor3ub(255,255,255);
		glBegin(GL_QUADS);
			glTexCoord2f(.25*index.x, .126*index.y);	glVertex2i(loc.x, loc.y);
			glTexCoord2f(.26*index.x, .126*index.y);	glVertex2i(loc.x + width, loc.y);
			glTexCoord2f(.26*index.x, .125*index.y);	glVertex2i(loc.x + width, loc.y + height);
			glTexCoord2f(.25*index.x, .125*index.y);	glVertex2i(loc.x, loc.y + width);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glUseProgram(0);
		//glRectf(loc.x,loc.y,loc.x+width,loc.y+height);
	}
	bool kill(float delta){
		duration -= delta;
		if(duration <= 0.0f){
			return true;
		}
		else return false;
	}
};

void initEntity();

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
	int dialogIndex;
	
	NPC();
	~NPC();
	
	void addAIFunc(int (*func)(NPC *),bool preload);
	void clearAIFunc(void);
	void interact();
	void wander(int);
};

class Structures : public Entity{
public:
	BUILD_SUB bsubtype;
	char *inside;
	
	Structures();
	~Structures();
	
	unsigned int spawn(BUILD_SUB, float, float);
};

class Mob : public Entity{
public:
	double init_y;
	void (*hey)(Mob *callee);
	
	Mob(int);
	~Mob();
	
	void wander(int);
};

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
