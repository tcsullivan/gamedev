#include <font.hpp>

#include <cstring>

FT_Library FontSystem::ftLibrary;
FT_Face    FontSystem::ftFace;
std::string                             FontSystem::fontFamily;
std::map<int, std::vector<FT_Info>>     FontSystem::fontData;
std::vector<DrawData> FontSystem::drawData;
int   FontSystem::currentSize = 0;
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

			// convert red-on-black to RGBA
			auto& g = ftFace->glyph;
			std::vector<uint32_t> buf (g->bitmap.width * g->bitmap.rows, 0x00FFFFFF);
			for (auto j = buf.size(); j--;)
				buf[j] |= g->bitmap.buffer[j] << 24;
//				buf[j] ^= !g->bitmap.buffer[j] ? buf[j] : 0;

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

vec2 FontSystem::putChar(float x, float y, char c)
{
	const auto& ch = fontData.at(currentSize)[c - 33];

	vec2 c1 (static_cast<float>(floor(x) + ch.bl.x), static_cast<float>(floor(y) + ch.bl.y));
	vec2 c2 (c1.x + ch.wh.x, c1.y - ch.wh.y);

	drawData.push_back( DrawData {
		ch.tex,
		currentColor,
		{
			c1.x, c1.y, currentZ, 0, 0,
			c2.x, c1.y, currentZ, 1, 0,
			c2.x, c2.y, currentZ, 1, 1,
			c2.x, c2.y, currentZ, 1, 1,
			c1.x, c2.y, currentZ, 0, 1,
			c1.x, c1.y, currentZ, 0, 0,
		}
	});

	return ch.ad;
}

void FontSystem::render(void)
{
	Render::textShader.use();
	Render::textShader.enable();

	for (const auto& d : drawData) {
		glUniform4f(Render::textShader.uniform[WU_tex_color],
			d.color.red, d.color.green, d.color.blue, d.color.alpha);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, d.tex);
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), d.verts);
		glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), d.verts + 3);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	Render::textShader.disable();
	Render::textShader.unuse();
	drawData.clear();
}
