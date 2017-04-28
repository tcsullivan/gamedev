#include <font.hpp>

FT_Library FontSystem::ftLibrary;
FT_Face FontSystem::ftFace;

std::string FontSystem::fontFamily;
std::map<int, std::vector<FT_Info>> FontSystem::fontData;

int FontSystem::currentSize = 0;
Color FontSystem::currentColor;
float FontSystem::currentZ = -8.0f;

void FontSystem::init(const std::string& ttf)
{
	FT_Init_FreeType(&ftLibrary);
	FT_New_Face(ftLibrary, ttf.c_str(), 0, &ftFace);
}

void FontSystem::setFontSize(int size)
{
	auto result = fontData.try_emplace(size, 93);
	if (result.second) {
		FT_Set_Pixel_Sizes(ftFace, 0, size);

		char c = 33;
		for (auto& d : fontData.at(size)) {
			glDeleteTextures(1, &d.tex);
			glGenTextures(1, &d.tex);    //	Generate new texture name/locations?

			// load the character from the font family file
			FT_Load_Char(ftFace, c++, FT_LOAD_RENDER);

			// transfer the character's bitmap (?) to a texture for rendering
			glBindTexture(GL_TEXTURE_2D, d.tex);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S , GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T , GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER , GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER , GL_LINEAR);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			/**
			 * The just-created texture will render red-on-black if we don't do anything to it, so
			 * here we create a buffer 4 times the size and transform the texture into an RGBA array,
			 * making it white-on-black.
			 */
			auto& g = ftFace->glyph;
			std::vector<uint32_t> buf (g->bitmap.width * g->bitmap.rows, 0xFFFFFFFF);
			for (auto j = buf.size(); j--;)
				buf[j] ^= !g->bitmap.buffer[j] ? buf[j] : 0;

			d.wh.x = g->bitmap.width;
			d.wh.y = g->bitmap.rows;
			d.bl.x = g->bitmap_left;
			d.bl.y = g->bitmap_top;
			d.ad.x = g->advance.x >> 6;
			d.ad.y = g->advance.y >> 6;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g->bitmap.width, g->bitmap.rows,
				0, GL_RGBA, GL_UNSIGNED_BYTE, buf.data());
		}
	}

	currentSize = size;
}

void FontSystem::setFontColor(float r, float g, float b)
{
	currentColor.red = r;
	currentColor.green = g;
	currentColor.blue = b;
}

void FontSystem::setFontZ(float z)
{
	currentZ = z;
}

vec2 FontSystem::putChar(float xx, float yy, char c)
{
	const auto& ch = fontData.at(currentSize)[c - 33];
	int x = xx, y = yy;

	// get dimensions of the rendered character
	vec2 c1 = {
		static_cast<float>(floor(x) + ch.bl.x),
		static_cast<float>(floor(y) + ch.bl.y)
	};
	const auto& c2 = ch.wh;

	Render::textShader.use();
	Render::textShader.enable();

	// draw the character
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ch.tex);

	glUniform4f(Render::textShader.uniform[WU_tex_color], 1.0f, 1.0f, 1.0f, 1.0f);

	GLfloat tex_coord[] = {
		0.0, 1.0, // bottom left
		1.0, 1.0, // bottom right
		1.0, 0.0, // top right
		1.0, 0.0, // top right
		0.0, 0.0, // top left
		0.0, 1.0, // bottom left
	};

	GLfloat text_vert[] = {
		c1.x,        c1.y - c2.y, currentZ,        // bottom left
		c1.x + c2.x, c1.y - c2.y, currentZ,        // bottom right
		c1.x + c2.x, c1.y + c2.y - c2.y, currentZ, // top right
		c1.x + c2.x, c1.y + c2.y - c2.y, currentZ, // top right
		c1.x,        c1.y + c2.y - c2.y, currentZ, // top left
		c1.x,        c1.y - c2.y, currentZ         // bottom left
	};

	glUniform4f(Render::textShader.uniform[WU_tex_color],
		currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);

	glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 0, text_vert);
	glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 0, tex_coord);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUniform4f(Render::textShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.0); 

	Render::textShader.disable();
	Render::textShader.unuse();

	// return the width.
	return ch.ad;
}

