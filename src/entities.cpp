#include <entities.h>

extern std::vector<Entity*>entity;
extern std::vector<NPC>npc;
extern std::vector<Structures>build;

void Entity::spawn(float x, float y){	//spawns the entity you pass to it based off of coords and global entity settings
	loc.x = x;
	loc.y = y;
	vel.x = 0;
	vel.y = 0;
	right = false;
	left = false;
	ticksToUse = 0;
	canMove = false;
	ground = false;
}

void Entity::draw(void){		//draws the entities
	if(type==NPCT)
		glColor3ub(0,0,100);
	else if(type==STRUCTURET)
		glColor3ub(100,0,100);
	glRectf(loc.x,loc.y,loc.x+width,loc.y+height);
}

void Entity::wander(int timeRun, vec2 *v){ //this makes the entites wander about
	static int direction;	//variable to decide what direction the entity moves
	if(ticksToUse == 0){
		ticksToUse = timeRun;
		v->x = .008*HLINE;	//sets the inital velocity of the entity
		direction = (getRand() % 3 - 1); 	//sets the direction to either -1, 0, 1
											//this lets the entity move left, right, or stay still
		v->x *= direction;	//changes the velocity based off of the direction
	}
	ticksToUse--; //removes one off of the entities timer
}

Player::Player(){ //sets all of the player specific traits on object creation
	width = HLINE * 8;
	height = HLINE * 12;
	speed = 1;
	type = PLAYERT; //set type to player
	subtype = 5;
	alive = true;
	ground = false;
}

void Player::interact(){ //the function that will cause the player to search for things to interact with
	
}

NPC::NPC(){	//sets all of the NPC specific traits on object creation
	width = HLINE * 8;
	height = HLINE * 12;
	speed = 1;
	type = NPCT; //sets type to npc
	subtype = 0;
	alive = true;
	canMove = true;
}

void NPC::interact(){ //have the npc's interact back to the player
	//loc.y += .01;
}

Structures::Structures(){ //sets the structure type
	type = STRUCTURET;
	speed = 0;
	alive = true;
}

unsigned int Structures::spawn(_TYPE t, float x, float y){ //spawns a structure based off of type and coords
	loc.x = x;
	loc.y = y;
	type = t;

	/*VILLAGE*/
	//spawns a village
	//spawns between 1 and 5 villagers around the village
	if(type == STRUCTURET){
		loc.y=100;
		width =  20 * HLINE;
		height = 16 * HLINE;

		int tempN = (getRand() % 5 + 1); //amount of villagers that will spawn
		//int tempN=200;
		for(int i=0;i<tempN;i++){
			entity.push_back(new NPC()); //create a new entity of NPC type
			npc.push_back(NPC()); //create new NPC
			entity[entity.size()] = &npc[npc.size()-1]; //set the new entity to have the same traits as an NPC
			entity[entity.size()-1]->spawn(loc.x + (float)(i - 5),100); //sets the position of the villager around the village
		}
		entity.pop_back();
		return entity.size();
	}
}
