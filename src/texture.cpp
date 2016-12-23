#include <algorithm>
#include <string>

#include <texture.hpp>

namespace Colors
{
	ColorTex white;
	ColorTex black;
	ColorTex red;

	void init(void) {
		white = ColorTex(Color(255, 255, 255));
		black = ColorTex(Color(0, 0, 0));
		red = ColorTex(Color(255, 0, 0));
	}
}

void loadTexture(const std::string& file, Texture& texture);

Texture::Texture(const std::string& file, const GLuint& t, const vec2& v)
	: name(file), tex(t), dim(v)
{
	if (t == 0xFFFFF && !file.empty())
		loadTexture(file, *this);
}

const std::string& Texture::getName(void) const
{
	return name;
}

const vec2& Texture::getDim(void) const
{
	return dim;
}

ColorTex::ColorTex(void)
{
	Texture();
}

ColorTex::ColorTex(const Color& color)
{
	unsigned char data[4] = {
		static_cast<unsigned char>(color.red),
		static_cast<unsigned char>(color.green),
		static_cast<unsigned char>(color.blue),
		static_cast<unsigned char>(color.alpha),
	};

	GLuint object;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &object);				// Turns "object" into a texture
	glBindTexture(GL_TEXTURE_2D, object);	// Binds "object" to the top of the stack
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	Texture("", object, vec2());
}

static std::vector<Texture> loadedTextures;

void loadTexture(const std::string& file, Texture& texture)
{
	auto preloaded =
		std::find_if(std::begin(loadedTextures), std::end(loadedTextures),
		[&file](const Texture& t) { return (t.getName() == file); });

	if (preloaded == std::end(loadedTextures)) {
		auto image = IMG_Load(file.c_str());
		if (image == nullptr)
			UserError("File not found: " + file);

#ifdef DEBUG
		DEBUG_printf("Loaded image file: %s\n", file.c_str());
#endif // DEBUG

		// load texture through OpenGL
		GLuint object;
		glGenTextures(1, &object);				// Turns "object" into a texture
		glBindTexture(GL_TEXTURE_2D, object);	// Binds "object" to the top of the stack
		glPixelStoref(GL_UNPACK_ALIGNMENT, 1);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	// Sets the "min" filter
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	// The the "max" filter of the stack
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Wrap the texture to the matrix
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); //
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0,
					 GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);

		// add texture to loadedTextures
		loadedTextures.emplace_back(file, object, vec2(image->w, image->h));

		texture = loadedTextures.back();

		// free the SDL_Surface
		SDL_FreeSurface(image);
	} else {
		texture = *preloaded;
	}
}


TextureIterator::TextureIterator(const std::vector<std::string> &l)
{
	for (const auto& s : l)
		textures.emplace_back(s);
	position = std::begin(textures);
}

void TextureIterator::operator++(int) noexcept
{
	if (++position >= std::end(textures))
		position = std::end(textures) - 1;
	position->use();
}

void TextureIterator::operator--(int) noexcept
{
	if (--position < std::begin(textures))
		position = std::begin(textures);
	position->use();
}

void TextureIterator::operator()(const int &index)
{
	if (index < 0 || index > static_cast<int>(textures.size()))
		throw std::invalid_argument("texture index out of range");

	position = std::begin(textures) + index;
	position->use();
}


void unloadTextures(void)
{
	while (!loadedTextures.empty()) {
		loadedTextures.back().destroy();
		loadedTextures.pop_back();
	}
}

