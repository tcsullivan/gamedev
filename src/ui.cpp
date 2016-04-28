#include <ui.hpp>

#include <world.hpp>
#include <gametime.hpp>

extern Menu* currentMenu;

extern SDL_Window *window;

/*
 *	External references for updating player coords / current world.
 */

extern Player *player;
extern World  *currentWorld;
extern World  *currentWorldToLeft;
extern World  *currentWorldToRight;
extern WorldWeather weather;

/*
 *	In the case of dialog, some NPC quests can be preloaded so that they aren't assigned until
 *	the dialog box closes. Reference variables for that here.
*/
extern std::vector<NPC *> aipreload;

/*
 *	Pressing ESC or closing the window will set this to false.
*/

extern bool gameRunning;

/*
 *	Freetype variables
 */

static FT_Library   ftl;
static FT_Face      ftf;

typedef struct {
	vec2 wh;
	vec2 bl;
	vec2 ad;
} FT_Info;

static std::vector<FT_Info> ftdat16 (93, { { 0, 0 }, { 0, 0 }, { 0, 0 } });
static std::vector<GLuint>  ftex16  (93, 0);
static bool ft16loaded = false;

static std::vector<FT_Info> ftdat24 (93, { { 0, 0 }, { 0, 0 }, { 0, 0 } });
static std::vector<GLuint>  ftex24  (93, 0);
static bool ft24loaded = false;

static auto *ftdat = &ftdat16;
static auto *ftex  = &ftex16;

static unsigned char fontColor[4] = {255,255,255,255};

/*
 *	Variables for dialog boxes / options.
 */

static std::vector<std::pair<std::string,vec3>> dialogOptText;
static std::string dialogBoxText;
static std::vector<vec3> merchArrowLoc (2, vec3 { 0, 0, 0 });
static bool typeOutDone = true;
static bool typeOutSustain = false;

static Mix_Chunk *dialogClick;

extern void mainLoop(void);

/*
 * Fade effect flags
 */

static bool fadeEnable = false;
static bool fadeWhite  = false;
static bool fadeFast   = false;
static unsigned int fadeIntensity = 0;

bool inBattle = false;
Mix_Chunk *battleStart;

Mix_Chunk *sanic;

static GLuint pageTex = 0;
static bool   pageTexReady = false;

void loadFontSize(unsigned int size, std::vector<GLuint> &tex, std::vector<FT_Info> &dat)
{
	FT_Set_Pixel_Sizes(ftf,0,size);

	/*
	 *	Pre-render 'all' the characters.
	*/

	glDeleteTextures(93, tex.data());
	glGenTextures(93, tex.data());		//	Generate new texture name/locations?

	for(char i=33;i<126;i++) {

		/*
		 *	Load the character from the font family file.
		*/

		if (FT_Load_Char (ftf, i, FT_LOAD_RENDER))
			UserError("Error! Unsupported character " + i);

		/*
		 *	Transfer the character's bitmap (?) to a texture for rendering.
		*/

		glBindTexture(GL_TEXTURE_2D,tex[i-33]);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S		,GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T		,GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER	,GL_LINEAR		);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER	,GL_LINEAR		);
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);

		/*
		 *	The just-created texture will render red-on-black if we don't do anything to it, so
		 *	here we create a buffer 4 times the size and transform the texture into an RGBA array,
		 *	making it white-on-black.
		*/


		std::vector<uint32_t> buf (ftf->glyph->bitmap.width * ftf->glyph->bitmap.rows, 0xFFFFFFFF);

		for(unsigned int j = buf.size(); j--;)
			buf[j] ^= !ftf->glyph->bitmap.buffer[j] ? buf[j] : 0;

		dat[i - 33].wh.x = ftf->glyph->bitmap.width;
		dat[i - 33].wh.y = ftf->glyph->bitmap.rows;
		dat[i - 33].bl.x = ftf->glyph->bitmap_left;
		dat[i - 33].bl.y = ftf->glyph->bitmap_top;
		dat[i - 33].ad.x = ftf->glyph->advance.x >> 6;
		dat[i - 33].ad.y = ftf->glyph->advance.y >> 6;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ftf->glyph->bitmap.width, ftf->glyph->bitmap.rows,
			          0, GL_RGBA, GL_UNSIGNED_BYTE, buf.data());
	}
}

namespace ui {

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
	Trade merchTrade;

	int fontTransInv = 255;

	/*
	 *	Dialog stuff that needs to be 'public'.
	*/

	bool dialogBoxExists = false;
	bool dialogImportant = false;
	unsigned char dialogOptChosen = 0;
	unsigned char merchOptChosen = 0;

