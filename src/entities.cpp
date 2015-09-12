#include <entities.h>

void Entities::spawn(float x, float y){
	loc.x = x;
	loc.y = y;
	vel.x = 0;
	vel.y = 0;
	right = false;
	left = false;
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
