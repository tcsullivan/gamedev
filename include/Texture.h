#ifndef TEXTURE_H
#define TEXTURE_H

#include <common.h>

#define DEBUG

namespace Texture{
	GLuint loadTexture(const char *fileName);
}

class Texturec{
private:
	unsigned int texState;
public:
	GLuint *image;
	
	Texturec(uint amt, ...);
	~Texturec();
	
	void bindNext();
	void bindPrev();
	void bind(unsigned int);
	void walk();
};

#endif //TEXTURE_H
