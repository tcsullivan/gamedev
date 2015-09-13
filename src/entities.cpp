#include <entities.h>

void Entity::spawn(float x, float y){
	loc.x = x;
	loc.y = y;
	vel.x = 0;
	vel.y = 0;
	right = false;
	left = false;
}

void Entity::draw(void){
	glColor3ub(0,0,100);
	glRectf(loc.x,loc.y,loc.x+width,loc.y+height);
}

Player::Player(){
	width = HLINE * 8;
	height = HLINE * 18;
	speed = 1;
	type = 0;
	subtype = 5;
}

NPC::NPC(){
	width = HLINE * 8;
	height = HLINE * 18;
	speed = 1;
	type = 0;
	subtype = 0;
}

Structures::Structures(){
	type = -1;
	speed = 0;
}

void Structures::spawn(int t, float x, float y){
	loc.x = x;
	loc.y = y;
	type = t;

	/*VILLAGE*/
	if(type == -1){
		width =  4 * HLINE;
		height = 4 * HLINE;

		for(int i = 0;i<10;i++){
			entnpc[i] = &npc[i];
			npc[i].type = -1;						 //this will make the NPC spawn the start of a village
			entnpc[i]->spawn(loc.x + (float)(i - 5) / 8,0); //this will spawn the start of a village
		}
	}
}
