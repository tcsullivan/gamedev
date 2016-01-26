#include <Texture.h>
#include <string.h>

struct texture_t {
	char *name;
	GLuint tex;
} __attribute__ ((packed));

struct index_t{
	Color color;
	int indexx;
	int indexy;
};

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
	
	void freeTextures(void){
		for(unsigned int i=0;i<LoadedTextureCounter;i++){
			glDeleteTextures(1,&LoadedTexture[i]->tex);
			delete[] LoadedTexture[i]->name;
			delete LoadedTexture[i];
		}
	}

	#define CINDEX_WIDTH (8*4*3)
	void initColorIndex(){
		unsigned int i;
		GLubyte *buffer;
		GLfloat *bufferf;
		
		buffer  = new GLubyte[CINDEX_WIDTH];
		bufferf = new GLfloat[CINDEX_WIDTH];
		
		colorIndex = loadTexture("assets/colorIndex.png");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorIndex);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
		
		for(i = 0; i < CINDEX_WIDTH; i++)
			bufferf[i] = (float)buffer[i] / 255.0f;
		
		i = 0;
		for(unsigned int y = 0; y < 8; y++){
			for(unsigned int x = 0; x < 4; x++){
					if(i >= CINDEX_WIDTH){
						delete[] buffer;
						delete[] bufferf;
						return;
					}
					pixels[y][x].red = buffer[i++];
					pixels[y][x].green = buffer[i++];
					pixels[y][x].blue = buffer[i++];
			}
		}
		delete[] buffer;
		delete[] bufferf;
	}

	//sqrt((255-145)^2+(90-145)^2+(0-0)^2);
	std::vector<index_t>ind;
	vec2 getIndex(Color c){
		for(auto &i : ind){
			if(c.red == i.color.red && c.green == i.color.green && c.blue == i.color.blue){
				//std::cout << float(i.indexy) << "," << float(i.indexx) << std::endl;
				return {float(i.indexx), float(i.indexy)};
			}
		}
		uint buf[2];
		float buff = 999;
		float shit = 999;
		for(uint y = 0; y < 8; y++){
			for(uint x = 0; x < 4; x++){
				//std::cout << y << "," << x << ":" << pixels[y][x].red << "," << pixels[y][x].green << "," << pixels[y][x].blue << std::endl;
				buff = sqrt(pow((pixels[y][x].red-	c.red),  2)+
							pow((pixels[y][x].green-c.green),2)+
							pow((pixels[y][x].blue-	c.blue), 2));
				//std::cout << buff << std::endl;
				if(buff < shit){
					shit = buff;
					buf[0] = y;
					buf[1] = x;
				}
				//
				//std::cout << shit << std::endl;
			}
		}
		ind.push_back({c, (int)buf[1], (int)buf[0]});
		//std::cout << float(buf[1]) << ", " << float(buf[0]) << std::endl;
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
