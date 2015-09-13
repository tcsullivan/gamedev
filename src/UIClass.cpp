#include <UIClass.h>

extern Player player;
extern World *currentWorld;

void UIClass::handleEvents(){
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
			if(e.key.keysym.sym == SDLK_SPACE) player.loc.y += .5;
			if(e.key.keysym.sym == SDLK_i)
				if(currentWorld->behind){
					player.loc.x-=(currentWorld->getWidth()-currentWorld->behind->getWidth())/2; // Match player's location to new area
					currentWorld=currentWorld->behind;											 // Go to new area
					if(player.loc.x>-1+currentWorld->getWidth())								 // Don't let player fall out of world if previous area was bigger
						player.loc.x=-1+currentWorld->getWidth()-player.width-HLINE;
				}
			if(e.key.keysym.sym == SDLK_k)
				if(currentWorld->infront){
					player.loc.x+=(currentWorld->infront->getWidth()-currentWorld->getWidth())/2; // Match player's location to new area
					currentWorld=currentWorld->infront;											  // Go to new area
					if(player.loc.x>-1+currentWorld->getWidth())								  // Don't let player fall out of world if previous area was bigger
						player.loc.x=-1+currentWorld->getWidth()-player.width-HLINE;
				}
			break;
		case SDL_KEYUP:
			if(e.key.keysym.sym == SDLK_d) player.right = false;
			if(e.key.keysym.sym == SDLK_a) player.left = false;
			if(e.key.keysym.sym == SDLK_ESCAPE) gameRunning = false;
			break;
		}	
	}
}
