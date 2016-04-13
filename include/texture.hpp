/** @file Texture.h
 * @brief Defines a method for loading textures.
 *
 * This file gives facilities for easily loading and binding textures.
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#include <common.hpp>

/**
 * When defined, DEBUG allows extra messages to be printed to the terminal for
 * debugging purposes.
 */

#define DEBUG

/**
 * Texture functions are given a namespace for better organization.
 */

namespace Texture {

	/**
	 * Loads a texture from the given file name, returning the GLuint used for
	 * later referencing of the texture.
	 */

	GLuint loadTexture(std::string fileName);

	void freeTextures(void);

	void initColorIndex();
	vec2 getIndex(Color c);
	dim2 imageDim(std::string fileName);
}

/**
 * The Texturec class.
 *
 * This class can handle an array of textures and allows easy binding of those
 * textures.
 */

class Texturec{
private:

	/**
	 * Contains the index in the image array of the currently loaded texture.
	 */

	unsigned int texState;

public:

	/**
	 * Contains an array of the GLuints returned from Texture::loadTexture().
	 */

	std::vector<GLuint> image;

	/**
	 * Populates the image array from a list of strings, with each string as a
	 * separate argument.
	 */

	Texturec(uint amt, ...);

	/**
	 * Populates the image array from an array of strings.
	 */

	Texturec(uint amt,const char **paths);
	Texturec(std::vector<std::string>vec);
	Texturec(std::initializer_list<std::string> l);

	/**
	 * Frees memory taken by the image array.
	 */

	~Texturec();

	/**
	 * Binds the next texture in the array, incrementing texState.
	 */

	void bindNext();

	/**
	 * Binds the previous texture in the array, decrementing texState.
	 */

	void bindPrev();

	/**
	 * Binds the texture with the provided index.
	 */

	void bind(unsigned int);
};

#endif //TEXTURE_H
