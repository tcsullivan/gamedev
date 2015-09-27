#include <entities.h>

extern std::vector<Entity*>entity;
extern std::vector<NPC>npc;
extern std::vector<Structures>build;

void Entity::spawn(float x, float y){
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

void Entity::draw(void){
	glColor3ub(0,0,100);
	glRectf(loc.x,loc.y,loc.x+width,loc.y+height);
}

void Entity::wander(int timeRun, vec2 *v){
	static int direction;
	if(ticksToUse == 0){
		ticksToUse = timeRun;
		v->x = .01*HLINE;
		direction = (getRand() % 3 - 1);
		v->x *= direction;
	}
	ticksToUse--;
}

Player::Player(){
	width = HLINE * 8;
	height = HLINE * 12;
	speed = 1;
	type = 0;
	subtype = 5;
	alive = true;
	ground = false;
}

void Player::interact(){
	
}

NPC::NPC(){	
	width = HLINE * 8;
	height = HLINE * 12;
	speed = 1;
	type = 0;
	subtype = 0;
	alive = false;
	canMove = true;
}

void NPC::interact(){
	//loc.y += .01;
}

Structures::Structures(){
	type = -1;
	speed = 0;
}

unsigned int Structures::spawn(int t, float x, float y){
	loc.x = x;
	loc.y = y;
	type = t;

	/*VILLAGE*/
	if(type == -1){
		width =  4 * HLINE;
		height = 4 * HLINE;

		//int tempN = (getRand() % 5 + 1);
		int tempN = 2;
		for(int i=0;i<tempN;i++){
			entity.push_back(new Entity());
			npc.push_back(NPC());
			std::cout<<"NPC:"<<npc.size()<<std::endl;
			std::cout<<"Entity:"<<entity.size()<<std::endl;
			entity[entity.size()] = &npc[npc.size()-1];
			entity[entity.size()]->alive=true;
			entity[entity.size()]->type = 1;
			entity[entity.size()]->spawn(loc.x + (float)(i - 5) / 8,100);
		}
		return entity.size();
	}
}
