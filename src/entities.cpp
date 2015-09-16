#include <entities.h>

void Entity::spawn(float x, float y){
	loc.x = x;
	loc.y = y;
	vel.x = 0;
	vel.y = 0;
	right = false;
	left = false;
	ticksToUse = 0;
	canMove = false;
}

void Entity::draw(void){
	glColor3ub(0,0,100);
	glRectf(loc.x,loc.y,loc.x+width,loc.y+height);
}

void Entity::wander(int timeRun, vec2 *v){
	static int hey;
	if(ticksToUse == 0){
		ticksToUse = timeRun;
		v->x = .00010;
		hey = (grand() % 3 - 1);
		v->x *= hey;
	}
	ticksToUse--;
}

Player::Player(){
	width = HLINE * 8;
	height = HLINE * 18;
	speed = 1;
	type = 0;
	subtype = 5;
	alive = true;
}

void Player::interact(){
	
}

NPC::NPC(){	
	width = HLINE * 8;
	height = HLINE * 18;
	speed = 1;
	type = 0;
	subtype = 0;
	alive = false;
	canMove = true;
}

void NPC::interact(){

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

		int tempN = (grand() % 5 + 1);
		npcAmt = tempN;

		for(int i = 0;i<eAmt(entnpc);i++){
			npc[i].alive = true;
			entnpc[i] = &npc[i];
			npc[i].type = -1;						 //this will make the NPC spawn the start of a village
			entnpc[i]->spawn(loc.x + (float)(i - 5) / 8,0); //this will spawn the start of a village
		}
	}
}
