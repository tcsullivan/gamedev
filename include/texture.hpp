/**
 * @file Texture.h
 * @brief Defines a method for loading textures.
 * This file gives facilities for easily loading and binding textures.
 */
#ifndef TEXTURE_HPP_
#define TEXTURE_HPP_

#include <common.hpp>

/**
 * When defined, DEBUG allows extra messages to be printed to the terminal for
 * debugging purposes.
 */
#define DEBUG

/**
 * @class Texture
 * Handles a single texture, loaded from the given file.
 */
class Texture {
private:
	std::string name; /**< The name (path) of the loaded file. */
	GLuint tex;       /**< The GLuint for the loaded texture. */
	vec2 dim;         /**< The dimensions of the loaded texture. */

public:
	/**
	 * Attempts to create a texture.
	 * Either:
	 * - Creates an empty class (no arguments given)
	 * - Load a texture from the given file, crashing if file not found
	 * - Fills the class with the given values
	 * @param file the path to the desired texture
	 * @param t the GLuint for the texture, if known
	 * @param v the size of the texture, if known
	 */
	Texture(const std::string& file = "", const GLuint& t = 0xFFFFF, const vec2& v = vec2(0, 0));

	/**
	 * Gets the name (path) of the loaded texture.
	 * @return the texture's name
	 */
	const std::string& getName(void) const;

	/**
	 * Gets the dimensions of the loaded texture.
	 * @return the texture's dimensions
	 */
	const vec2& getDim(void) const;

	/**
	 * Binds the texture, so it may be used for rendering.
	 */
	inline void use(void) const
	{ glBindTexture(GL_TEXTURE_2D, tex); }

	/**
	 * Frees GPU resources for the loaded texture.
	 */
	inline void destroy(void)
	{ glDeleteTextures(1, &tex), tex = 0xFFFFF; }

	/**
	 * Checks if a texture is currently loaded in this class.
	 * @return true if there is not a loaded texture
	 */
	inline bool isEmpty(void) const
	{ return (tex == 0xFFFFF); }
};

/**
 * @class ColorTex
 * Creates a single-pixel texture of the given color.
 */
class ColorTex : public Texture {
public:
	ColorTex(void);

	/**
	 * Creates a texture of the given color.
	 * @param color the desired color
	 */
	ColorTex(const Color& color);
};

/**
 * A collection of preloaded colors, to save resources.
 */
namespace Colors {
	extern ColorTex white; /**< A solid white texture. */
	extern ColorTex black; /**< A solid black texture. */
	extern ColorTex red;   /**< A solid red texture. */
	extern ColorTex blue;  /**< A solid blue texture. */

	/**
	 * Creates the colors.
	 */
	void init(void);
}

/**
 * @class TextureIterator
 * Keeps a collection of textures for easy usage/looping.
 */
class TextureIterator {
private:
	/**
	 * The set of textures to loop through.
	 */
	std::vector<Texture> textures;

	/**
	 * The current position in the texture array.
	 * @see textures
	 */
	std::vector<Texture>::iterator position;
public:
	TextureIterator(void)
		: position(std::begin(textures)) {}
	~TextureIterator(void) {}

	/**
	 * Constructs a set of textures from the given list.
	 * @param l the list of textures
	 */
	TextureIterator(const std::vector<std::string> &l);
	/**
	 * Shifts to the next texture in the array, stopping at the end if we're there.
	 * Also binds the texture.
	 */
	void operator++(int) noexcept;

	/**
	 * Shifts back in the array, staying at the beginning if we're there.
	 * Also binds the texture.
	 */
	void operator--(int) noexcept;
	/**
	 * Goes to the given index in the list.
	 * @param index the index to use
	 */
	void operator()(const int &index);
	/**
	 * Gets the dimensions of the currently selected texture.
	 * @return the texture's dimensions
	 */
	inline const vec2& getTextureDim(void)
	{ return position->getDim(); }
};

/**
 * Frees all loaded textures, rendering them all useless.
 */
void unloadTextures(void);

#endif //TEXTURE_HPP_
