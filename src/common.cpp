#include <common.h>

/*SDL_Surface* loadTexture(char* filename){
	SDL_Surface *optimizedSurface = NULL;
	SDL_Surface *texture = IMG_Load(filename);
	if(texture == NULL){
		std::cout << "Unable to load an image properly from " << filename << "; Error: " << IMG_GetError() << std::endl;
	}else{
		optimizedSurface = SDL_ConvertSurface(texture, renderSurface->format, NULL);
		if(optimizedSurface == NULL){
			std::cout << "Unable to optimize image properly from " << filename << "; Error: " << IMG_GetError() << std::endl;
		}
		SDL_FreeSurface(texture);
	}
	return optimizedSurface;
}*/

GLuint loadTexture(const char *fileName){
  SDL_Surface *image = IMG_Load(fileName);
 
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