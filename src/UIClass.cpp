#include <UIClass.h>

extern Player player;
extern World *currentWorld;

void UIClass::handleEvents(){
	SDL_Event e;
	while(SDL_PollEvent(&e)){
		switch(e.type){
		case SDL_QUIT:
			gameRunning=false;
			break;
		case SDL_KEYDOWN:
			switch(e.key.keysym.sym){
			case 27:						///ESCAPE
				gameRunning=false;
				break;
			case SDLK_d:					///D
				player.velg.x = 10;
				break;
			case SDLK_a:					///A
				player.velg.x = -10;
				break;
			case SDLK_i:
				if(currentWorld->behind)currentWorld=currentWorld->behind;
				break;
			case SDLK_k:
				if(currentWorld->infront)currentWorld=currentWorld->infront;
				break;
			default:
				break;
			}
		case SDL_KEYUP:
			switch(e.key.keysym.sym){
			/*case SDLK_d:					///D
				break;
			case SDLK_a:					///A
				break;*/
			default:
				break;
			}
		default:
			break;
		}
	}
}
