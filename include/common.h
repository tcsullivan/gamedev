#ifndef COMMON_H
#define COMMON_H

///THIS FILE IS USED FOR VARIABLES THAT WILL BE ACCESED BY MULTIPLE CLASSES/FILES

#include <iostream>
#include <vector>
#include <math.h>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>

typedef struct { float x; float y; }vec2;

enum _TYPE { //these are the main types of entities
	STRUCTURET = -1,
	PLAYERT    = 0,
	NPCT       = 1
};

enum GENDER{
	MALE,
	FEMALE,
	NONE
};

#include <entities.h>

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720
//#define FULLSCREEN

#define HLINE 3								//base unit of the world

#define initRand(s) srand(s)
#define getRand()	rand()

template<typename T, size_t N>						//this fuction returns the size of any array
int eAmt(T (&)[N]){return N;}

extern bool gameRunning;
extern unsigned int deltaTime;

extern FILE* config;
extern FILE* names;

#endif // COMMON_H
