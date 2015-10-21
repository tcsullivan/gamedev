#include <common.h>

#define DEBUG

GLuint loadTexture(const char *fileName){
	SDL_Surface *image = IMG_Load(fileName);

	if(!image)return 0;
#ifdef DEBUG
	DEBUG_printf("Loaded image file: %s\n", fileName);
#endif // DEBUG
	unsigned object = 0; //creates a new unsigned variable for the texture

	glGenTextures(1, &object); //turns "object" into a texture
	glBindTexture(GL_TEXTURE_2D, object); //binds "object" to the top of the stack

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //sets the "min" filter
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //the the "max" filter of the stack

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //Wrap the texture to the matrix
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); //Wrap the texutre to the matrix

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels); //sets the texture to the image file loaded above

	SDL_FreeSurface(image);	//Free surface
	return object;
}

void DEBUG_prints(const char* file, int line, const char *s,...){
	va_list args;
	printf("%s:%d: ",file,line);
	va_start(args,s);
	vprintf(s,args);
	va_end(args);
}
