#include <Texture.h>
#include <string.h>

struct texture_t {
	char *name;
	GLuint tex;
} __attribute__ ((packed));

struct texture_t *LoadedTexture[256];
unsigned int LoadedTextureCounter = 0;

namespace Texture{
	Color pixels[8][4];
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

		if(!fileName)
			return 0;

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
		
		LoadedTexture[LoadedTextureCounter]		  = new struct texture_t;		//(struct texture_t *)malloc(sizeof(struct texture_t));
		LoadedTexture[LoadedTextureCounter]->name = new char[strlen(fileName)+1];	//(char *)malloc(safe_strlen(fileName));
		LoadedTexture[LoadedTextureCounter]->tex  = object;
		strcpy(LoadedTexture[LoadedTextureCounter]->name,fileName);
		LoadedTextureCounter++;
		
		return object;
	}

	void initColorIndex(){
		colorIndex = loadTexture("assets/colorIndex.png");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorIndex);
		GLubyte* buffer = new GLubyte[8*4*3];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		uint i = 0;
		for(uint o = 0; o < 8; o++){
			for(uint t = 0; t < 4; t++){
				for (int r = 0; r < 3; r++){
					pixels[o][t].red = buffer[i++];
					pixels[o][t].green = buffer[i++];
					pixels[o][t].blue = buffer[i++];
					std::cout << pixels[o][t].red << "," << pixels[o][t].green << "," << pixels[o][t].blue << std::endl;
				}
				//std::cout << std::endl;
			}
		}

	}

	//sqrt((255-145)^2+(90-145)^2+(0-0)^2);
	vec2 getIndex(Color c){
		uint buf[2];
		float buff = 999;
		float shit = 999;
		for(uint o = 0; o < 8; o++){
			for(uint t = 0; t < 4; t++){
				buff = sqrt(pow((c.red-pixels[o][t].red),2)+pow((c.green-pixels[o][t].green),2)+pow((c.blue-pixels[o][t].blue),2));
				//std::cout << buff << std::endl;
				if(buff < shit){
					shit = buff;
					buf[0] = o;
					buf[1] = t;
				}
			}
		}
		std::cout << float(buf[1]) << ", " << float(buf[0]) << std::endl;
		return {float(buf[1]),float(buf[0])};
	}
}

Texturec::Texturec(uint amt, ...){
	va_list fNames;
	texState = 0;
	image = new GLuint[amt];
	va_start(fNames, amt);
	for(unsigned int i = 0; i < amt; i++){
		image[i] = Texture::loadTexture(va_arg(fNames, char *));
	}
	va_end(fNames);
}

Texturec::Texturec(uint amt,const char **paths){
	texState = 0;
	image = new GLuint[amt];
	for(unsigned int i = 0; i < amt; i++){
		image[i] = Texture::loadTexture(paths[i]);
	}
}

Texturec::~Texturec(){
	delete[] image;
}

void Texturec::bind(unsigned int bn){
	texState = bn;
	glBindTexture(GL_TEXTURE_2D,image[(int)texState]);
}

void Texturec::bindNext(){
	bind(++texState);
}

void Texturec::bindPrev(){
	bind(--texState);
}
