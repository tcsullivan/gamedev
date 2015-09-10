#include <entities.h>

void Entities::spawn(float x, float y){
	loc.x = x;
	loc.y = y;

}

Player::Player(){
	width = 24;
	height = 42;
	speed = 1;
	type = 0;
}

Player::~Player(){}