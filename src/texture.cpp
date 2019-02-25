#include <texture.hpp>

#include <algorithm>
#include <string>
#include <stdexcept>

#include <SDL2/SDL_image.h>

#include <config.hpp>
#include <debug.hpp>
#include <error.hpp>
#include <gif_lib.h>

ObjectTexture::ObjectTexture(const std::string filename)
{
	valid = !filename.empty();
	if (!valid)
		return;

	auto image = IMG_Load(filename.c_str());

	// format: RGBA8
	solidMap.resize(image->w * image->h);
	auto rgba = ((uint8_t *)image->pixels) + 3;
	auto iter = solidMap.begin();
	for (int i = 0; i < image->w * image->h; i++) {
		*iter++ = *rgba > 0;
		rgba += 4;
	}

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

	name = filename;
	tex = object;
	dim = vec2 {(float)image->w, (float)image->h};

	// free the SDL_Surface
	SDL_FreeSurface(image);
}

int ObjectTexture::getHeight(int index)
{
	if (index < 0 || index > dim.x)
		return 100;

	unsigned int h;
	for (h = 0; h < dim.y; h++) {
		if (solidMap[h * dim.x + index])
			return dim.y - h;
	}
	return 0;
}

bool ObjectTexture::isInsideObject(vec2 coord) const {
	coord /= 2;
	return solidMap[(int)coord.y * (int)dim.x + (int)coord.x];
}

namespace Colors
{
	ColorTex white;
	ColorTex black;
	ColorTex red;
	ColorTex blue;

	GLfloat texCoord[12] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	void init(void) {
		white = ColorTex(Color(255, 255, 255));
		black = ColorTex(Color(0,   0,   0  ));
		red   = ColorTex(Color(255, 0,   0  ));
		blue  = ColorTex(Color(0,   0,   255));
	}
}

void loadTexture(const std::string& file, Texture& texture);

Texture::Texture(const std::string& file, const GLuint& t, const vec2& v)
	: name(file), tex(t), dim(v)
{
	if (t == 0xFFFFF && !file.empty())
		loadTexture(file, *this);
}

Texture::Texture(const std::string& file, bool hline)
{
	loadTexture(file, *this);
	if (hline)
		dim /= game::HLINE;
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
	glGenTextures(1, &object);				// Turns "object" into a texture
	glBindTexture(GL_TEXTURE_2D, object);	// Binds "object" to the top of the stack
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	tex = object;
	dim = vec2(1, 1);
}

static std::vector<Texture> loadedTextures;

void loadTexture(const std::string& file, Texture& texture)
{
	auto preloaded =
		std::find_if(std::begin(loadedTextures), std::end(loadedTextures),
		[&file](const Texture& t) { return (t.getName() == file); });

	if (preloaded == std::end(loadedTextures)) {
		auto image = IMG_Load(file.c_str());
		UserAssert(image != nullptr, "File not found: " + file);

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

void TextureIterator::appendGIF(const std::string& gif)
{
	int* error = nullptr;
	auto handle = DGifOpenFileName(gif.c_str(), error);
	UserAssert(handle != nullptr && error == nullptr, "Failed to load GIF: " + gif);
	UserAssert(DGifSlurp(handle) == GIF_OK, "Failed to extract from GIF: " + gif);

	for (int i = 0; i < handle->ImageCount; i++) {
		vec2 dim (handle->SavedImages[i].ImageDesc.Width,
			handle->SavedImages[i].ImageDesc.Height);
		int pcount = dim.x * dim.y;
		auto buf = new GLubyte[pcount * 4];
		auto bits = handle->SavedImages[i].RasterBits;
		auto map = handle->SColorMap->Colors;
		for (int j = 0; j < pcount; j++) {
			//if (bits[j * 4] == handle->SBackGroundColor) {
			auto c = map[bits[j]];
			if (c.Red == 0xFF && c.Green == 0xFF && c.Blue == 0xFF) {
				buf[j * 4] = buf[j * 4 + 1] = buf[j * 4 + 2] = buf[j * 4 + 3] = 0;
			} else {
				buf[j * 4 + 0] = c.Red;
				buf[j * 4 + 1] = c.Green;
				buf[j * 4 + 2] = c.Blue;
				buf[j * 4 + 3] = 0xFF;
			}
		}

		GLuint object;
		glGenTextures(1, &object);
		glBindTexture(GL_TEXTURE_2D, object);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dim.x, dim.y, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, buf);

		textures.emplace_back(gif, object, dim);
		delete[] buf;
	}
}

void unloadTextures(void)
{
	while (!loadedTextures.empty()) {
		loadedTextures.back().destroy();
		loadedTextures.pop_back();
	}
}