	unsigned int textWrapLimit = 110;

	/*
	 *	Current font size. Changing this WILL NOT change the font size, see setFontSize() for
	 *	actual font size changing.
	*/

	unsigned int fontSize;

    void takeScreenshot(GLubyte* pixels);

	/*
	 *	Initialises the Freetype library, and sets a font size.
	*/

	void initFonts(void) {
		if (FT_Init_FreeType(&ftl))
			UserError("Couldn't initialize freetype.");

#ifdef DEBUG
		DEBUG_printf("Initialized FreeType2.\n",NULL);
#endif // DEBUG
		dialogClick = Mix_LoadWAV("assets/sounds/click.wav");
		battleStart = Mix_LoadWAV("assets/sounds/frig.wav");
		sanic = Mix_LoadWAV("assets/sounds/sanic.wav");
		//Mix_Volume(1,50);

		fontSize = 0;
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
		ft16loaded = false;
		ft24loaded = false;
	}

	/*
	 *	Sets a new font size (default: 12).
	*/

	void setFontSize(unsigned int size) {
		if (size == 16) {
			if (!ft16loaded) {
				loadFontSize(fontSize = size, ftex16, ftdat16);
				ft16loaded = true;
			}
			ftex = &ftex16;
			ftdat = &ftdat16;
			fontSize = 16;
		} else if (size == 24) {
			if (!ft24loaded) {
				loadFontSize(fontSize = size, ftex24, ftdat24);
				ft24loaded = true;
			}
			ftex = &ftex24;
			ftdat = &ftdat24;
			fontSize = 24;
		}
	}

	/*
	 *	Set a color for font rendering (default: white).
	*/

	void setFontColor(unsigned char r,unsigned char g,unsigned char b) {
		fontColor[0]=r;
		fontColor[1]=g;
		fontColor[2]=b;
		fontColor[3]=255;
	}

	void setFontColor(unsigned char r,unsigned char g,unsigned char b, unsigned char a) {
		fontColor[0]=r;
		fontColor[1]=g;
		fontColor[2]=b;
		fontColor[3]=a;
	}

	/*
	 *	Draws a character at the specified coordinates, aborting if the character is unknown.
	*/

