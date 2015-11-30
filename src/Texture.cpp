#include <Texture.h>
#include <string.h>

struct texture_t {
	char *name;
	GLuint tex;
} __attribute__ ((packed));

struct texture_t *LoadedTexture[256];
unsigned int LoadedTextureCounter = 0;

namespace Texture{
	GLuint loadTexture(const char *fileName){
		SDL_Surface *image;
		GLuint object = 0;

		for(unsigned int i=0;i<LoadedTextureCounter;i++){
			if(!strcmp(LoadedTexture[i]->name,fileName)){
#ifdef DEBUG
				DEBUG_printf("Reusing loaded texture for %s\n",fileName);
#endif // DEBUG
				return LoadedTexture[i]->tex;
			}
		}

		if(!(image = IMG_Load(fileName)))
			return 0;
#ifdef DEBUG
		DEBUG_printf("Loaded image file: %s\n", fileName);
#endif // DEBUG
		
		glGenTextures(1,&object);				// Turns "object" into a texture
		glBindTexture(GL_TEXTURE_2D,object);	// Binds "object" to the top of the stack
		glPixelStoref(GL_UNPACK_ALIGNMENT,1);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Sets the "min" filter
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// The the "max" filter of the stack

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Wrap the texture to the matrix
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); //

		glTexImage2D(GL_TEXTURE_2D,  // Sets the texture to the image file loaded above
					 0, 
					 GL_RGBA,
					 image->w,
					 image->h, 
					 0, 
					 GL_RGBA, 
					 GL_UNSIGNED_BYTE, 
					 image->pixels
					 );

		SDL_FreeSurface(image);	// Free the surface
		
		LoadedTexture[LoadedTextureCounter]		  = (struct texture_t *)malloc(sizeof(struct texture_t));
		LoadedTexture[LoadedTextureCounter]->name = (char *)malloc(safe_strlen(fileName));
		LoadedTexture[LoadedTextureCounter]->tex  = object;
		strcpy(LoadedTexture[LoadedTextureCounter]->name,fileName);
		LoadedTextureCounter++;
		
		return object;
	}
}

Texturec::Texturec(uint amt, ...){
	image = new GLuint(amt);
	va_list fNames;
	va_start(fNames, amt);
	for(int i = 0; i < amt; i++){
		char* f = va_arg(fNames, char*);
		image[i] = Texture::loadTexture(f);
	}
	va_end(fNames);
}

void Texturec::bind(int bn){
	texState = bn;
	glBindTexture(GL_TEXTURE_2D, image[texState]);
}

void Texturec::bindNext(){
	bind(++texState);
}

void Texturec::bindPrev(){
	bind(--texState);
}

void Texturec::walk(){
	// hey
}
