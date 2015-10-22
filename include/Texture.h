#ifndef TEXTURE_H
#define TEXTURE_H

#include <common.h>

namespace Texture{
	GLuint loadTexture(const char *fileName);
}

class Texturec{
public:
	Texturec(uint amt, ...);
	void bindNext();
	void bindPrev();

	GLuint *image;
private:
	int texState;

};

#endif //TEXTURE_H