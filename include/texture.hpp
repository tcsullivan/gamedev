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
 * DRAFT texture iterator?
 */
class TextureIterator {
private:
	std::vector<std::pair<GLuint, std::string>> textures;
	std::vector<std::pair<GLuint, std::string>>::iterator position;
public:
	TextureIterator(void) {
		position = std::begin(textures);
	}
	TextureIterator(const std::vector<std::string> &l) {
		for (const auto &s : l)
			textures.emplace_back(Texture::loadTexture(s), s);
		position = std::begin(textures);
	}
	void operator++(int) noexcept {
		if (++position < std::end(textures))
			glBindTexture(GL_TEXTURE_2D, (*position).first);
		else
			position = std::end(textures) - 1;
	}
	void operator--(int) noexcept {
		if (--position >= std::begin(textures))
			glBindTexture(GL_TEXTURE_2D, (*position).first);
		else
			position = std::begin(textures);
	}
	void operator()(const int &index) {
		if (index < 0 || index > static_cast<int>(textures.size()))
			throw std::invalid_argument("texture index out of range");

		position = std::begin(textures) + index;
		glBindTexture(GL_TEXTURE_2D, (*position).first);
	}
	const std::string& getTexturePath(const int &index) {
		if (index < 0 || index > static_cast<int>(textures.size()))
			throw std::invalid_argument("texture index out of range");

		return textures[index].second;
	}
};

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
	 * Contains the dimensions of each texture in the vector
	 */
	 //TODO
	//std::vector<vec2> imageDim;

	/**
	 * Stores the location of all the images
	 */
	std::vector<std::string> texLoc;

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
