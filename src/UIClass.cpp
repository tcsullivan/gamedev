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
			if(e.key.keysym.sym == SDLK_SPACE) player.loc.y += 10;
			break;
		case SDL_KEYUP:
			if(e.key.keysym.sym == SDLK_d) player.right = false;
			if(e.key.keysym.sym == SDLK_a) player.left = false;
			if(e.key.keysym.sym == SDLK_ESCAPE) gameRunning = false;
			break;
		}	
	}
}