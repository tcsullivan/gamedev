#include <algorithm>
#include <string>

#include <texture.h>

/**
 * A structure for keeping track of loaded textures.
 */

typedef struct {
	std::string name;	/**< The file path of the texture.		*/
	GLuint tex;			/**< The GLuint for the loaded texture. */
	dim2 dim;			/**< The dimensions of the texture.		*/
} texture_t;

struct index_t {
	Color color;
	int indexx;
	int indexy;
};

/**
 * A vector of all loaded textures.
 *
 * Should a texture be asked to be loaded twice, loadTexture() can reference
 * this array and reuse GLuint's to save memory.
 */

static std::vector<texture_t> LoadedTexture;

namespace Texture{
	Color pixels[8][4];

	GLuint loadTexture(std::string fileName){
		SDL_Surface *image;
		GLuint object = 0;

		// check if texture is already loaded
		for(auto &t : LoadedTexture){
			if(t.name == fileName){

#ifdef DEBUG
				DEBUG_printf("Reusing loaded texture for %s\n", fileName.c_str());
#endif // DEBUG

				return t.tex;
			}
		}

		// load SDL_surface of texture
		if(!(image = IMG_Load(fileName.c_str())))
			return 0;

#ifdef DEBUG
		DEBUG_printf("Loaded image file: %s\n", fileName.c_str());
#endif // DEBUG

		/*
		 * Load texture through OpenGL.
		 */

		glGenTextures(1,&object);				// Turns "object" into a texture
		glBindTexture(GL_TEXTURE_2D,object);	// Binds "object" to the top of the stack
		glPixelStoref(GL_UNPACK_ALIGNMENT,1);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	// Sets the "min" filter
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	// The the "max" filter of the stack

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

		// add texture to LoadedTexture
		LoadedTexture.push_back(texture_t{fileName,object,{image->w,image->h}});

		// free the SDL_Surface
		SDL_FreeSurface(image);

		return object;
	}

	dim2 imageDim(std::string fileName){
		for(auto &t : LoadedTexture){
			if(t.name == fileName)
				return t.dim;
		}
		return {0,0};
	}

	void freeTextures(void){
		while(!LoadedTexture.empty()){
			glDeleteTextures(1, &LoadedTexture.back().tex);
			LoadedTexture.pop_back();
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
	va_start(fNames, amt);
	for(unsigned int i = 0; i < amt; i++)
		image.push_back( Texture::loadTexture(va_arg(fNames, char *)) );
	va_end(fNames);
}

Texturec::Texturec( std::initializer_list<std::string> l )
{
	texState = 0;
	std::for_each( l.begin(), l.end(), [&](std::string s){ image.push_back( Texture::loadTexture( s ) ); });
}

Texturec::Texturec(std::vector<std::string>v){
	texState = 0;
	std::for_each( v.begin(), v.end(), [&](std::string s){ image.push_back( Texture::loadTexture( s ) ); });
}

Texturec::Texturec(uint amt,const char **paths){
	texState = 0;
	for(unsigned int i = 0; i < amt; i++)
		image.push_back( Texture::loadTexture(paths[i]) );
}

Texturec::~Texturec(){
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
