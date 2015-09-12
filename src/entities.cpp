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
}

Player::~Player(){

}

NPC::NPC(){
	width = HLINE * 8;
	height = HLINE * 18;
	speed = 1;
	type = 0;
}
