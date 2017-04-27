#ifndef FONT_HPP_
#define FONT_HPP_

#include <vector>
#include <map>

#include <color.hpp>
#include <render.hpp>
#include <vector2.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

struct FT_Info {
	vec2 wh;
	vec2 bl;
	vec2 ad;
	GLuint tex;

	FT_Info(void)
		: tex(0) {}
};

class FontSystem {
private:
	static FT_Library ftLibrary;
	static FT_Face ftFace;

	static std::string fontFamily;
	static std::map<int, std::vector<FT_Info>> fontData;

	static int currentSize;
	static Color currentColor;
	static float currentZ;

public:
	~FontSystem(void) {
		FT_Done_Face(ftFace);
		FT_Done_FreeType(ftLibrary);
	}

	static void init(const std::string& ttf);
	static void setFontSize(int size);
	static void setFontColor(float r, float g, float b);
	static void setFontZ(float z = -8.0f);

	static vec2 putChar(float xx, float yy, char c);

	static inline int getSize(void)
	{ return currentSize; }

	static inline auto& getFont(void)
	{ return fontData.at(currentSize); }
};

#endif // FONT_HPP_