	vec2 putChar(float xx,float yy,char c) {
		vec2 c1,c2;

		int x = xx, y = yy;

		/*
		 *	Get the width and height of the rendered character.
		*/

		c1={(float)floor(x)+(*ftdat)[c-33].bl.x,
		    (float)floor(y)+(*ftdat)[c-33].bl.y};
		c2=(*ftdat)[c-33].wh;

		/*
		 *	Draw the character:
		*/

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,(*ftex)[c-33]);
		glPushMatrix();
		glTranslatef(0,-c2.y,0);
		glBegin(GL_QUADS);
			glColor4ub(fontColor[0],fontColor[1],fontColor[2],fontColor[3]);
			glTexCoord2f(0,1);glVertex2f(c1.x     ,c1.y		);
			glTexCoord2f(1,1);glVertex2f(c1.x+c2.x,c1.y		);
			glTexCoord2f(1,0);glVertex2f(c1.x+c2.x,c1.y+c2.y);
			glTexCoord2f(0,0);glVertex2f(c1.x     ,c1.y+c2.y);
		glEnd();
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);

		// return the width.
		return (*ftdat)[c-33].ad;
	}

	/*
	 *	Draw a string at the specified coordinates.
	*/

	float putString(const float x, const float y, std::string s) {
		unsigned int i=0;
		vec2 add, o = {x, y};

		/*
		 *	Loop on each character:
		*/

		do{
			if (i && ((i / 110.0) == (i / 110))) {
				o.y -= fontSize * 1.05f;
				o.x = x;
				if (s[i] == ' ')
					i++;
			}

			if (i && (i / (float)textWrapLimit == i / textWrapLimit)) {
 				o.y -= fontSize * 1.05f;
 				o.x = x;

				// skip a space if it's there since we just newline'd
  				if (s[i] == ' ')
  					i++;
  			}

			switch (s[i]) {
			case '\n':
				o.y -= fontSize * 1.05f;
				o.x = x;
				break;
			case '\r':
				break;
			case '\t':
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

		}while(s[++i]);

		return o.x;	// i.e. the string width
	}

	float putStringCentered(const float x, const float y, std::string s) {
		unsigned int i = 0;
		float width = 0;

		do {
			switch (s[i]) {
			case '\n':
				// TODO
				break;
			case '\b':
				break;
			case ' ':
				width += fontSize / 2;
				break;
			default:
				width += (*ftdat)[i].wh.x + fontSize * 0.1f;
				break;
			}
		} while(s[++i]);
		putString(floor(x-width/2),y,s);
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
		static unsigned int tadv = TICKS_PER_SEC / 12;
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

			if (linc < size)
				linc++;
			else
				typeOutDone = true;
		}

		return ret;		//	The buffered string.
	}

	/*
	 *	Draw a formatted string to the specified coordinates.
	*/

	float putText(const float x, const float y, const char *str, ...) {
		va_list args;
		std::unique_ptr<char[]> buf (new char[512]);

		// zero out the buffer
		memset(buf.get(),0,512*sizeof(char));

		/*
		 *	Handle the formatted string, printing it to the buffer.
		 */

		va_start(args,str);
		vsnprintf(buf.get(),512,str,args);
		va_end(args);

		// draw the string and return the width
		return putString(x, y, buf.get());
	}

	void dialogBox(std::string name, std::string opt, bool passive, std::string text, ...) {
		va_list dialogArgs;
		std::unique_ptr<char[]> printfbuf (new char[512]);

		textWrapLimit = 110;
		dialogPassive = passive;

		// add speaker prefix
		dialogBoxText = name + ": ";

		// handle the formatted string
		va_start(dialogArgs, text);
		vsnprintf(printfbuf.get(), 512, text.c_str(), dialogArgs);
		va_end(dialogArgs);
		dialogBoxText += printfbuf.get();

		// setup option text
		dialogOptText.clear();

		dialogOptChosen = 0;

		if (!opt.empty()) {
			char *sopt = strtok(&opt[0], ":");

			// cycle through options
			while (sopt) {
				dialogOptText.push_back(std::make_pair((std::string)sopt, vec3 {0,0,0}));
				sopt = strtok(NULL,":");
			}
		}

		/*
		 *	Tell draw() that the box is ready.
		*/

		dialogBoxExists = true;
		dialogImportant = false;

		ret.clear();
	}

	void merchantBox(const char *name,Trade trade,const char *opt,bool passive,const char *text,...) {
		va_list dialogArgs;
		std::unique_ptr<char[]> printfbuf (new char[512]);

		dialogPassive = passive;
		merchTrade = trade;

		// clear the buffer
		dialogBoxText.clear();
		dialogBoxText = (std::string)name + ": ";

		va_start(dialogArgs,text);
		vsnprintf(printfbuf.get(),512,text,dialogArgs);
		va_end(dialogArgs);
		dialogBoxText += printfbuf.get();

		// free old option text
		dialogOptText.clear();

		dialogOptChosen = 0;
		merchOptChosen = 0;

		// handle options if desired
		if (opt) {
			std::string soptbuf = opt;
			char *sopt = strtok(&soptbuf[0], ":");

			// cycle through options
			while(sopt) {
				dialogOptText.push_back(std::make_pair((std::string)sopt, vec3 {0,0,0}));
				sopt = strtok(NULL,":");
			}
		}

		// allow box to be displayed
		dialogBoxExists = true;
		dialogImportant = false;
		dialogMerchant = true;
		textWrapLimit = 50;

		ret.clear();
	}

	void merchantBox() {
		textWrapLimit = 50;
		dialogMerchant = true;
	}

	/**
	 * Wait for a dialog box to be dismissed.
	 */

	void waitForDialog(void) {
		while (dialogBoxExists);
	}

	void waitForCover(void) {
		while (fadeIntensity < 255);
		fadeIntensity = 255;
	}

	void waitForNothing (unsigned int ms) {
		unsigned int target = millis() + ms;
		while (millis() < target);
	}

	void importantText(const char *text,...) {
		va_list textArgs;
		std::unique_ptr<char[]> printfbuf (new char[512]);

		dialogBoxText.clear();

		va_start(textArgs,text);
		vsnprintf(printfbuf.get(),512,text,textArgs);
		va_end(textArgs);
		dialogBoxText = printfbuf.get();

		dialogBoxExists = true;
		dialogImportant = true;
		dialogPassive = false;
		dialogPassiveTime = 0;
	}

	void passiveImportantText(int duration, const char *text, ...) {
		va_list textArgs;
		std::unique_ptr<char[]> printfbuf (new char[512]);

		dialogBoxText.clear();

		va_start(textArgs,text);
		vsnprintf(printfbuf.get(),512,text,textArgs);
		va_end(textArgs);
		dialogBoxText = printfbuf.get();

		dialogBoxExists = true;
		dialogImportant = true;
		dialogPassive = true;
		dialogPassiveTime = duration;
	}


	void drawPage(std::string path) {
		pageTex = Texture::loadTexture(path);
		pageTexReady = true;
	}

	void drawBox(vec2 c1, vec2 c2) {
		// draw black body
		glColor3ub(0, 0, 0);
		glRectf(c1.x, c1.y, c2.x, c2.y);

		// draw white border
		glColor3ub(255, 255, 255);
		glBegin(GL_LINE_STRIP);
			glVertex2i(c1.x    , c1.y);
			glVertex2i(c2.x + 1, c1.y);
			glVertex2i(c2.x + 1, c2.y);
			glVertex2i(c1.x - 1, c2.y);
			glVertex2i(c1.x    , c1.y);
		glEnd();
	}

	void draw(void){
		unsigned char i;
		float x,y,tmp;
		std::string rtext;

		auto SCREEN_WIDTH = game::SCREEN_WIDTH;
		auto SCREEN_HEIGHT = game::SCREEN_HEIGHT;

		// will return if not toggled
		action::draw(vec2 {player->loc.x + player->width / 2, player->loc.y + player->height + game::HLINE});

		if (pageTexReady) {
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, pageTex);
			glBegin(GL_QUADS);
				glTexCoord2i(0, 0); glVertex2i(offset.x - 300, SCREEN_HEIGHT - 100);
				glTexCoord2i(1, 0); glVertex2i(offset.x + 300, SCREEN_HEIGHT - 100);
				glTexCoord2i(1, 1); glVertex2i(offset.x + 300, SCREEN_HEIGHT - 600);
				glTexCoord2i(0, 1); glVertex2i(offset.x - 300, SCREEN_HEIGHT - 600);
			glEnd();
			glDisable(GL_TEXTURE_2D);

		} else if (dialogBoxExists) {
			rtext = typeOut(dialogBoxText);

			if (dialogImportant) {
				setFontColor(255,255,255);
				if (dialogPassive) {
					dialogPassiveTime -= game::time::getDeltaTime();
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
			}else if (dialogMerchant) {
				x = offset.x - SCREEN_WIDTH / 6;
				y = (offset.y + SCREEN_HEIGHT / 2) - HLINES(8);

				drawBox(vec2 {x, y}, vec2 {x + SCREEN_WIDTH / 3, y - SCREEN_HEIGHT * 0.6f});

				// draw typeOut'd text
				putString(x + game::HLINE, y - fontSize - game::HLINE, (rtext = typeOut(dialogBoxText)));

				std::string itemString1 = std::to_string(merchTrade.quantity[0]) + "x",
				            itemString2 = std::to_string(merchTrade.quantity[1]) + "x";

				vec2 merchBase = {offset.x, offset.y + SCREEN_HEIGHT / 5};

				putStringCentered(merchBase.x + SCREEN_WIDTH / 10 - 20, merchBase.y + 40 + fontSize * 2, itemString1);
				putStringCentered(merchBase.x + SCREEN_WIDTH / 10 - 20, merchBase.y + 40 + fontSize    , merchTrade.item[0]);
				putStringCentered(merchBase.x - SCREEN_WIDTH / 10     , merchBase.y + 40 + fontSize * 2, itemString2);
				putStringCentered(merchBase.x - SCREEN_WIDTH / 10     , merchBase.y + 40 + fontSize    , merchTrade.item[1]);
				putStringCentered(offset.x, merchBase.y + 60, "for");

				glEnable(GL_TEXTURE_2D);

				glBindTexture(GL_TEXTURE_2D, getItemTexture(merchTrade.item[0]));
				glBegin(GL_QUADS);
					glTexCoord2d(0,1);glVertex2f(offset.x - (SCREEN_WIDTH / 10)     ,offset.y + (SCREEN_HEIGHT/5));
					glTexCoord2d(1,1);glVertex2f(offset.x - (SCREEN_WIDTH / 10) + 40,offset.y + (SCREEN_HEIGHT/5));
					glTexCoord2d(1,0);glVertex2f(offset.x - (SCREEN_WIDTH / 10) + 40,offset.y + (SCREEN_HEIGHT/5) + 40);
					glTexCoord2d(0,0);glVertex2f(offset.x - (SCREEN_WIDTH / 10)     ,offset.y + (SCREEN_HEIGHT/5) + 40);
				glEnd();

				glBindTexture(GL_TEXTURE_2D, getItemTexture(merchTrade.item[1]));
				glBegin(GL_QUADS);
					glTexCoord2d(0,1);glVertex2f(offset.x + (SCREEN_WIDTH / 10) - 40,offset.y + (SCREEN_HEIGHT/5));
					glTexCoord2d(1,1);glVertex2f(offset.x + (SCREEN_WIDTH / 10)     ,offset.y + (SCREEN_HEIGHT/5));
					glTexCoord2d(1,0);glVertex2f(offset.x + (SCREEN_WIDTH / 10)     ,offset.y + (SCREEN_HEIGHT/5) + 40);
					glTexCoord2d(0,0);glVertex2f(offset.x + (SCREEN_WIDTH / 10) - 40,offset.y + (SCREEN_HEIGHT/5) + 40);
				glEnd();

				glDisable(GL_TEXTURE_2D);

				merchArrowLoc[0].x = offset.x - (SCREEN_WIDTH / 8.5) - 16;
				merchArrowLoc[1].x = offset.x + (SCREEN_WIDTH / 8.5) + 16;
				merchArrowLoc[0].y = offset.y + (SCREEN_HEIGHT *.2);
				merchArrowLoc[1].y = offset.y + (SCREEN_HEIGHT *.2);
				merchArrowLoc[0].z = offset.x - (SCREEN_WIDTH / 8.5);
				merchArrowLoc[1].z = offset.x + (SCREEN_WIDTH / 8.5);

				for(i = 0; i < 2; i++) {
					if (((merchArrowLoc[i].x < merchArrowLoc[i].z) ?
						(mouse.x > merchArrowLoc[i].x     && mouse.x < merchArrowLoc[i].z) :
						(mouse.x < merchArrowLoc[i].x     && mouse.x > merchArrowLoc[i].z)) &&
					     mouse.y > merchArrowLoc[i].y - 8 && mouse.y < merchArrowLoc[i].y + 8) {
						glColor3ub(255,255, 0);
					}else{
						glColor3ub(255,255,255);
					}
					glBegin(GL_TRIANGLES);
						glVertex2f(merchArrowLoc[i].x,merchArrowLoc[i].y);
						glVertex2f(merchArrowLoc[i].z,merchArrowLoc[i].y-8);
						glVertex2f(merchArrowLoc[i].z,merchArrowLoc[i].y+8);
					glEnd();
				}


				// draw / handle dialog options if they exist
				for(i = 0; i < dialogOptText.size(); i++) {
					setFontColor(255, 255, 255);

					// draw option
					dialogOptText[i].second.y = y - SCREEN_HEIGHT / 2 - (fontSize + game::HLINE) * (i + 1);
					tmp = putStringCentered(offset.x, dialogOptText[i].second.y, dialogOptText[i].first);

					// get coordinate information on option
					dialogOptText[i].second.z = offset.x + tmp;
					dialogOptText[i].second.x = offset.x - tmp;

					// make text yellow if the mouse hovers over the text
					if (mouse.x > dialogOptText[i].second.x && mouse.x < dialogOptText[i].second.z &&
					   mouse.y > dialogOptText[i].second.y && mouse.y < dialogOptText[i].second.y + 16) {
						  setFontColor(255, 255, 0);
						  putStringCentered(offset.x, dialogOptText[i].second.y, dialogOptText[i].first);
					}
				}

				setFontColor(255, 255, 255);
			} else { //normal dialog box

				x = offset.x - SCREEN_WIDTH / 2  + HLINES(8);
				y = offset.y + SCREEN_HEIGHT / 2 - HLINES(8);

				drawBox(vec2 {x, y}, vec2 {x + SCREEN_WIDTH - HLINES(16), y - SCREEN_HEIGHT / 4});

				rtext = typeOut(dialogBoxText);
				putString(x + game::HLINE, y - fontSize - game::HLINE, rtext);

				for(i=0;i<dialogOptText.size();i++) {
					setFontColor(255,255,255);
					tmp = putStringCentered(offset.x,dialogOptText[i].second.y,dialogOptText[i].first);
					dialogOptText[i].second.z = offset.x + tmp;
					dialogOptText[i].second.x = offset.x - tmp;
					dialogOptText[i].second.y = y - SCREEN_HEIGHT / 4 + (fontSize + game::HLINE) * (i + 1);
					if (mouse.x > dialogOptText[i].second.x &&
					   mouse.x < dialogOptText[i].second.z &&
					   mouse.y > dialogOptText[i].second.y &&
					   mouse.y < dialogOptText[i].second.y + 16) { // fontSize
						  setFontColor(255,255,0);
						  putStringCentered(offset.x,dialogOptText[i].second.y,dialogOptText[i].first);
					}
				}
				setFontColor(255,255,255);
			}

			static unsigned int rtext_oldsize = 0;
			if (rtext_oldsize != rtext.size()) {
				if (!isspace(rtext[(rtext_oldsize = rtext.size()) - 1]))
					Mix_PlayChannel(1, dialogClick, 0);
			}

		}
		if (!fadeIntensity) {
			vec2 hub = {
				(SCREEN_WIDTH/2+offset.x)-fontSize*10,
				(offset.y+SCREEN_HEIGHT/2)-fontSize
			};

			putText(hub.x,hub.y,"Health: %u/%u",player->health>0?(unsigned)player->health:0,
												(unsigned)player->maxHealth
												);
			if (player->isAlive()) {
				glColor3ub(150,0,0);
				hub.y-=fontSize*1.15;
				glRectf(hub.x,
						hub.y,
						hub.x+150,
						hub.y+12);
				glColor3ub(255,0,0);
				glRectf(hub.x,
						hub.y,
						hub.x+(player->health/player->maxHealth * 150),
						hub.y+12);
			}

			/*
			 *	Lists all of the quests the player is currently taking.
			*/

			setFontColor(255,255,255,fontTransInv);
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
			}
			setFontColor(255,255,255,255);
		}
	}

	void quitGame() {
		dialogBoxExists = false;
		currentMenu = NULL;
		gameRunning = false;
		game::config::update();
		game::config::save();
	}

	void closeBox() {
		dialogBoxExists = false;
		dialogMerchant = false;
	}

	void dialogAdvance(void) {
		unsigned char i;

		dialogPassive = false;
		dialogPassiveTime = 0;

		if (pageTex) {
			glDeleteTextures(1, &pageTex);
			pageTex = 0;
			pageTexReady = false;
			return;
		}

		/*if (!typeOutDone) {
			typeOutDone = true;
			return;
		}*/

		for(i=0;i<dialogOptText.size();i++) {
			if (mouse.x > dialogOptText[i].second.x &&
			   mouse.x < dialogOptText[i].second.z &&
			   mouse.y > dialogOptText[i].second.y &&
			   mouse.y < dialogOptText[i].second.y + 16) { // fontSize
				dialogOptChosen = i + 1;
				goto EXIT;
			}
		}

		if (dialogMerchant) {
			for (i = 0; i < merchArrowLoc.size(); i++) {

				// TODO neaten this if statement

				if (((merchArrowLoc[i].x < merchArrowLoc[i].z) ?
				    (mouse.x > merchArrowLoc[i].x && mouse.x < merchArrowLoc[i].z) :
				    (mouse.x < merchArrowLoc[i].x && mouse.x > merchArrowLoc[i].z) &&
					 mouse.y > merchArrowLoc[i].y - 8 && mouse.y < merchArrowLoc[i].y + 8)) {
						merchOptChosen = i + 1;
						goto EXIT;
				}
			}
		}


EXIT:
		//if (!dialogMerchant)closeBox();
		dialogBoxExists = false;
		dialogMerchant = false;

		// handle important text
		if (dialogImportant) {
			dialogImportant = false;
			setFontSize(16);
		}
	}

	void handleEvents(void) {
		static bool left=true,right=false;
		static int heyOhLetsGo = 0;
		static int mouseWheelUpCount = 0, mouseWheelDownCount = 0;

		auto SCREEN_WIDTH = game::SCREEN_WIDTH;
		auto SCREEN_HEIGHT = game::SCREEN_HEIGHT;

		World *tmp;
		vec2 oldpos,tmppos;
		SDL_Event e;

		// update mouse coords
		mouse.x = premouse.x + offset.x - (SCREEN_WIDTH / 2);
		mouse.y = (offset.y + SCREEN_HEIGHT / 2) - premouse.y;

		static vec2 fr;
		static Entity *ig;

		while(SDL_PollEvent(&e)) {
			switch(e.type) {

			// escape - quit game
			case SDL_QUIT:
				gameRunning = false;
				break;

			// mouse movement - update mouse vector
			case SDL_MOUSEMOTION:
				premouse.x=e.motion.x;
				premouse.y=e.motion.y;
				break;

			case SDL_MOUSEBUTTONUP:
				if (ig) {
					ig->vel.x = (fr.x - mouse.x) / 50.0f;
					ig->vel.y = (fr.y - mouse.y) / 50.0f;
                    //ig->forcedMove = true; // kills vel.x too quickly
					ig = NULL;
				}
				break;

			// mouse clicks
			case SDL_MOUSEBUTTONDOWN:

				// run actions?
				if ((action::make = e.button.button & SDL_BUTTON_RIGHT))
					/*player->inv->invHover =*/ edown = false;

				if (dialogBoxExists || pageTexReady) {
					// right click advances dialog
					if ((e.button.button & SDL_BUTTON_RIGHT))
						dialogAdvance();
				} else {
					// left click uses item
					if (e.button.button & SDL_BUTTON_LEFT)
						player->inv->useCurrent();
				}

				if(mouse.x > player->loc.x && mouse.x < player->loc.x + player->width &&
				   mouse.y > player->loc.y && mouse.y < player->loc.y + player->height) {
					player->vel.y = .05;
					fr = mouse;
					ig = player;
				} else {
					for (auto &e : currentWorld->entity) {
						if (mouse.x > e->loc.x && mouse.x < e->loc.x + e->width &&
							mouse.y > e->loc.y && mouse.y < e->loc.y + e->height) {
							e->vel.y = .05;
							fr = mouse;
							ig = e;
							break;
						}
					}
				}

				break;
			case SDL_MOUSEWHEEL:
				if (e.wheel.y < 0) {
					if (mouseWheelUpCount++ && mouseWheelUpCount%5==0) {
						player->inv->setSelectionUp();
						mouseWheelUpCount = 0;
					}
				}else{
					if (mouseWheelDownCount-- && mouseWheelDownCount%5==0) {
						player->inv->setSelectionDown();
						mouseWheelDownCount = 0;
					}
				}
				break;
			// key presses
			case SDL_KEYDOWN:

				// space - make player jump
				if (SDL_KEY == SDLK_SPACE) {
					if (player->ground) {
						player->loc.y += HLINES(2);
						player->vel.y = .4;
						player->ground = false;
					}
					break;

				// only let other keys be handled if dialog allows it
				} else if (!dialogBoxExists || dialogPassive) {
					tmp = currentWorld;
					switch(SDL_KEY) {
					case SDLK_t:
						game::time::tick(50);
						break;
					case SDLK_a:
						if (fadeEnable)break;
						player->vel.x = -PLAYER_SPEED_CONSTANT;
						player->left = left = true;
						player->right = right = false;
						if (currentWorldToLeft) {
							oldpos = player->loc;
							if ((tmp = currentWorld->goWorldLeft(player)) != currentWorld) {
								tmppos = player->loc;
								player->loc = oldpos;

								toggleBlackFast();
								waitForCover();
								player->loc = tmppos;

								currentWorld = tmp;
								toggleBlackFast();
							}
						}
						break;
					case SDLK_d:
						if (fadeEnable)break;
						player->vel.x = PLAYER_SPEED_CONSTANT;
						player->right = right = true;
						player->left = left = false;
						if (currentWorldToRight) {
							oldpos = player->loc;
							if ((tmp = currentWorld->goWorldRight(player)) != currentWorld) {
								tmppos = player->loc;
								player->loc = oldpos;

								toggleBlackFast();
								waitForCover();
								player->loc = tmppos;

								currentWorld = tmp;
								toggleBlackFast();
							}
						}
						break;
					case SDLK_w:
						if (inBattle) {
							tmp = currentWorld;
							currentWorld = ((Arena *)currentWorld)->exitArena(player);
							if (tmp != currentWorld)
								toggleBlackFast();
						} else {
							auto tmpp = currentWorld->goInsideStructure(player);

							if (tmpp.first != currentWorld) {
								ui::toggleBlackFast();
								ui::waitForCover();

								currentWorld = tmpp.first;
								if (tmpp.second)
									player->loc.x = tmpp.second;

								ui::toggleBlackFast();
							}
						}
						break;
					case SDLK_LSHIFT:
						if (debug) {
							Mix_PlayChannel(1, sanic, -1);
							player->speed = 4.0f;
						} else
							player->speed = 2.0f;
						break;
					case SDLK_LCTRL:
						player->speed = .5;
						break;
					case SDLK_e:
						edown = true;

						// start hover counter?
						if (!heyOhLetsGo) {
							heyOhLetsGo = game::time::getTickCount();
							player->inv->mouseSel = false;
						}

						// run hover thing
						if (game::time::getTickCount() - heyOhLetsGo >= 2 && !(player->inv->invOpen) && !(player->inv->selected)) {
							player->inv->invHover = true;

							// enable action ui
							action::enable();
						}

						break;
					default:
						break;
					}

					// handle world switches?
					if (tmp != currentWorld) {
						std::swap(tmp, currentWorld);
						toggleBlackFast();
						waitForCover();
						std::swap(tmp, currentWorld);
						toggleBlackFast();
					}
				}
				break;
			/*
			 *	KEYUP
			*/

			case SDL_KEYUP:
				if (SDL_KEY == SDLK_ESCAPE) {
					ui::menu::toggle();
					player->save();
					return;
				}
				switch (SDL_KEY) {
				case SDLK_F3:
					debug ^= true;
					break;
				case SDLK_z:
					weather = WorldWeather::Rain;
					break;
				case SDLK_i:
					if (isCurrentWorldIndoors() && Indoorp(currentWorld)->isFloorAbove(player)) {
						player->loc.y += getIndoorWorldFloorHeight();
						player->ground = false;
					}
					break;
				case SDLK_k:
					if (isCurrentWorldIndoors() && Indoorp(currentWorld)->isFloorBelow(player)) {
						player->loc.y -= getIndoorWorldFloorHeight();
						player->ground = false;
					}
					break;
				case SDLK_a:
					left = false;
					break;
				case SDLK_d:
					right = false;
					break;
				case SDLK_LSHIFT:
					if (player->speed == 4) {
						Mix_FadeOutChannel(1,2000);
					}
					player->speed = 1;
					break;
				case SDLK_LCTRL:
					player->speed = 1;
					break;
				case SDLK_e:
					edown=false;
					if (player->inv->invHover) {
						player->inv->invHover = false;
					}else{
						if (!player->inv->selected)player->inv->invOpening ^= true;
						else player->inv->selected = false;
						player->inv->mouseSel = false;
					}

					// disable action ui
					action::disable();

					heyOhLetsGo = 0;
					break;
				case SDLK_l:
					currentWorld->addLight({player->loc.x + SCREEN_WIDTH/2, player->loc.y},{1.0f,1.0f,1.0f});
					//currentWorld->getLastLight()->follow(player);
					currentWorld->getLastLight()->makeFlame();
					break;
				case SDLK_f:
					currentWorld->addLight({player->loc.x + SCREEN_WIDTH/2, player->loc.y},{1.0f,1.0f,1.0f});
					break;
				case SDLK_b:
					if (debug)
						posFlag ^= true;
					else {
						currentWorld->addStructure(FIRE_PIT, player->loc.x, player->loc.y, "", "");
						currentWorld->addLight({player->loc.x + SCREEN_WIDTH/2, player->loc.y},{1.0f,1.0f,1.0f});
						//currentWorld->getLastLight()->follow(currentWorld->build.back());
						currentWorld->getLastLight()->makeFlame();
					}
					break;
				case SDLK_F12:
					// Make the BYTE array, factor of 3 because it's RBG.
					static GLubyte* pixels;
					pixels = new GLubyte[ 3 * SCREEN_WIDTH * SCREEN_HEIGHT];
					glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels);
					takeScreenshot(pixels);

					std::cout << "Took screenshot" << std::endl;
					break;
				case SDLK_UP:
					player->inv->setSelectionUp();
					break;
				case SDLK_DOWN:
					player->inv->setSelectionDown();
					break;
				default:
					break;
				}

				if (!left&&!right)
					player->vel.x=0;

				break;
			default:
				break;
			}
		}

		// Flush preloaded AI functions if necessary
		if (!dialogBoxExists) {
			while (!aipreload.empty()) {
				aipreload.front()->addAIFunc(false);
				aipreload.erase(std::begin(aipreload));
			}
		}
	}

	void drawFade(void) {
		auto SCREEN_WIDTH = game::SCREEN_WIDTH;
		auto SCREEN_HEIGHT = game::SCREEN_HEIGHT;

		if (!fadeIntensity) {
			if (fontSize != 16)
				setFontSize(16);
			return;
		}

		if (fadeWhite)
			safeSetColorA(255, 255, 255, fadeIntensity);
		else
			safeSetColorA(0, 0, 0, fadeIntensity);

		glRectf(offset.x - SCREEN_WIDTH  / 2,
				offset.y - SCREEN_HEIGHT / 2,
				offset.x + SCREEN_WIDTH  / 2,
				offset.y + SCREEN_HEIGHT / 2
			    );
	}

	void fadeUpdate(void) {
		if (fadeEnable) {
			if (fadeIntensity < 150)
				fadeIntensity += fadeFast ? 40 : 10;
			else if (fadeIntensity < 255)
				fadeIntensity += fadeFast ? 20 : 5;
			else
				fadeIntensity = 255;
		} else {
			if (fadeIntensity > 150)
				fadeIntensity -= fadeFast ? 20 : 5;
			else if (fadeIntensity > 0)
				fadeIntensity -= fadeFast ? 40 : 10;
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

		for(uint x = 0; x < SCREEN_WIDTH*SCREEN_HEIGHT*3; x+=3) {
			bgr[x] = pixels[x+2];
			bgr[x+1] = pixels[x+1];
			bgr[x+2] = pixels[x];
		}

		time_t epoch = time(NULL);
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
