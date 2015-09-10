<<<<<<< HEAD
#include <entities.h>

void Entities::spawn(float x, float y){
	loc.x = x;
	loc.y = y;
	loci.x = loc.x;
	loci.y = loc.y;
	vel.x = 0;
	vel.y = 0;
	velg.x = 0;
	velg.y = 0;
}

Player::Player(){
	width = HLINE * 6;
	height = HLINE * 16;
	speed = 1;
	type = 0;
}

=======
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

>>>>>>> origin/master
Player::~Player(){}