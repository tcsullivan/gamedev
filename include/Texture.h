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
	void bind(int);

	GLuint *image;
	int texState;
private:

};

#endif //TEXTURE_H