#include <ui.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <bmpimage.hpp>
#include <debug.hpp>
#include <error.hpp>
#include <ui_menu.hpp>
#include <vector3.hpp>

#include <ui_quest.hpp>
#include <brice.hpp>
#include <world.hpp>
#include <gametime.hpp>

#include <render.hpp>
#include <engine.hpp>
#include <events.hpp>
#include <window.hpp>
#include <player.hpp>

#include <chrono>

using namespace std::literals::chrono_literals;

extern Menu *currentMenu;

std::array<SDL_Keycode, 6> controlMap = {
	SDLK_w, SDLK_a, SDLK_d, SDLK_LSHIFT, SDLK_LCTRL, SDLK_e
};

void setControl(int index, SDL_Keycode key)
{
	controlMap[index] = key;
}

SDL_Keycode getControl(int index)
{
	if (index >= static_cast<int>(controlMap.size()))
		return 0;

	return controlMap[index];
}

/**
 *	Freetype variables
 */

static FT_Library   ftl;
static FT_Face      ftf;

struct FT_Info {
	vec2 wh;
	vec2 bl;
	vec2 ad;
	GLuint tex;

	FT_Info(void)
		: tex(0) {}
};

static std::vector<FT_Info> ftData (93);

static Color fontColor (255, 255, 255);

/*
 *	Variables for dialog boxes / options.
 */

static std::vector<std::pair<vec2, std::string>> textToDraw;

static std::vector<std::pair<std::string,vec3>> dialogOptText;
static std::string dialogBoxText;
static bool typeOutDone = true;
static bool typeOutSustain = false;

static Mix_Chunk *dialogClick;

/*
 * Fade effect flags
 */

static bool fadeWhite  = false;
static bool fadeFast   = false;

bool inBattle = false;
Mix_Chunk *battleStart;

Mix_Chunk *sanic;

static GLuint pageTex = 0;
static bool   pageTexReady = false;

