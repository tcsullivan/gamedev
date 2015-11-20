#ifndef TEXTURE_H
#define TEXTURE_H

#include <common.h>

#define DEBUG

namespace Texture{
	GLuint loadTexture(const char *fileName);
}

class Texturec{
private:
	GLuint *image;
	int texState;
public:
	Texturec(uint amt, ...);
	void bindNext();
	void bindPrev();
	void bind(int);
	void walk();
};

#endif //TEXTURE_H
