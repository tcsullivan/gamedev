#include <UIClass.h>

void UIClass::handleEvents(){
	SDL_Event e;
	while(SDL_PollEvent(&e)){
		switch(e.type){
		case SDL_QUIT:
			gameRunning=false;
			break;
		case SDL_KEYDOWN:
			switch(e.key.keysym.sym){
			case 27:
				gameRunning=false;
				break;
			default:
				break;
			}
		default:
			break;
		}
	}
}