void loadFontSize(int size, std::vector<FT_Info> &data)
{
	FT_Set_Pixel_Sizes(ftf, 0, size);

	// pre-render 'all' the characters
	for (auto& d : data) {
		glDeleteTextures(1, &d.tex);
		glGenTextures(1, &d.tex);    //	Generate new texture name/locations?
	}

	for (char i = 33; i < 126; i++) {
		// load the character from the font family file
		if (FT_Load_Char(ftf, i, FT_LOAD_RENDER))
			UserError("Error! Unsupported character " + i);

		// transfer the character's bitmap (?) to a texture for rendering
		glBindTexture(GL_TEXTURE_2D, data[i - 33].tex);
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
		auto& g = ftf->glyph;
		std::vector<uint32_t> buf (g->bitmap.width * g->bitmap.rows, 0xFFFFFFFF);
		for (auto j = buf.size(); j--;)
			buf[j] ^= !g->bitmap.buffer[j] ? buf[j] : 0;

		auto& d = data[i - 33];
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

namespace ui {

	bool fadeEnable = false;
	int fadeIntensity = 0;

	/*
	 *	Mouse coordinates.
	*/

	vec2 mouse;
	vec2 premouse={0,0};

	/*
	 *	Variety of keydown bools
	*/
	bool edown;

	/*
	 *	Debugging flags.
	*/

	bool debug=false;
	bool posFlag=false;
	bool dialogPassive = false;
	bool dialogMerchant = false;
	int dialogPassiveTime = 0;

	int fontTransInv = 255;

	/*
	 *	Dialog stuff that needs to be 'public'.
	*/

	bool dialogBoxExists = false;
	bool dialogImportant = false;
	unsigned char dialogOptChosen = 0;

	unsigned int textWrapLimit = 72;

	/*
	 *	Current font size. Changing this WILL NOT change the font size, see setFontSize() for
	 *	actual font size changing.
	*/

	unsigned int fontSize;
	float fontZ = -8.0;

    void takeScreenshot(GLubyte* pixels);

	/*
	 *	Initialises the Freetype library, and sets a font size.
	*/

	void initFonts(void) {
		if (FT_Init_FreeType(&ftl))
			UserError("Couldn't initialize freetype.");

#ifdef DEBUG
		DEBUG_printf("Initialized FreeType2.\n", nullptr);
#endif // DEBUG

		fontSize = 0;
	}

	void initSounds(void) {
		dialogClick = Mix_LoadWAV("assets/sounds/click.wav");
		battleStart = Mix_LoadWAV("assets/sounds/frig.wav");
		sanic = Mix_LoadWAV("assets/sounds/sanic.wav");
	}

	void destroyFonts(void) {
		FT_Done_Face(ftf);
		FT_Done_FreeType(ftl);

		Mix_FreeChunk(dialogClick);
		Mix_FreeChunk(battleStart);
		Mix_FreeChunk(sanic);
	}

	/*
	 *	Sets a new font family to use (*.ttf).
	*/

	void setFontFace(const char *ttf) {
		if (FT_New_Face(ftl, ttf, 0, &ftf))
			UserError("Error! Couldn't open " + (std::string)ttf + ".");

#ifdef DEBUG
		DEBUG_printf("Using font %s\n",ttf);
#endif // DEBUG
	}

	/*
	 *	Sets a new font size (default: 12).
	*/

	void setFontSize(unsigned int size) {
		fontSize = size;
		loadFontSize(size, ftData);
	}

	/*
	 *	Set a color for font rendering (default: white).
	 */
	void setFontColor(int r, int g, int b, int a = 255) {
		fontColor = Color(r, g, b, a);
	}

	/*
 	 *	Set the font's z layer
 	 */
	void setFontZ(float z) {
		fontZ = z;
	}

	/*
	 *	Draws a character at the specified coordinates, aborting if the character is unknown.
	 */
	vec2 putChar(float xx,float yy,char c){
		const auto& ch = ftData[c - 33];
		int x = xx, y = yy;

		// get dimensions of the rendered character
		vec2 c1 = {
			static_cast<float>(floor(x) + ch.bl.x),
			static_cast<float>(floor(y) + ch.bl.y)
		};

		const auto& c2 = ch.wh;

		// draw the character
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ch.tex);

		Render::textShader.use();
		Render::textShader.enable();

		glUniform4f(Render::textShader.uniform[WU_tex_color], 1.0f, 1.0f, 1.0f, 1.0f);

		GLfloat tex_coord[] = {
			0.0, 1.0,				//bottom left
			1.0, 1.0,				//bottom right
			1.0, 0.0,				//top right
			1.0, 0.0,				//top right
			0.0, 0.0,				//top left
			0.0, 1.0,				//bottom left
		};

		GLfloat text_vert[] = {
			c1.x, 		c1.y     -c2.y, fontZ,	//bottom left
			c1.x+c2.x, 	c1.y	 -c2.y, fontZ, 	//bottom right
			c1.x+c2.x, 	c1.y+c2.y-c2.y, fontZ,	//top right
			c1.x+c2.x, 	c1.y+c2.y-c2.y, fontZ,	//top right
			c1.x, 		c1.y+c2.y-c2.y, fontZ,	//top left
			c1.x, 		c1.y	 -c2.y, fontZ	//bottom left
		};

        glUniform4f(Render::textShader.uniform[WU_tex_color],
                    static_cast<float>(fontColor.red / 255),
                    static_cast<float>(fontColor.green / 255),
                    static_cast<float>(fontColor.blue / 255),
                    static_cast<float>(fontColor.alpha / 255));

		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 0, text_vert);
		glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 0, tex_coord);
		glDrawArrays(GL_TRIANGLES, 0, 6);

        glUniform4f(Render::textShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.0); // TODO seg faults

		Render::textShader.disable();
		Render::textShader.unuse();

		// return the width.
		return ch.ad;
	}

	/*
	 *	Draw a string at the specified coordinates.
	*/

	float putString(const float x, const float y, std::string s) {
		unsigned int i = 0, nl = 1;
		vec2 add, o = {x, y};

		// loop on each character
		do {
			if (dialogBoxExists && i > textWrapLimit * nl) {
 				o.y -= fontSize * 1.05f;
 				o.x = x;
				++nl;

				// skip a space if it's there since we just newline'd
  				if (s[i] == ' ')
  					i++;
  			}

			switch (s[i]) {
			case '\r':
			case '\t':
				break;
			case '\n':
				o.y -= fontSize * 1.05f;
				o.x = x;
				break;
			case '\b':
				o.x -= add.x;
				break;
			case ' ':
				o.x += fontSize / 2;
				break;
			default:
				add = putChar(floor(o.x), floor(o.y), s[i]);
				o.x += add.x;
				o.y += add.y;
				break;
			}
		} while (s[++i]);

		return o.x;	// the string width
	}

	float putStringCentered(const float x, const float y, std::string s) {
		unsigned int i = 0, lastnl = 0;
		float width = 0, yy = y;

		do {
			switch (s[i]) {
			case '\n':
				putString(floor(x - width / 2), yy, s.substr(0, i));
				lastnl = 1 + i;
				width = 0;
				yy -= fontSize * 1.15f;
				break;
			case '\b':
				break;
			case ' ':
				width += fontSize / 2;
				break;
			default:
				width += ftData[i].wh.x + fontSize * 0.1f;
				break;
			}
		} while(s[++i]);

		putString(floor(x - width / 2), yy, s.substr(lastnl));
		return width;
	}

	/**
 	 * Prevents typeOut from typing the next string it's given.
	 */

	 void dontTypeOut(void) {
		 typeOutSustain = true;
	 }

	/*
	 *	Draw a string in a typewriter-esque fashion. Each letter is rendered as calls are made
	 *	to this function. Passing a different string to the function will reset the counters.
	*/

	std::string ret;
	std::string typeOut(std::string str) {
		static unsigned int tadv = 1;
		static unsigned int tickk,
							linc=0,	//	Contains the number of letters that should be drawn.
							size=0;	//	Contains the full size of the current string.

		auto tickCount = game::time::getTickCount();

		// reset values if a new string is being passed.
		if (!linc || ret.substr(0, linc) != str.substr(0, linc)) {
			tickk = tickCount + tadv;
			ret  = str.substr(0, 1);
			size = str.size();			//	Set the new target string size
			linc = 1;					//	Reset the incrementers
			if ((typeOutDone = typeOutSustain))
				typeOutSustain = false;
		}

		if (typeOutDone)
			return str;

		// Draw the next letter if necessary.
		else if (tickk <= tickCount) {
			tickk = tickCount + tadv;
			ret += str[linc];

			if (linc < size) {
				switch (str[++linc]) {
				case '!':
				case '?':
				case '.':
				case ':':
					tadv = 10;
					break;
				case ',':
				case ';':
					tadv = 5;
				break;
				default:
					tadv = 1;
					break;
				}
			} else {
				typeOutDone = true;
			}
		}

		return ret;		//	The buffered string.
	}

	/*
	 *	Draw a formatted string to the specified coordinates.
	*/

	std::string uisprintf(const char *s, va_list args) {
		std::unique_ptr<char[]> buf (new char[512]);
		vsnprintf(buf.get(), 512, s, args);
		std::string ret (buf.get());
		return ret;
	}

	float putText(const float x, const float y, const char *str, ...) {
		va_list args;
	
		va_start(args,str);
		auto s = uisprintf(str, args);
		va_end(args);

		// draw the string and return the width
		return putString(x, y, s);
	}

	void putTextL(vec2 c, const char *str, ...) {
		va_list args;

		va_start(args, str);
		auto s = uisprintf(str, args);
		va_end(args);

		textToDraw.push_back(std::make_pair(c, s));
	}

	void dialogBox(std::string name, std::string opt, bool passive, std::string text, ...) {
		va_list args;

		dialogPassive = passive;

		// add speaker prefix
		dialogBoxText = name + ": ";

		// handle the formatted string
		va_start(args, text);
		auto s = uisprintf(text.c_str(), args);
		va_end(args);

		dialogBoxText += s;

		// setup option text
		dialogOptText.clear();
		dialogOptChosen = 0;

		if (!opt.empty()) {
			char *sopt = strtok(&opt[0], ":");

			// cycle through options
			while (sopt) {
				dialogOptText.push_back(std::make_pair((std::string)sopt, vec3 {0,0,0}));
				sopt = strtok(nullptr, ":");
			}
		}

		// tell draw() that the box is ready
		dialogBoxExists = true;
		dialogImportant = false;

		ret.clear();
	}

	/**
	 * Wait for a dialog box to be dismissed.
	 */

	void waitForDialog(void) {
		while (dialogBoxExists)
			std::this_thread::sleep_for(1ms);
	}

	void waitForCover(void) {
		auto& fi = fadeIntensity;
		fi = 0;

		while (fi < 255)
			std::this_thread::sleep_for(1ms);
			
		fi = 255;
	}

	void waitForUncover(void) {
		fadeIntensity = 255;
		while (fadeIntensity > 0);
		fadeIntensity = 0;
	}

	void importantText(const char *text, ...) {
		va_list args;

		dialogBoxText.clear();

		va_start(args, text);
		auto s = uisprintf(text, args);
		va_end(args);

		dialogBoxText = s;

		dialogBoxExists = true;
		dialogImportant = true;
		dialogPassive = false;
		dialogPassiveTime = 0;
	}

	void passiveImportantText(int duration, const char *text, ...) {
		va_list args;

		dialogBoxText.clear();

		va_start(args, text);
		auto s = uisprintf(text, args);
		va_end(args);

		dialogBoxText = s;

		dialogBoxExists = true;
		dialogImportant = true;
		dialogPassive = true;
		dialogPassiveTime = duration;
	}


	void drawPage(const GLuint& tex) {
		pageTex = tex;
		pageTexReady = true;
	}

	void drawBox(vec2 c1, vec2 c2) {
        GLfloat box[] = {c1.x, c1.y, -7.0,
                         c2.x, c1.y, -7.0,
                         c2.x, c2.y, -7.0,

                         c2.x, c2.y, -7.0,
                         c1.x, c2.y, -7.0,
                         c1.x, c1.y, -7.0};

        GLfloat line_strip[] = {c1.x,     c1.y, -7.1,
                                c2.x + 1, c1.y, -7.1,
                                c2.x + 1, c2.y, -7.1,
                                c1.x,     c2.y, -7.1,
                                c1.x,     c1.y, -7.1};

        GLfloat box_tex[] = {0,0,
                             1,0,
                             1,1,

                             1,1,
                             0,1,
                             0,0};

        glActiveTexture(GL_TEXTURE0);
		Colors::black.use();
        glUniform1i(Render::textShader.uniform[WU_texture], 0);

        Render::textShader.use();
		Render::textShader.enable();

        glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 0, box);
        glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 0, box_tex);
        glDrawArrays(GL_TRIANGLES, 0 ,6);

        Colors::white.use();
        glUniform1i(Render::textShader.uniform[WU_texture], 0);

        glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 0, line_strip);
        glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 0, box_tex);
        glDrawArrays(GL_LINE_STRIP, 0 ,8);

        Render::textShader.disable();
		Render::textShader.unuse();
	}

	void drawNiceBox(vec2 c1, vec2 c2, float z) {
		drawNiceBoxColor(c1, c2, z, Color(1.0f, 1.0f, 1.0f));
	}

	void drawNiceBoxColor(vec2 c1, vec2 c2, float z, Color c) {
		// the textures for the box corners
		static Texture boxCorner  ("assets/ui/button_corners.png");
		static Texture boxSideTop ("assets/ui/button_top_bot_borders.png");
		static Texture boxSide    ("assets/ui/button_side_borders.png");

		// the dimensions of the corner textures
		static const auto& boxCornerDim  = boxCorner.getDim();
		static const auto& boxCornerDim2 = boxCornerDim / 2;

		// the amount of bytes to skip in the OpenGL arrays (see below)
		auto stride = 5 * sizeof(GLfloat);

		// we always want to make sure c1 is lower left and c2 is upper right
		if (c1.x > c2.x) std::swap(c1.x, c2.x);
		if (c1.y > c2.y) std::swap(c1.y, c2.y);

		// if the box is too small, we will not be able to draw it
		if (c2.x - c1.x < (boxCornerDim.x) || c2.y - c1.y < (boxCornerDim.y))
			return;

		GLfloat box_ul[] = {c1.x, 						c2.y - boxCornerDim2.y, z,	0.0f, 0.5f,
                          	c1.x + boxCornerDim2.x, 	c2.y - boxCornerDim2.y, z,	0.5f, 0.5f,
                          	c1.x + boxCornerDim2.x,	c2.y, 				    z,	0.5f, 1.0f,

                          	c1.x + boxCornerDim2.x,	c2.y, 					z, 0.5f, 1.0f,
                       	  	c1.x, 						c2.y, 					z, 0.0f, 1.0f,
                          	c1.x, 						c2.y - boxCornerDim2.y, z,	0.0f, 0.5f};

		GLfloat box_ll[] = {c1.x, 						c1.y, 					z,	0.0f, 0.0f,
                          	c1.x + boxCornerDim2.x, 	c1.y,					z,	0.5f, 0.0f,
                          	c1.x + boxCornerDim2.x,	c1.y + boxCornerDim2.y, z,	0.5f, 0.5f,

                          	c1.x + boxCornerDim2.x,	c1.y + boxCornerDim2.y, z, 0.5f, 0.5f,
                       	  	c1.x, 						c1.y + boxCornerDim2.y, z, 0.0f, 0.5f,
                          	c1.x, 						c1.y,					z, 0.0f, 0.0f};

		GLfloat box_ur[] = {c2.x - boxCornerDim2.x,	c2.y - boxCornerDim2.y, z,	0.5f, 0.5f,
                          	c2.x, 						c2.y - boxCornerDim2.y, z,	1.0f, 0.5f,
                          	c2.x,						c2.y, 					 z,	1.0f, 1.0f,

                          	c2.x,						c2.y, 					 z, 1.0f, 1.0f,
                       	  	c2.x - boxCornerDim2.x, 	c2.y, 					 z, 0.5f, 1.0f,
                          	c2.x - boxCornerDim2.x, 	c2.y - boxCornerDim2.y, z,	0.5f, 0.5f};

		GLfloat box_lr[] = {c2.x - boxCornerDim2.x, 	c1.y, 					 z,	0.5f, 0.0f,
                          	c2.x, 						c1.y,					 z,	1.0f, 0.0f,
                          	c2.x,						c1.y + boxCornerDim2.y, z,	1.0f, 0.5f,

                          	c2.x,						c1.y + boxCornerDim2.y, z, 1.0f, 0.5f,
                       	  	c2.x - boxCornerDim2.x, 	c1.y + boxCornerDim2.y, z, 0.5f, 0.5f,
                          	c2.x - boxCornerDim2.x, 	c1.y,					 z,	0.5f, 0.0f};

		GLfloat box_l[] =  {c1.x,						c1.y + boxCornerDim2.y, z, 0.0f, 0.0f,
							c1.x + boxCornerDim2.x,	c1.y + boxCornerDim2.y, z, 0.5f, 0.0f,
							c1.x + boxCornerDim2.x,	c2.y - boxCornerDim2.y, z, 0.5f, 1.0f,

							c1.x + boxCornerDim2.x,	c2.y - boxCornerDim2.y, z, 0.5f, 1.0f,
							c1.x,						c2.y - boxCornerDim2.y, z, 0.0f, 1.0f,
							c1.x,						c1.y + boxCornerDim2.y, z, 0.0f, 0.0f};

		GLfloat box_r[] =  {c2.x - boxCornerDim2.x,	c1.y + boxCornerDim2.y, z, 0.5f, 0.0f,
							c2.x,						c1.y + boxCornerDim2.y, z, 1.0f, 0.0f,
							c2.x,						c2.y - boxCornerDim2.y, z, 1.0f, 1.0f,

							c2.x,						c2.y - boxCornerDim2.y, z, 1.0f, 1.0f,
							c2.x - boxCornerDim2.x,	c2.y - boxCornerDim2.y, z, 0.5f, 1.0f,
							c2.x - boxCornerDim2.x,	c1.y + boxCornerDim2.y, z, 0.5f, 0.0f};

        GLfloat box_b[] =  {c1.x + boxCornerDim2.x,	c1.y,					 z,	0.0f, 0.0f,
							c2.x - boxCornerDim2.x,	c1.y,					 z, 1.0f, 0.0f,
							c2.x - boxCornerDim2.x,	c1.y + boxCornerDim2.y, z,	1.0f, 0.5f,

							c2.x - boxCornerDim2.x,	c1.y + boxCornerDim2.y, z,	1.0f, 0.5f,
							c1.x + boxCornerDim2.x,	c1.y + boxCornerDim2.y, z, 0.0f, 0.5f,
							c1.x + boxCornerDim2.x,	c1.y,					 z, 0.0f, 0.0f};

        GLfloat box_t[] =  {c1.x + boxCornerDim2.x,	c2.y - boxCornerDim2.y, z,	0.0f, 0.5f,
							c2.x - boxCornerDim2.x,	c2.y - boxCornerDim2.y, z, 1.0f, 0.5f,
							c2.x - boxCornerDim2.x,	c2.y, 					 z,	1.0f, 1.0f,

							c2.x - boxCornerDim2.x,	c2.y,					 z,	1.0f, 1.0f,
							c1.x + boxCornerDim2.x,	c2.y,					 z, 0.0f, 1.0f,
							c1.x + boxCornerDim2.x,	c2.y - boxCornerDim2.y, z, 0.0f, 0.5f};

		GLfloat box_f[] =  {c1.x + boxCornerDim2.x,	c1.y + boxCornerDim2.y, z, 0.5f, 0.5f,
							c2.x - boxCornerDim2.x,	c1.y + boxCornerDim2.y, z, 0.5f, 0.5f,
							c2.x - boxCornerDim2.x,	c2.y - boxCornerDim2.y, z, 0.5f, 0.5f,

							c2.x - boxCornerDim2.x,	c2.y - boxCornerDim2.y, z, 0.5f, 0.5f,
							c1.x + boxCornerDim2.x,	c2.y - boxCornerDim2.y, z, 0.5f, 0.5f,
							c1.x + boxCornerDim2.x,	c1.y + boxCornerDim2.y, z, 0.5f, 0.5f};

		glActiveTexture(GL_TEXTURE0);
		glUniform1f(Render::textShader.uniform[WU_texture], 0);

		Render::textShader.use();
		Render::textShader.enable();

		glUniform4f(Render::textShader.uniform[WU_tex_color], c.red, c.green, c.blue, c.alpha);
		boxCorner.use();

		// draw upper left corner
        glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_ul[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_ul[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		// lower left corner
        glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_ll[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_ll[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		// upper right corner
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_ur[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_ur[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		// lower right corner
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_lr[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_lr[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		// draw the middle of the box
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_f[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_f[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		boxSide.use();

		// draw the left edge of the box
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_l[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_l[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		// draw right edge of the box
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_r[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_r[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		boxSideTop.use();

		// draw bottom of the box
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_b[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_b[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		// draw top of the box
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, stride, &box_t[0]);
        glVertexAttribPointer(Render::textShader.tex,   2, GL_FLOAT, GL_FALSE, stride, &box_t[3]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        Render::textShader.disable();
		Render::textShader.unuse();
	}

	void draw(void){
		unsigned char i;
		std::string rtext;

		auto SCREEN_HEIGHT = static_cast<float>(game::SCREEN_HEIGHT);

		// will return if not toggled
		quest::draw();

		if (pageTexReady) {

            GLfloat page_loc[] = {offset.x - 300, SCREEN_HEIGHT - 100, -6.0,
                                  offset.x + 300, SCREEN_HEIGHT - 100, -6.0,
                                  offset.x + 300, SCREEN_HEIGHT - 600, -6.0,

                                  offset.x + 300, SCREEN_HEIGHT - 600, -6.0,
                                  offset.x - 300, SCREEN_HEIGHT - 600, -6.0,
                                  offset.x - 300, SCREEN_HEIGHT - 100, -6.0};

            static const GLfloat page_tex[] = {
				0.0, 0.0,
                1.0, 0.0,
                1.0, 1.0,
	            1.0, 1.0,
                0.0, 1.0,
                0.0, 0.0
			};

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, pageTex);
            glUniform1i(Render::textShader.uniform[WU_texture], 0);

			Render::textShader.use();
			Render::textShader.enable();

            glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 0, page_loc);
            glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 0, page_tex);
            glDrawArrays(GL_TRIANGLES, 0 ,6);

            Render::textShader.disable();
			Render::textShader.unuse();

		} else if (dialogBoxExists) {
			rtext = typeOut(dialogBoxText);

			if (dialogImportant) {
				setFontColor(255, 255, 255);
				if (dialogPassive) {
					dialogPassiveTime -= game::time::getDeltaTime() * 12;
					if (dialogPassiveTime < 0) {
						dialogPassive = false;
						dialogImportant = false;
						dialogBoxExists = false;
					}
				}

				if (fadeIntensity == 255 || dialogPassive) {
					setFontSize(24);
					putStringCentered(offset.x,offset.y,rtext);
					setFontSize(16);
				}
			} else { //normal dialog box

				float y = offset.y + SCREEN_HEIGHT / 2 - HLINES(8);
				float x = offset.x - 300;

				drawNiceBox(vec2 {x, y}, vec2 {x + 600, y - SCREEN_HEIGHT / 4}, -7.0);

				setFontZ(-7.2f);
				rtext = typeOut(dialogBoxText);
				putString(x + HLINES(2), y - fontSize - game::HLINE, rtext);

				for (i = 0; i < dialogOptText.size(); i++) {
					auto& sec = dialogOptText[i].second;

					setFontColor(255, 255, 255);
					auto tmp = putStringCentered(offset.x, sec.y,dialogOptText[i].first);
					sec.z = offset.x + tmp;
					sec.x = offset.x - tmp;
					sec.y = y - SCREEN_HEIGHT / 4 + (fontSize + game::HLINE) * (i + 1);
					if (mouse.x > sec.x && mouse.x < sec.z &&
					    mouse.y > sec.y && mouse.y < sec.y + 16) { // fontSize
						  setFontColor(255, 255, 0);
						  putStringCentered(offset.x, sec.y, dialogOptText[i].first);
					}
				}

				setFontColor(255, 255, 255);
			}

			static unsigned int rtext_oldsize = 0;
			if (rtext_oldsize != rtext.size()) {
				if (!isspace(rtext[(rtext_oldsize = rtext.size()) - 1]))
					Mix_PlayChannel(0, dialogClick, 0);

			}

		} else {
			for (const auto &s : textToDraw)
				putString(s.first.x, s.first.y, s.second);
		}

		if (!fadeIntensity) {
			/*vec2 hub = {
				(SCREEN_WIDTH/2+offset.x)-fontSize*10,
				(offset.y+SCREEN_HEIGHT/2)-fontSize
			};*/

			/*putText(hub.x,hub.y,"Health: %u/%u",player->health>0?(unsigned)player->health:0,
												(unsigned)player->maxHealth
												);*/
			/*static GLuint frontHealth = Texture::genColor(Color(255,0,0));
			static GLuint backHealth = Texture::genColor(Color(150,0,0));

			if (player->isAlive()) {
				hub.y-=fontSize*1.15;

                GLfloat tex[] = {0.0, 0.0,
                                 1.0, 0.0,
                                 0.0, 1.0,
                                 1.0, 1.0};

                GLfloat back[] = {hub.x,        hub.y,      -7.0,
                                  hub.x + 150,  hub.y,      -7.0,
                                  hub.x,        hub.y + 12, -7.0,
                                  hub.x + 150,  hub.y + 12, -7.0};

                GLfloat front[] = {hub.x,       hub.y,      -7.1,
                                   hub.x + 150, hub.y,      -7.1,
                                   hub.x,       hub.y + 12, -7.1,
                                   hub.x + 150, hub.y + 12, -7.1};


                glUniform1i(Render::textShader.uniform[WU_texture], 0);

				Render::textShader.use();
				Render::textShader.enable();

                glBindTexture(GL_TEXTURE_2D, frontHealth);

                glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 0, front);
                glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 0, tex);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                glBindTexture(GL_TEXTURE_2D, backHealth);

                glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 0, back);
                glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 0, tex);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                Render::textShader.disable();
				Render::textShader.unuse();
            }*/

			/*
			 *	Lists all of the quests the player is currently taking.
			*/

			/*setFontColor(255,255,255,fontTransInv);
			if (player->inv->invOpen) {
				hub.y = player->loc.y + fontSize * 8;
				hub.x = player->loc.x;// + player->width / 2;

				putStringCentered(hub.x,hub.y,"Current Quests:");

				for(auto &c : player->qh.current) {
					hub.y -= fontSize * 1.15;
					putStringCentered(hub.x,hub.y,c.title);
				}

				hub.y = offset.y + 40*1.2;
				hub.x = offset.x + SCREEN_WIDTH/2 - 40*1.5;

				putStringCentered(hub.x,hub.y,"Equipment:");

				hub.y = offset.y + SCREEN_HEIGHT/2 - 20;
				hub.x = offset.x - SCREEN_WIDTH/2 + 45*4*1.5;

				putStringCentered(hub.x,hub.y,"Inventory:");
			}*/
			setFontColor(255,255,255,255);
		}

		menu::draw();

		// draw the mouse
		static const Texture mouseTex ("assets/goodmouse.png");
		Render::textShader.use();
		glActiveTexture(GL_TEXTURE0);
		mouseTex.use();
		Render::useShader(&Render::textShader);
		Render::drawRect(vec2(ui::mouse.x, ui::mouse.y - 64), vec2(ui::mouse.x + 64, ui::mouse.y), -9.9);
		Render::textShader.unuse();
	}

	void closeBox() {
		dialogBoxExists = false;
		dialogMerchant = false;
	}

	void dialogAdvance(void) {
		dialogPassive = false;
		dialogPassiveTime = 0;

		if (pageTex) {
			glDeleteTextures(1, &pageTex);
			pageTex = 0;
			pageTexReady = false;
			return;
		}

		if (!typeOutDone) {
			if (!dialogImportant)
				typeOutDone = true;
			return;
		}

		for (int i = 0; i < static_cast<int>(dialogOptText.size()); i++) {
			const auto& dot = dialogOptText[i].second;

			if (mouse.x > dot.x && mouse.x < dot.z &&
			    mouse.y > dot.y && mouse.y < dot.y + 16) { // fontSize
				dialogOptChosen = i + 1;
				break;
			}
		}

		dialogBoxExists = false;

		// handle important text
		if (dialogImportant) {
			dialogImportant = false;
			setFontSize(16);
		}
	}

	void drawFade(void) {
		if (!fadeIntensity) {
			if (fontSize != 16)
				setFontSize(16);
			return;
		}

		static const GLfloat tex[12] = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};

		vec2 p1 (offset.x - game::SCREEN_WIDTH / 2, offset.y - game::SCREEN_HEIGHT / 2);
		vec2 p2 (p1.x + game::SCREEN_WIDTH, p1.y + game::SCREEN_HEIGHT);
        GLfloat backdrop[18] = {
			p1.x, p1.y, -7.9,
			p2.x, p1.y, -7.9,
			p2.x, p2.y, -7.9,

			p2.x, p2.y, -7.9,
			p1.x, p2.y, -7.9,
			p1.x, p1.y, -7.9
		};

		setFontZ(-8.2);
		Render::textShader.use();
		Render::textShader.enable();

		Colors::black.use();
		glUniform4f(Render::textShader.uniform[WU_tex_color], 1.0f, 1.0f, 1.0f, fadeIntensity / 255.0f);
        glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 0, backdrop);
        glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 0, tex);
        glDrawArrays(GL_TRIANGLES, 0, 6);

		Render::textShader.disable();
		Render::textShader.unuse();
		setFontZ(-8.0);
    }

	void fadeUpdate(void) {
		if (fadeEnable) {
			if (fadeIntensity < 150)
				fadeIntensity += fadeFast ? 20 : 5;
			else if (fadeIntensity < 255)
				fadeIntensity += fadeFast ? 10 : 5;
			else
				fadeIntensity = 255;
		} else {
			if (fadeIntensity > 150)
				fadeIntensity -= fadeFast ? 10 : 5;
			else if (fadeIntensity > 0)
				fadeIntensity -= fadeFast ? 20 : 5;
			else
				fadeIntensity = 0;
		}
	}

	void toggleBlack(void) {
		fadeEnable ^= true;
		fadeWhite   = false;
		fadeFast    = false;
	}
	void toggleBlackFast(void) {
		fadeEnable ^= true;
		fadeWhite   = false;
		fadeFast    = true;
	}
	void toggleWhite(void) {
		fadeEnable ^= true;
		fadeWhite   = true;
		fadeFast    = false;
	}
	void toggleWhiteFast(void) {
		fadeEnable ^= true;
		fadeWhite   = true;
		fadeFast    = true;

		Mix_PlayChannel(1, battleStart, 0);
	}

    void takeScreenshot(GLubyte* pixels) {
		auto SCREEN_WIDTH = game::SCREEN_WIDTH;
		auto SCREEN_HEIGHT = game::SCREEN_HEIGHT;

		std::vector<GLubyte> bgr (SCREEN_WIDTH * SCREEN_HEIGHT * 3, 0);

		for(unsigned int x = 0; x < SCREEN_WIDTH*SCREEN_HEIGHT*3; x+=3) {
			bgr[x] = pixels[x+2];
			bgr[x+1] = pixels[x+1];
			bgr[x+2] = pixels[x];
		}

		time_t epoch = time(nullptr);
		struct tm* timen = localtime(&epoch);

		std::string name = "screenshots/";
		name += std::to_string(1900 + timen->tm_year) += "-";
		name += std::to_string(timen->tm_mon + 1) += "-";
		name += std::to_string(timen->tm_mday) += "_";
		name += std::to_string(timen->tm_hour) += "-";
		name += std::to_string(timen->tm_min) += "-";
		name += std::to_string(timen->tm_sec);
		name += ".bmp";
		FILE* bmp = fopen(name.c_str(), "w+");

		// unsigned long header_size = sizeof(BITMAPFILEHEADER) +
		// 							sizeof(BITMAPINFOHEADER);

		BITMAPFILEHEADER bmfh;
		BITMAPINFOHEADER bmih;

		memset(&bmfh, 0, sizeof(BITMAPFILEHEADER));
		memset(&bmih, 0, sizeof(BITMAPINFOHEADER));

		bmfh.bfType = 0x4d42;

		bmfh.bfOffBits = 54;
		bmfh.bfSize = sizeof(BITMAPFILEHEADER) +
					  sizeof(BITMAPINFOHEADER);
		bmfh.bfReserved1 = 0;
		bmfh.bfReserved2 = 0;


		bmih.biSize = sizeof(BITMAPINFOHEADER);
		bmih.biBitCount = 24;

		bmih.biClrImportant = 0;
		bmih.biClrUsed = 0;

		bmih.biCompression = 0;

		bmih.biWidth = SCREEN_WIDTH;
		bmih.biHeight = SCREEN_HEIGHT;

		bmih.biPlanes = 1;
		bmih.biSizeImage = 0;

		bmih.biXPelsPerMeter = 0x0ec4;
		bmih.biYPelsPerMeter = 0x0ec4;

		fwrite(&bmfh, 1,sizeof(BITMAPFILEHEADER),bmp);
		fwrite(&bmih, 1,sizeof(BITMAPINFOHEADER),bmp);
		fwrite(&bgr, 1,3*SCREEN_WIDTH*SCREEN_HEIGHT,bmp);

		delete[] pixels;

		fclose(bmp);
	}
}

using namespace ui;

void InputSystem::receive(const MainSDLEvent& event)
{
	const auto& e = event.event;
	auto& ev = game::events;
	
	switch (e.type) {
	// escape - quit game
	case SDL_QUIT:
		game::endGame();
		break;

	// window events - used for resizing and stuff
	case SDL_WINDOWEVENT:
		switch (e.window.event) {
		case SDL_WINDOWEVENT_RESIZED:
			std::cout << "Window " << e.window.windowID << " resized to: " << e.window.data1 << ", " << e.window.data2 << std::endl;
			auto w = e.window.data1;
			auto h = e.window.data2;
			ev.emit<WindowResizeEvent>(w,h);
			break;
		}
		break;

	// mouse movement - update mouse vector
	case SDL_MOUSEMOTION:
		premouse.x=e.motion.x;
		premouse.y=e.motion.y;
		break;

	case SDL_MOUSEBUTTONUP:
		ev.emit<MouseReleaseEvent>(mouse, e.button.button);
		break;

	case SDL_MOUSEBUTTONDOWN:
		ev.emit<MouseClickEvent>(mouse, e.button.button);

		// run actions?
		//if ((action::make = e.button.button & SDL_BUTTON_RIGHT))
		//	/*player->inv->invHover =*/ edown = false;

		textToDraw.clear();

		if (dialogBoxExists || pageTexReady) {
			// right click advances dialog
			if ((e.button.button & SDL_BUTTON_RIGHT))
				dialogAdvance();
		} else {
			// left click uses item
			if (e.button.button & SDL_BUTTON_LEFT) {
				/*if ((ent = currentWorld->getNearMob(*player)) != nullptr) {
					player->inv->currentAddInteract(ent);
				}
					player->inv->useCurrent();*/
			}

		}
		break;

	case SDL_MOUSEWHEEL:
		ev.emit<MouseScrollEvent>(e.wheel.y);
		break;

	// key presses
	case SDL_KEYDOWN:
		ev.emit<KeyDownEvent>(SDL_KEY);
		switch(SDL_KEY){
			case SDLK_t:
				game::time::tick(100);
				break;
		}
		break;
	
	// key release
	case SDL_KEYUP:
		ev.emit<KeyUpEvent>(SDL_KEY);

		if (SDL_KEY == SDLK_ESCAPE)
			ui::menu::toggle();

		if (SDL_KEY == SDLK_q) {
			/*auto item = player->inv->getCurrentItem();
			if (item != nullptr) {
				if (player->inv->takeItem(item->name, 1) == 0)
					currentWorld->addObject(item->name, "o shit waddup",
											player->loc.x + player->width / 2, player->loc.y + player->height / 2);
			}*/
		} else if (SDL_KEY == SDLK_h) {
			quest::toggle();
		} else switch (SDL_KEY) {
		case SDLK_F3:
			debug ^= true;
			break;
		case SDLK_BACKSLASH:
			dialogBoxExists = false;
			break;
		case SDLK_b:
			if (debug)
				posFlag ^= true;
			break;
		case SDLK_F12:
			// Make the BYTE array, factor of 3 because it's RBG.
			/*static GLubyte* pixels;
			pixels = new GLubyte[ 3 * SCREEN_WIDTH * SCREEN_HEIGHT];
			glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels);
			takeScreenshot(pixels);*/

			ev.emit<ScreenshotEvent>(game::SCREEN_WIDTH, game::SCREEN_HEIGHT);

			std::cout << "Took screenshot" << std::endl;
			break;
		case SDLK_UP:
			//player->inv->setSelectionUp();
			break;
		case SDLK_DOWN:
			//player->inv->setSelectionDown();
			break;
		default:
			break;
		}
		break;
		
	default:
		break;
		
	}
}

void InputSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)en;
	(void)ev;
	(void)dt;
	
	// update mouse coords
	mouse.x = premouse.x + offset.x - (game::SCREEN_WIDTH / 2);
	mouse.y = (offset.y + game::SCREEN_HEIGHT / 2) - premouse.y;
}
