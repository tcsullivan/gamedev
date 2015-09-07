#ifndef UICLASS_H
#define UICLASS_H

#include <iostream>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

extern SDL_Event e;
extern bool gameRunning;

class UIClass{
	public:
		void handleEvents();
	
};

#endif //UICLASS_H