#include <common.h>

GLuint loadTexture(const char *fileName){
	SDL_Surface *image = IMG_Load(fileName);

	if(!image)return 0;

	//SDL_DisplayFormatAlpha(image);

	unsigned object(0);

	glGenTextures(1, &object);

	glBindTexture(GL_TEXTURE_2D, object);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);

	//Free surface
	SDL_FreeSurface(image);

	return object;
}

void DEBUG_printf(const char *s,...){
	va_list args;
	printf("%s:%u: ",__FILE__,__LINE__);
	va_start(args,s);
	vprintf(s,args);
	va_end(args);
}
