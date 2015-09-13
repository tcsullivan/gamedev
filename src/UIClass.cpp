#include <UIClass.h>

extern Player player;
extern World *currentWorld;

void UIClass::handleEvents(){
	float thing;
	SDL_Event e;
	while(SDL_PollEvent(&e)){
		switch(e.type){
		case SDL_WINDOWEVENT:
			switch(e.window.event){
				case SDL_WINDOWEVENT_CLOSE:
					gameRunning = false;
				break;
			}
		case SDL_KEYDOWN:
			if(e.key.keysym.sym == SDLK_d) player.right = true;
			if(e.key.keysym.sym == SDLK_a) player.left = true;
			if(e.key.keysym.sym == SDLK_LSHIFT) player.speed = 3;
			if(e.key.keysym.sym == SDLK_SPACE){
				player.loc.y += HLINE*1.2;
				player.vel.y += .004;
			}
			if(e.key.keysym.sym == SDLK_i){
				if(currentWorld->behind){
					thing=(currentWorld->getWidth()-currentWorld->behind->getWidth())/2;
					if(player.loc.x>thing-1&&
					   player.loc.x<thing-1+currentWorld->behind->getWidth()){
						player.loc.x-=thing;
						currentWorld=currentWorld->behind;
					}
				}
			}
			if(e.key.keysym.sym == SDLK_k){
				if(currentWorld->infront){
					player.loc.x+=(currentWorld->infront->getWidth()-currentWorld->getWidth())/2;
					currentWorld=currentWorld->infront;
				}
			}
			break;
		case SDL_KEYUP:
			if(e.key.keysym.sym == SDLK_d) player.right = false;
			if(e.key.keysym.sym == SDLK_a) player.left = false;
			if(e.key.keysym.sym == SDLK_LSHIFT) player.speed = 1.0;
		
			if(e.key.keysym.sym == SDLK_ESCAPE) gameRunning = false;
			break;
		}	
	}
}
