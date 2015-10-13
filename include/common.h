#ifndef COMMON_H
#define COMMON_H

///THIS FILE IS USED FOR VARIABLES THAT WILL BE ACCESED BY MULTIPLE CLASSES/FILES

#include <iostream>
#include <vector>
#include <math.h>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_opengl.h>

typedef struct { float x; float y; }vec2;

enum _TYPE { //these are the main types of entities
	STRUCTURET = -1,
	PLAYERT    = 0,
	NPCT       = 1,
	MOBT	   = 2
};

enum GENDER{
	MALE,
	FEMALE,
	NONE 
};

#include <Quest.h>
#include <entities.h>

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720
//#define FULLSCREEN

#define HLINE 3								//base unit of the world

#define initRand(s) srand(s)
#define getRand()	rand()

#define DEBUG_printf( message, ...) DEBUG_prints(__FILE__, __LINE__, message, __VA_ARGS__ )

template<typename T, size_t N>				//this fuction returns the size of any array
int eAmt(T (&)[N]){return N;}

extern bool gameRunning;
extern unsigned int deltaTime;
extern unsigned int loops;

extern FILE* config;
extern FILE* names;

extern Mix_Music *music;
extern Mix_Chunk *horn;

GLuint loadTexture(const char *fileName);
void DEBUG_prints(const char* file, int line, const char *s,...);

#endif // COMMON_H
