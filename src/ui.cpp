#include <ui.h>

/**
 * A macro for easier SDL key reading
 */

#define SDL_KEY e.key.keysym.sym

/*
 * Important to have..
 */

extern SDL_Window	*window;
extern Player		*player;
extern World		*currentWorld;
extern bool			 gameRunning;

/*
 * NPC AI functions are given to the NPCs within UI loops.
 */

extern std::vector<int (*)(NPC *)> AIpreload;
extern std::vector<NPC *> AIpreaddr;

/*
 * Variables and objects used with the FreeType library and font rendering.
 */

#define FT_CHAR_COUNT	93

typedef struct {
	GLuint	tex;	/**< OpenGL texture object				 */
	ivec2	wh;		/**< Width and height of the character	 */
	ivec2	bl;		/**< Offset for drawing the character?	 */
	ivec2	ad;		/**< Number of pixels to advance cursor. */
} FT_Tex;

static FT_Tex ftmap16[FT_CHAR_COUNT],
			  ftmap24[FT_CHAR_COUNT],
			  *ftmapptr;

static FT_Library   ftl;
static FT_Face      ftf;
static unsigned char fontColor[3] = {255,255,255};

/*
 *	Variables for dialog boxes / options.
 */

static char			 dialogBoxText[512];
static char			*dialogOptText[4];
static float         merchAOptLoc[2][3];
static float	 	 dialogOptLoc[4][3];
static unsigned char dialogOptCount = 0;
static bool			 typeOutDone = true;

/*
 * Menu-related objects
 */

extern Menu *currentMenu;
extern Menu pauseMenu;

/**
 * The sound made when displaying characters with dialogBox or importantText.
 */

static Mix_Chunk *dialogClick;

/*
 * Other sounds
 */

Mix_Chunk *battleStart;
Mix_Chunk *sanic;

/**
 * A reference to the main loop function for functions like waitForCover().
 */

extern void mainLoop(void);

/*
 * Overlay variables
 */

bool fadeEnable = false;
bool fadeWhite  = false;
bool fadeFast   = false;

unsigned int fadeIntensity = 0;

/**
 * Set to true when the player is in a battle area (i.e. currentWorld points to
 * an Arena).
 */

bool inBattle = false;



void Menu::gotoParent(){
	if(!parent){
		currentMenu = NULL;
		updateConfig();
	}else
		currentMenu = parent;
}

void Menu::gotoChild(){
	if(!child)
		currentMenu = NULL;
	else
		currentMenu = child;
}

static vec2 premouse={0,0};

namespace ui {
	
	/**
	 * The current position of the mouse.
	 */
	
	vec2 mouse;

	/**
	 * If true, debug information will be drawn to the screen.
	 */
	
	bool debug = false;
	
	/**
	 * If true, lines should be drawn to the player when the debug menu is open.
	 */
	
	bool posFlag=false;
	
	/**
	 * If true, the player will be able to move when the current dialog box is
	 * displayed.
	 */
	
	bool dialogPassive = false;
	bool dialogMerchant = false;
	std::vector<BuySell> *minv;
	int dialogPassiveTime = 0;

	/**
	 * When set to true the dialog box will attempt to display.
	 */
	
	bool dialogBoxExists = false;
	
	/**
	 * When set to true the text will display as 'important' text.
	 */
	
	bool dialogImportant = false;
	
	/**
	 * Contains the last chosen dialog option.
	 */
	
	unsigned char dialogOptChosen = 0;
	
	/**
	 * Determines how many characters can be displayed in a dialog box before
	 * a new line is required.
	 */
	
	unsigned int textWrapLimit = 110;
	
	/**
	 * The current font size.
	 * 
	 * DO NOT change this directly, use setFontSize() instead.
	 */
	
	unsigned int fontSize;

	/**
	 * Initializes the Freetype library, and other UI related variables.
	 */

	void initFonts(void){
		
		// init the FreeType library
		if(FT_Init_FreeType(&ftl)){
			std::cout<<"Error! Couldn't initialize the FreeType library."<<std::endl;
			abort();
		}
		
		fontSize = 16;
		
		//ftmap16 = new FT_Tex[FT_CHAR_COUNT];
		//ftmap24 = new FT_Tex[FT_CHAR_COUNT];
		
		memset(&ftmap16, 0, FT_CHAR_COUNT * sizeof(FT_Tex));
		memset(&ftmap24, 0, FT_CHAR_COUNT * sizeof(FT_Tex));
		
#ifdef DEBUG
		DEBUG_printf("Initialized FreeType2.\n",NULL);
#endif // DEBUG

		/*
		 * Load UI related sounds.
		 */

		dialogClick = Mix_LoadWAV("assets/sounds/click.wav");
		battleStart = Mix_LoadWAV("assets/sounds/frig.wav");
		sanic		= Mix_LoadWAV("assets/sounds/sanic.wav");
	}
	
	/**
	 * Frees resources taken by the UI facilities
	 */
	
	void destroyFonts(void){
		//delete[] ftmap16;
		//delete[] ftmap24;
		
		FT_Done_Face(ftf);
		FT_Done_FreeType(ftl);
		
		Mix_FreeChunk(dialogClick);
		Mix_FreeChunk(battleStart);
		Mix_FreeChunk(sanic);
	}
	
	/**
	 * Sets a new font face to use (*.ttf).
	 */
	
	void setFontFace(const char *ttf){
		std::unique_ptr<uint8_t[]> rgbaBuf;
		size_t rgbaBufSize, ftsize;
		unsigned int i,j;
		FT_Error fte;
		
		if((fte = FT_New_Face(ftl,ttf,0,&ftf))){
			std::cout<<"Error! Couldn't open "<<ttf<<" (Error "<<fte<<")."<<std::endl;
			abort();
		}
		
#ifdef DEBUG
		DEBUG_printf("Using font %s\n",ttf);
#endif // DEBUG

		/*
		 * Load the font in the two sizes we use, 16px and 24px.
		 */
		
		ftmapptr = ftmap16;
		ftsize = 16;

		do{
			FT_Set_Pixel_Sizes(ftf, 0, ftsize);
			
			// allocate texture space
			for(i=0; i < FT_CHAR_COUNT; i++)
				glGenTextures(1, &ftmapptr[i].tex);
			
			// Load all characters we expect to use
			for(i=33; i<126; i++){
			
				// Load the bitmap for the current character.
				if(FT_Load_Char(ftf,i,FT_LOAD_RENDER)){
					std::cout<<"Error! Unsupported character "<<(char)i<<" ("<<i<<")."<<std::endl;
					abort();
				}
				
				/*
				 * Set up the OpenGL texture thing.
				 */
			
				glBindTexture(GL_TEXTURE_2D, ftmapptr[i-33].tex);
				
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S		, GL_CLAMP_TO_EDGE);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T		, GL_CLAMP_TO_EDGE);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER	, GL_LINEAR		 );
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER	, GL_LINEAR		 );
				
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			
				/*
				 * Convert the bitmap font given to us from FreeType into an RGBA
				 * format, for ease of drawing.
				 */
			
				rgbaBuf.reset(new uint8_t [(rgbaBufSize = ftf->glyph->bitmap.width * ftf->glyph->bitmap.rows * 4)]);
				rgbaBufSize /= 4;
			
				// populate the buffer
				for(j=0; j < rgbaBufSize; j++){
					rgbaBuf[j * 4] = rgbaBuf[j * 4 + 1] = rgbaBuf[j * 4 + 2] = 255;
					rgbaBuf[j * 4 + 3] = ftf->glyph->bitmap.buffer[j] ? 255 : 0;
				}
				
				// save important character information
				ftmapptr[i-33].wh = { (int)ftf->glyph->bitmap.width, (int)ftf->glyph->bitmap.rows };
				ftmapptr[i-33].bl = { ftf->glyph->bitmap_left,		  ftf->glyph->bitmap_top       };
				ftmapptr[i-33].ad = { ftf->glyph->advance.x >> 6,	  ftf->glyph->advance.y >> 6   };
			
				// do the thing
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ftf->glyph->bitmap.width, ftf->glyph->bitmap.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaBuf.get());
				rgbaBuf.reset();
			}

			// repeat for 24px, or exit
			if(ftmapptr == ftmap16){
				ftmapptr = ftmap24;
				ftsize = 24;
			}else{
				break;
			}
			
		}while(1);

		setFontSize(16);
	}
	
	/**
	 * Sets a new font size and renders the necessary characters (default font
	 * size: 16; 24 for importantText).
	 */
	
	void setFontSize(unsigned int size){
		mtx.lock();
		switch((fontSize = size)){
		case 24:
			ftmapptr = ftmap24;
			break;
		default:
		case 16:
			ftmapptr = ftmap16;
			break;
		}
		mtx.unlock();
	}
	
	/**
	 * Set a color for font rendering (default: white).
	 */
	
	void setFontColor(unsigned char r,unsigned char g,unsigned char b){
		fontColor[0]=r;
		fontColor[1]=g;
		fontColor[2]=b;
	}
	
	/**
	 * Draws a character at the specified coordinates, aborting if the character is undrawable.
	 */
	
	ivec2 putChar(float x,float y,char c){
		ivec2 c1,c2;

		// calculate coordinates
		c1 = { (int)floor(x) + ftmapptr[c-33].bl.x,
		       (int)floor(y) + ftmapptr[c-33].bl.y };
		c2 = ftmapptr[c-33].wh;
		
		/*
		 *	Draw the character
		 */
		
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, ftmapptr[c-33].tex);
		
		glPushMatrix();
		glTranslatef(0,-c2.y,0);
		
		glBegin(GL_QUADS);
			glColor3ub(fontColor[0], fontColor[1], fontColor[2]);
			
			glTexCoord2f(0, 1); glVertex2f(c1.x       , c1.y	   );
			glTexCoord2f(1, 1); glVertex2f(c1.x + c2.x, c1.y	   );
			glTexCoord2f(1, 0); glVertex2f(c1.x + c2.x, c1.y + c2.y);
			glTexCoord2f(0, 0); glVertex2f(c1.x       , c1.y + c2.y);
			
		glEnd();
		
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);

		// return the number of pixels the cursor should move
		return ftmapptr[c-33].ad;
	}
	
	/**
	 * Draw a string at the specified coordinates.
	 */
	
	float putString(const float x,const float y,const char *s){
		unsigned int i = 0;
		ivec2 add;
		vec2 off = { (float)floor(x), (float)floor(y) };
		
		/*
		 *	Loop on each character:
		 */
		
		do{
			// wrap text if necessary
			if(i && (i / (float)textWrapLimit == i / textWrapLimit)){
				off.y -= fontSize * 1.05;
				off.x = x;
				
				// skip a space if it's there since we just newline'd
				if(s[i] == ' ')
					i++;
			}
			
			// handle newlines
			if(s[i] == '\n'){
				off.y -= fontSize * 1.05;
				off.x  = x;

			// (TODO) handle carriage returns and tabs
			}else if(s[i] == '\r' || s[i] == '\t'){

			// handle spaces
			}else if(s[i]==' '){
				off.x += fontSize / 2;
				
			// handle backspaces
			}else if(s[i]=='\b'){
				off.x -= add.x;

			// handle everything else
			}else{
				add = putChar(off.x, off.y, s[i]);
				off.x += add.x;
				off.y += add.y;
			}
			
		}while(s[++i]);
		
		// return string width
		return off.x;
	}
	
	/**
	 * Print a string center-aligned on the specified coordinate.
	 */
	
	float putStringCentered(const float x,const float y,const char *s){
		unsigned int i = 0;
		float width = 0;
		
		/*
		 * Calculate the string's width by cycling through each character.
		 */
		
		do{
			// handle newlines
			if(s[i]=='\n'){

			// handle spaces
			}else if(s[i]==' '){
				width += fontSize / 2;

			// handle backspaces
			}else if(s[i]=='\b'){

			// handle everything else
			}else
				width += ftmapptr[i].wh.x + fontSize * .1;

		}while(s[++i]);

		// print the string
		return putString(floor(x - width / 2), y, s);
	}
	
	/**
	 * Draw a string in a typewriter-esque fashion.
	 * 
	 * This function is expected to be called as it is rendered, slowly allowing
	 * more characters to be drawn as it is looped on. Only one call to this
	 * function can be handled at a time.
	 */

	static char *typeOutStr = NULL;
	char *typeOut(char *str){
		static unsigned int sinc=0,	//	Acts as a delayer for the space between each character.
							linc=0,	//	Contains the number of letters that should be drawn.
							size=0;	//	Contains the full size of the current string.

		// allocate memory for the string if necessary
		if(!typeOutStr){
			typeOutStr = new char[512];
			memset(typeOutStr, 0, 512 * sizeof(char));
		}
		
		/*
		 *	Reset values if a new string is being passed.
		 */
		
		if(strncmp(typeOutStr, str, linc - 1)){
			memset(typeOutStr, 0, 512);		// Zero the buffer
			size = strlen(str);				// Set the new target string size
			linc = 0;						// Reset the incrementers
			sinc = 1;						//
			typeOutDone = false;
		}
		
		/*
		 *	Draw the next letter if necessary.
		 */
		
		if(typeOutDone)
			return str;
		else if(++sinc == 2){
			sinc = 0;
			
			// add next character to output string
			strncpy(typeOutStr + linc,str + linc, 1);
			
			if(linc < size)
				linc++;
			else
				typeOutDone = true;
		}

		// return the string
		return typeOutStr;
	}
	
	/**
	 * Draw a formatted string to the specified coordinates.
	 */
	
	float putText(const float x,const float y,const char *str,...){
		va_list args;
		std::unique_ptr<char[]> buf (new char[512]);
		
		// create the formatted string
		va_start(args, str);
		vsnprintf(buf.get(), 512, str, args);
		va_end(args);
		
		return putString(x, y, buf.get());
	}
	
	/**
	 * Prints a character dialog box.
	 * 
	 * This function sets up the variables necessary to draw a dialog box. If
	 * `opt` contains a valid string, options will be printed with the dialog
	 * box. If the box is passive, the player will be allowed to move while it
	 * is being displayed.
	 */
	
	void dialogBox(const char *name,const char *opt,bool passive,const char *text,...){
		textWrapLimit = 110;
		va_list dialogArgs;
		size_t len;
		
		dialogPassive = passive;
		
		// clear the buffer
		memset(dialogBoxText, '\0', 512);
		
		// create the string
		strcpy(dialogBoxText, name);
		strcat(dialogBoxText, ": ");
		
		len=strlen(dialogBoxText);
		va_start(dialogArgs,text);
		vsnprintf(dialogBoxText + len, 512 - len, text, dialogArgs);
		va_end(dialogArgs);
				
		// free old option text
		while(dialogOptCount){
			if(dialogOptText[dialogOptCount]){
				delete[] dialogOptText[dialogOptCount];
				dialogOptText[dialogOptCount] = NULL;
			}
			
			dialogOptCount--;
		};

		dialogOptChosen = 0;
		memset(&dialogOptLoc, 0, sizeof(float) * 12);
		//char *sopt = (char*)malloc(255);
		//dialogOptCount = 4;
		
		// handle options if desired
		if(opt){
			//std::vector<char*> soptbuf (new char[strlen(opt) + 1]);
			//std::unique_ptr<char[]> soptbuf (new char[strlen(opt) + 1]);
			char soptbuf[255];
			strcpy(soptbuf, opt);
			char *sopt = strtok(soptbuf, ":");

			// cycle through options
			while(sopt){
				printf("%s",sopt);
				strcpy( (dialogOptText[dialogOptCount++] = new char[strlen(sopt) + 1]), sopt);
				sopt = strtok(NULL,":");
			}
		}
		
		// allow box to be displayed
		dialogBoxExists = true;
		dialogImportant = false;
		
		// kill the string created by typeOut if it contains something
		if(typeOutStr)
			*typeOutStr = '\0';
	}

	void merchantBox(const char *name,std::vector<BuySell> *bsinv,const char *opt,bool passive,const char *text,...){
		std::cout << "Buying and selling on the bi-weekly!" << std::endl;
		va_list dialogArgs;
		size_t len;
		
		minv = bsinv;
		dialogPassive = passive;
		
		// clear the buffer
		memset(dialogBoxText, '\0', 512);
		
		// create the string
		strcpy(dialogBoxText, name);
		strcat(dialogBoxText, ": ");
		
		len=strlen(dialogBoxText);
		va_start(dialogArgs,text);
		vsnprintf(dialogBoxText + len, 512 - len, text, dialogArgs);
		va_end(dialogArgs);
				
		// free old option text
		while(dialogOptCount){
			if(dialogOptText[dialogOptCount]){
				delete[] dialogOptText[dialogOptCount];
				dialogOptText[dialogOptCount] = NULL;
			}
			
			dialogOptCount--;
		};

		dialogOptChosen = 0;
		memset(&dialogOptLoc, 0, sizeof(float) * 12);
		
		// handle options if desired
		if(opt){
			//std::unique_ptr<char[]> soptbuf (new char[strlen(opt) + 1]);
			char soptbuf[255];
			strcpy(soptbuf, opt);
			char *sopt = strtok(soptbuf, ":");

			// cycle through options
			while(sopt){
				strcpy( (dialogOptText[dialogOptCount++] = new char[strlen(sopt) + 1]), sopt);
				sopt = strtok(NULL,":");
			}
		}
		
		// allow box to be displayed
		dialogBoxExists = true;
		dialogImportant = false;
		dialogMerchant = true;
		textWrapLimit = 50;
		
		// kill the string created by typeOut if it contains something
		if(typeOutStr)
			*typeOutStr = '\0';
	}
	
	void merchantBox(){
		textWrapLimit = 50;
		dialogMerchant = true;
	}
	
	/**
	 * Wait for a dialog box to be dismissed.
	 */
	
	void waitForDialog(void){
		do{
			mainLoop();
		}while(ui::dialogBoxExists);
	}
	
	/**
	 * Wait for the screen to be fully covered through toggle___().
	 */
	
	void waitForCover(void){
		do{
			mainLoop();
		}while(fadeIntensity < 255);
		fadeIntensity = 255;
	}
	
	/**
	 * Prepare formatted 'important' string for drawing.
	 * 
	 * "Important" text will display in a bigger font size at the center of the
	 * screen. Usually accompanied by a cover from toggle___() and limits on
	 * player controls.
	 */
	
	void importantText(const char *text,...){
		va_list textArgs;

		// clear dialog buffer (we share the same)
		memset(dialogBoxText, '\0', 512);

		// format the string
		va_start(textArgs,text);
		vsnprintf(dialogBoxText, 512, text, textArgs);
		va_end(textArgs);

		// set draw flags
		dialogBoxExists = true;
		dialogImportant = true;
	}
	
	/**
	 * Draw a passive 'important' text for a certain duration.
	 */
	
	void passiveImportantText(int duration, const char *text,...){
		va_list textArgs;

		// clear buffer
		memset(dialogBoxText, '\0', 512);

		// format the string
		va_start(textArgs,text);
		vsnprintf(dialogBoxText, 512, text, textArgs);
		va_end(textArgs);

		// set draw flags
		dialogBoxExists = true;
		dialogImportant = true;
		dialogPassive = true;
		dialogPassiveTime = duration;
	}

	/**
	 * Draws all UI-related elements to the screen.
	 */

	void draw(void){
		unsigned char i;
		float x,y,tmp;
		char *rtext;
		
		// handle dialog box / important text
		if(dialogBoxExists){
			
			rtext = typeOut(dialogBoxText);
			
			if(dialogImportant){
				setFontColor(255, 255, 255);
				
				// handle timeout
				if(dialogPassive && (dialogPassiveTime -= deltaTime) <= 0){
					dialogPassive = false;
					dialogImportant = false;
					dialogBoxExists = false;
				}
				
				// draw text
				if(fadeIntensity == 255 || dialogPassive){
					setFontSize(24);
					putStringCentered(offset.x, offset.y, rtext);
					setFontSize(16);
				}
			}else if(dialogMerchant){
				static int dispItem;

				x=offset.x-SCREEN_WIDTH/6;
				y=(offset.y+SCREEN_HEIGHT/2)-HLINE*8;
			
			
				glColor3ub(255,255,255);
				glBegin(GL_LINE_STRIP);
					glVertex2f(x-1				 	  ,y+1);
					glVertex2f(x+1+(SCREEN_WIDTH/3),y+1);
					glVertex2f(x+1+(SCREEN_WIDTH/3),y-1-SCREEN_HEIGHT*.6);
					glVertex2f(x-1,y-1-SCREEN_HEIGHT*.6);
					glVertex2f(x,y+1);
				glEnd();
			
				glColor3ub(0,0,0);
				glRectf(x,y,x+SCREEN_WIDTH/3,y-SCREEN_HEIGHT*.6);
				
				// draw typeOut'd text
				putString(x + HLINE, y - fontSize - HLINE, (rtext = typeOut(dialogBoxText)));
				merchAOptLoc[0][0] = offset.x - (SCREEN_WIDTH / 6.5) - 16;
				merchAOptLoc[1][0] = offset.x + (SCREEN_WIDTH / 6.5);
				merchAOptLoc[0][1] = offset.y + (SCREEN_HEIGHT *.25);
				merchAOptLoc[1][1] = offset.y + (SCREEN_HEIGHT *.25);
				merchAOptLoc[0][2] = offset.x - (SCREEN_WIDTH / 6.5);
				merchAOptLoc[1][2] = offset.x + (SCREEN_WIDTH / 6.5) + 16;

				for(i = 0; i < 2; i++){
					if(mouse.x > merchAOptLoc[i][0] && mouse.x < merchAOptLoc[i][2] &&
					   mouse.y > merchAOptLoc[i][1] - 8 && mouse.y < merchAOptLoc[i][1] + 8){
					   	dispItem++;
						glColor3ub(255,255,  0);
					}else{
						glColor3ub(255,255,255);
					}
				}

				glBegin(GL_TRIANGLES);
					glVertex2f(merchAOptLoc[0][0],merchAOptLoc[0][1]);
					glVertex2f(merchAOptLoc[0][2],merchAOptLoc[0][1]-8);
					glVertex2f(merchAOptLoc[0][2],merchAOptLoc[0][1]+8);

					glVertex2f(merchAOptLoc[1][2],merchAOptLoc[1][1]);
					glVertex2f(merchAOptLoc[1][0],merchAOptLoc[1][1]-8);
					glVertex2f(merchAOptLoc[1][0],merchAOptLoc[1][1]+8);
				glEnd();

			
				// draw / handle dialog options if they exist
				for(i = 0; i < dialogOptCount; i++){
					setFontColor(255, 255, 255);
					
					// draw option
					tmp = putStringCentered(offset.x, dialogOptLoc[i][1], dialogOptText[i]);
					
					// get coordinate information on option
					dialogOptLoc[i][2] = offset.x + tmp;
					dialogOptLoc[i][0] = offset.x - tmp;
					dialogOptLoc[i][1] = y - SCREEN_HEIGHT / 2 - (fontSize + HLINE) * (i + 1);
					
					// make text yellow if the mouse hovers over the text
					if(mouse.x > dialogOptLoc[i][0] && mouse.x < dialogOptLoc[i][2] &&
					   mouse.y > dialogOptLoc[i][1] && mouse.y < dialogOptLoc[i][1] + 16 ){
						  setFontColor(255, 255, 0);
						  putStringCentered(offset.x, dialogOptLoc[i][1], dialogOptText[i]);
					}
				}
				
				setFontColor(255, 255, 255);
			}else{ //normal dialog box
			
				x=offset.x-SCREEN_WIDTH/2+HLINE*8;
				y=(offset.y+SCREEN_HEIGHT/2)-HLINE*8;
						
				// draw white border
				glColor3ub(255, 255, 255);
				glBegin(GL_LINE_STRIP);
					glVertex2f(x - 1							, y + 1						);
					glVertex2f(x + 1 + SCREEN_WIDTH - HLINE * 16, y + 1						);
					glVertex2f(x + 1 + SCREEN_WIDTH - HLINE * 16, y - 1 - SCREEN_HEIGHT / 4 );
					glVertex2f(x - 1							, y - 1 - SCREEN_HEIGHT / 4 );
					glVertex2f(x								, y + 1						);
				glEnd();
			
				// draw black box
				glColor3ub(0, 0, 0);
				glRectf(x, y, x + SCREEN_WIDTH - HLINE * 16, y - SCREEN_HEIGHT / 4);
			
				// draw typeOut'd text
				putString(x + HLINE, y - fontSize - HLINE, (rtext = typeOut(dialogBoxText)));
			
				// draw / handle dialog options if they exist
				for(i = 0; i < dialogOptCount; i++){
					setFontColor(255, 255, 255);
					
					// draw option
					tmp = putStringCentered(offset.x, dialogOptLoc[i][1], dialogOptText[i]);
					
					// get coordinate information on option
					dialogOptLoc[i][2] = offset.x + tmp;
					dialogOptLoc[i][0] = offset.x - tmp;
					dialogOptLoc[i][1] = y - SCREEN_HEIGHT / 4 + (fontSize + HLINE) * (i + 1);
					
					// make text yellow if the mouse hovers over the text
					if(mouse.x > dialogOptLoc[i][0] && mouse.x < dialogOptLoc[i][2] &&
					   mouse.y > dialogOptLoc[i][1] && mouse.y < dialogOptLoc[i][1] + 16 ){
						  setFontColor(255, 255, 0);
						  putStringCentered(offset.x, dialogOptLoc[i][1], dialogOptText[i]);
					}
				}
				
				setFontColor(255, 255, 255);
			}
			
			// make click for each character update
			if(strcmp(rtext, dialogBoxText))
				Mix_PlayChannel(1, dialogClick, 0);
			
		}
		
		// draw information stuffs
		if(!fadeIntensity){
			
			vec2 hub = {
				(offset.x + SCREEN_WIDTH  / 2) - fontSize * 10,
				(offset.y + SCREEN_HEIGHT / 2) - fontSize
			};
			
			// health text
			putText(hub.x,
					hub.y,
					"Health: %.0f/%.0f",
					player->health > 0 ? player->health : 0,
					player->maxHealth
					);

			// health bar
			if(player->alive){
				hub.y -= fontSize * 1.15;
				
				glColor3ub(150, 0, 0);
				glRectf(hub.x,
						hub.y,
						hub.x + 150,
						hub.y + 12
						);
						
				glColor3ub(255,0,0);
				glRectf(hub.x,
						hub.y,
						hub.x + (player->health / player->maxHealth * 150),
						hub.y + 12
						);
			}
			
			// inventory
			if(player->inv->invOpen){
				hub.y = player->loc.y + fontSize * 8;
				hub.x = player->loc.x;
				
				putStringCentered(hub.x, hub.y, "Current Quests:");
				
				for(auto &q : player->qh.current)
					putStringCentered(hub.x, (hub.y -= fontSize * 1.15), q.title.c_str());
			}
		}
	}

	/**
	 * Safely exits the game.
	 */

	void quitGame(){
		// clean menu stuff
		currentMenu = NULL;
		delete[] currentMenu;
		
		// save options
		updateConfig();
		saveConfig();
		
		// tell main loop to exit
		gameRunning = false;
	}
	
	menuItem createButton(vec2 l, dim2 d, Color c, const char* t, menuFunc f){
		menuItem temp;
		temp.member = 0;

		temp.button.loc = l;
		temp.button.dim = d;
		temp.button.color = c;

		temp.button.text = t;

		temp.button.func = f;

		return temp;
	}

	menuItem createChildButton(vec2 l, dim2 d, Color c, const char* t){
		menuItem temp;
		temp.member = -1;

		temp.button.loc = l;
		temp.button.dim = d;
		temp.button.color = c;

		temp.button.text = t;

		temp.button.func = NULL;

		return temp;
	}

	menuItem createParentButton(vec2 l, dim2 d, Color c, const char* t){
		menuItem temp;
		temp.member = -2;

		temp.button.loc = l;
		temp.button.dim = d;
		temp.button.color = c;

		temp.button.text = t;

		temp.button.func = NULL;

		return temp;
	}

	menuItem createSlider(vec2 l, dim2 d, Color c, float min, float max, const char* t, float* v){
		menuItem temp;
		temp.member = 1;

		temp.slider.loc = l;
		temp.slider.dim = d;
		temp.slider.color = c;
		temp.slider.minValue = min;
		temp.slider.maxValue = max;

		temp.slider.text = t;

		temp.slider.var = v;

		temp.slider.sliderLoc = *v;

		return temp;
	}

	/*
	 *	Draws the menu
	*/

	void drawMenu(Menu *menu){
		setFontSize(24);
		updateConfig();
		SDL_Event e;
			
		mouse.x=premouse.x+offset.x-(SCREEN_WIDTH/2);
		mouse.y=(offset.y+SCREEN_HEIGHT/2)-premouse.y;

		//custom event polling for menu's so all other events are disregarded
		while(SDL_PollEvent(&e)){
			switch(e.type){
			case SDL_QUIT:
				gameRunning=false;
				return;
				break;
			case SDL_MOUSEMOTION:
				premouse.x=e.motion.x;
				premouse.y=e.motion.y;
				break;
			case SDL_KEYUP:
				if(SDL_KEY == SDLK_ESCAPE){
					menu->gotoParent();
					return;
				}
				break;
			default:break;
			}
		}

		//draw the dark transparent background
		glColor4f(0.0f, 0.0f, 0.0f, .8f);
		glRectf(offset.x-SCREEN_WIDTH/2,0,offset.x+SCREEN_WIDTH/2,SCREEN_HEIGHT);

		//loop through all elements of the menu
		for(auto &m : menu->items){
			//if the menu is any type of button
			if(m.member == 0 || m.member == -1 || m.member == -2){

				//draw the button background
				glColor3f(m.button.color.red,m.button.color.green,m.button.color.blue);
				glRectf(offset.x+m.button.loc.x, 
						offset.y+m.button.loc.y, 
						offset.x+m.button.loc.x + m.button.dim.x, 
						offset.y+m.button.loc.y + m.button.dim.y);
				//draw the button text
				putStringCentered(offset.x + m.button.loc.x + (m.button.dim.x/2),
								  (offset.y + m.button.loc.y + (m.button.dim.y/2)) - ui::fontSize/2,
								  m.button.text);
				
				//tests if the mouse is over the button
				if(mouse.x >= offset.x+m.button.loc.x && mouse.x <= offset.x+m.button.loc.x + m.button.dim.x){
					if(mouse.y >= offset.y+m.button.loc.y && mouse.y <= offset.y+m.button.loc.y + m.button.dim.y){

						//if the mouse if over the button, it draws this white outline
						glColor3f(1.0f,1.0f,1.0f);
						glBegin(GL_LINE_STRIP);
							glVertex2f(offset.x+m.button.loc.x, 					offset.y+m.button.loc.y);
							glVertex2f(offset.x+m.button.loc.x+m.button.dim.x, 		offset.y+m.button.loc.y);
							glVertex2f(offset.x+m.button.loc.x+m.button.dim.x, 		offset.y+m.button.loc.y+m.button.dim.y);
							glVertex2f(offset.x+m.button.loc.x, 					offset.y+m.button.loc.y+m.button.dim.y);
							glVertex2f(offset.x+m.button.loc.x, 					offset.y+m.button.loc.y);
						glEnd();

						//if the mouse is over the button and clicks
						if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)){
							switch(m.member){
								case 0: //normal button
									m.button.func();
									break;
								case -1:
									menu->gotoChild(); //goto child menu
									break;
								case -2:
									menu->gotoParent(); //goto parent menu
								default:break;
							}
						}
					}
				}

				//if element is a slider
			}else if(m.member == 1){
				//combining slider text with variable amount
				char outSV[32];
				sprintf(outSV, "%s: %.1f",m.slider.text, *m.slider.var);

				float sliderW, sliderH;

				if(m.slider.dim.y > m.slider.dim.x){
					//width of the slider handle
					sliderW = m.slider.dim.x;
					sliderH = m.slider.dim.y * .05;
					//location of the slider handle
					m.slider.sliderLoc = m.slider.minValue + (*m.slider.var/m.slider.maxValue)*(m.slider.dim.y-sliderW);
				}else{
					//width of the slider handle
					sliderW = m.slider.dim.x * .05;
					sliderH = m.slider.dim.y;
					//location of the slider handle
					m.slider.sliderLoc = m.slider.minValue + (*m.slider.var/m.slider.maxValue)*(m.slider.dim.x-sliderW);
				}
				//draw the background of the slider
				glColor4f(m.slider.color.red,m.slider.color.green,m.slider.color.blue, .5f);
				glRectf(offset.x+m.slider.loc.x, 
						offset.y+m.slider.loc.y, 
						offset.x+m.slider.loc.x + m.slider.dim.x, 
						offset.y+m.slider.loc.y + m.slider.dim.y);

				//draw the slider handle
				glColor4f(m.slider.color.red,m.slider.color.green,m.slider.color.blue, 1.0f);
				if(m.slider.dim.y > m.slider.dim.x){
					glRectf(offset.x+m.slider.loc.x,
						offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05),
						offset.x+m.slider.loc.x + sliderW,
						offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05) + sliderH);

					//draw the now combined slider text
					putStringCentered(offset.x + m.slider.loc.x + (m.slider.dim.x/2), (offset.y + m.slider.loc.y + (m.slider.dim.y*1.05)) - ui::fontSize/2, outSV);
				}else{
					glRectf(offset.x+m.slider.loc.x+m.slider.sliderLoc,
							offset.y+m.slider.loc.y,
							offset.x+m.slider.loc.x + m.slider.sliderLoc + sliderW,
							offset.y+m.slider.loc.y + sliderH);

					//draw the now combined slider text
					putStringCentered(offset.x + m.slider.loc.x + (m.slider.dim.x/2), (offset.y + m.slider.loc.y + (m.slider.dim.y/2)) - ui::fontSize/2, outSV);
				}				
				//test if mouse is inside of the slider's borders
				if(mouse.x >= offset.x+m.slider.loc.x && mouse.x <= offset.x+m.slider.loc.x + m.slider.dim.x){
					if(mouse.y >= offset.y+m.slider.loc.y && mouse.y <= offset.y+m.slider.loc.y + m.slider.dim.y){

						//if it is we draw a white border around it
						glColor3f(1.0f,1.0f,1.0f);
						glBegin(GL_LINE_STRIP);
							glVertex2f(offset.x+m.slider.loc.x, 					offset.y+m.slider.loc.y);
							glVertex2f(offset.x+m.slider.loc.x+m.slider.dim.x, 		offset.y+m.slider.loc.y);
							glVertex2f(offset.x+m.slider.loc.x+m.slider.dim.x, 		offset.y+m.slider.loc.y+m.slider.dim.y);
							glVertex2f(offset.x+m.slider.loc.x, 					offset.y+m.slider.loc.y+m.slider.dim.y);
							glVertex2f(offset.x+m.slider.loc.x, 					offset.y+m.slider.loc.y);

							if(m.slider.dim.y > m.slider.dim.x){
								//and a border around the slider handle
								glVertex2f(offset.x+m.slider.loc.x, 		  offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05));
								glVertex2f(offset.x+m.slider.loc.x + sliderW, offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05));
								glVertex2f(offset.x+m.slider.loc.x + sliderW, offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05) + sliderH);
								glVertex2f(offset.x+m.slider.loc.x,           offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05) + sliderH);
								glVertex2f(offset.x+m.slider.loc.x,           offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05));
							}else{
								//and a border around the slider handle
								glVertex2f(offset.x+m.slider.loc.x + m.slider.sliderLoc, offset.y+m.slider.loc.y);
								glVertex2f(offset.x+m.slider.loc.x + (m.slider.sliderLoc + sliderW), offset.y+m.slider.loc.y);
								glVertex2f(offset.x+m.slider.loc.x + (m.slider.sliderLoc + sliderW), offset.y+m.slider.loc.y+m.slider.dim.y);
								glVertex2f(offset.x+m.slider.loc.x + m.slider.sliderLoc, offset.y+m.slider.loc.y+m.slider.dim.y);
								glVertex2f(offset.x+m.slider.loc.x + m.slider.sliderLoc, offset.y+m.slider.loc.y);
							}

						glEnd();

						//if we are inside the slider and click it will set the slider to that point
						if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)){
							//change handle location
							if(m.slider.dim.y > m.slider.dim.x){
								*m.slider.var = (((mouse.y-offset.y) - m.slider.loc.y)/m.slider.dim.y)*100;
								//draw a white box over the handle
								glColor3f(1.0f,1.0f,1.0f);
								glRectf(offset.x+m.slider.loc.x, 
										offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05), 
										offset.x+m.slider.loc.x + sliderW, 
										offset.y+m.slider.loc.y + (m.slider.sliderLoc * 1.05) + sliderH);

							}else{
								*m.slider.var = (((mouse.x-offset.x) - m.slider.loc.x)/m.slider.dim.x)*100;
								//draw a white box over the handle
								glColor3f(1.0f,1.0f,1.0f);
								glRectf(offset.x+m.slider.loc.x + m.slider.sliderLoc, 
										offset.y+m.slider.loc.y, 
										offset.x+m.slider.loc.x + (m.slider.sliderLoc + sliderW), 
										offset.y+m.slider.loc.y + m.slider.dim.y);
							}
						}

						//makes sure handle can't go below or above min and max values
						if(*m.slider.var >= m.slider.maxValue)*m.slider.var = m.slider.maxValue;
						else if(*m.slider.var <= m.slider.minValue)*m.slider.var = m.slider.minValue;
					}
				}
			}
		}
		setFontSize(16);
	}

	void takeScreenshot(GLubyte* pixels){
		GLubyte bgr[SCREEN_WIDTH*SCREEN_HEIGHT*3];
		for(uint x = 0; x < SCREEN_WIDTH*SCREEN_HEIGHT*3; x+=3){
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

	/**
	 * Handles dialog box closing, option selecting and stuff.
	 */

	void dialogAdvance(void){
		unsigned char i;
		
		// if typeOut hasn't finished, tell it to then try again
		if(!typeOutDone){
			typeOutDone = true;
			return;
		}

		// check for selected option
		for(i = 0; i < dialogOptCount; i++){
			if(mouse.x > dialogOptLoc[i][0] && mouse.x < dialogOptLoc[i][2] &&
			   mouse.y > dialogOptLoc[i][1] && mouse.y < dialogOptLoc[i][1] + 16 ){
				dialogOptChosen = i + 1;
				break;
			}
		}

		if(dialogMerchant){
			for(i = 0; i < 2; i++){
			}
		}
		
		// handle important text
		if(dialogImportant){
			dialogImportant = false;
			setFontSize(16);
		}

		if(dialogMerchant) dialogMerchant = false;
		dialogBoxExists = false;
	}

	void handleEvents(void){
		static bool left=true,right=false;
		static int heyOhLetsGo = 0;
		World *tmp;
		vec2 oldpos,tmppos;
		SDL_Event e;
		
		mouse.x=premouse.x+offset.x-(SCREEN_WIDTH/2);
		mouse.y=(offset.y+SCREEN_HEIGHT/2)-premouse.y;
		
		while(SDL_PollEvent(&e)){
			switch(e.type){
			case SDL_QUIT:
				gameRunning=false;
				break;
			case SDL_MOUSEMOTION:
				premouse.x=e.motion.x;
				premouse.y=e.motion.y;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if((e.button.button & SDL_BUTTON_RIGHT) && dialogBoxExists)
					dialogAdvance();
				if((e.button.button & SDL_BUTTON_LEFT) && !dialogBoxExists)
					player->inv->usingi = true;
				break;
			/*
				KEYDOWN
			*/
			case SDL_KEYDOWN:
				/*if(SDL_KEY == SDLK_ESCAPE){
					//gameRunning = false;
					pMenu = true;
					return;
				}else */if(SDL_KEY == SDLK_SPACE){
					/*if(dialogBoxExists)
						dialogAdvance();
					else */if(player->ground){
						player->vel.y=.4;
						player->loc.y+=HLINE*2;
						player->ground=false;
					}
					break;
				}else if(!dialogBoxExists || dialogPassive){
					tmp = currentWorld;
					switch(SDL_KEY){
					case SDLK_a:
						if(fadeEnable)break;
						player->vel.x=-.15;
						player->left = true;
						player->right = false;
						left = true;
						right = false;
						if(currentWorld->toLeft){
							oldpos = player->loc;
							if((tmp = currentWorld->goWorldLeft(player)) != currentWorld){
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
						if(fadeEnable)break;
						player->vel.x=.15;
						player->right = true;
						player->left = false;
						left = false;
						right = true;
						if(currentWorld->toRight){
							oldpos = player->loc;
							if((tmp = currentWorld->goWorldRight(player)) != currentWorld){
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
					case SDLK_s:
						break;
					case SDLK_w:
						if(inBattle){
							tmp = currentWorld;
							currentWorld = ((Arena *)currentWorld)->exitArena(player);
							if(tmp != currentWorld){
								//delete &tmp;
								toggleBlackFast();
							}
						}else{
							if((tmp = currentWorld->goInsideStructure(player)) != currentWorld)
								currentWorld = tmp;
						}
						break;
					case SDLK_i:
						/*currentWorld=currentWorld->goWorldBack(player);	// Go back a layer if possible	
						if(tmp!=currentWorld){
							currentWorld->detect(player);
							player->vel.y=.2;
							player->loc.y+=HLINE*5;
							player->ground=false;
						}*/
						player->health -= 5;
						break;
					case SDLK_k:
						/*currentWorld=currentWorld->goWorldFront(player);	// Go forward a layer if possible
						if(tmp!=currentWorld){
							currentWorld->behind->detect(player);
							player->vel.y=.2;
							player->loc.y+=HLINE*5;
							player->ground=false;
						}*/
						break;
					case SDLK_LSHIFT:
						if(debug){
							Mix_PlayChannel(1,sanic,-1);
							player->speed = 4.0f;
						}else
							player->speed = 2.0f;
						break;
					case SDLK_LCTRL:
						player->speed = .5;
						break;
					case SDLK_F3:
						debug ^= true;
						break;
					case SDLK_b:
						if(debug)posFlag ^= true;
						break;
					case SDLK_e:
						if(!heyOhLetsGo){
							heyOhLetsGo = loops;
							player->inv->mouseSel = false;
						}
						if(loops - heyOhLetsGo >= 2 && !(player->inv->invOpen) && !(player->inv->selected))
							player->inv->invHover=true;
						break;
					default:
						break;
					}
					if(tmp != currentWorld){
						std::swap(tmp,currentWorld);
						toggleBlackFast();
						waitForCover();
						std::swap(tmp,currentWorld);
						toggleBlackFast();
					}
				}
				break;
			/*
			 *	KEYUP
			*/
			
			case SDL_KEYUP:
				if(SDL_KEY == SDLK_ESCAPE){
					//gameRunning = false;
					currentMenu = &pauseMenu;
					player->save();
					return;
				}
				switch(SDL_KEY){
				case SDLK_a:
					left = false;
					break;
				case SDLK_d:
					right = false;
					break;
				case SDLK_LSHIFT:
					if(player->speed == 4){
						Mix_FadeOutChannel(1,2000);
					}
					player->speed = 1;
					break;
				case SDLK_LCTRL:
					player->speed = 1;
					break;
				case SDLK_e:
					if(player->inv->invHover){
						player->inv->invHover = false;
					}else{
						if(!player->inv->selected)player->inv->invOpening ^= true;
						else player->inv->selected = false;
						player->inv->mouseSel = false;
					}
					heyOhLetsGo = 0;
					break;
				case SDLK_LEFT:
					if(player->inv->sel)player->inv->sel--;
					break;
				case SDLK_RIGHT:
					player->inv->sel++;
					break;
				case SDLK_l:
					player->light^=true;
					break;
				case SDLK_f:
					currentWorld->addLight({player->loc.x + SCREEN_WIDTH/2, player->loc.y},{1.0f,1.0f,1.0f});
					break;
				case SDLK_g:
					//currentWorld->addStructure(LAMP_POST, player->loc.x, player->loc.y, NULL);
					break;
				case SDLK_h:
					//currentWorld->addStructure(TOWN_HALL, player->loc.x, player->loc.y, NULL);
					break;
				case SDLK_j:
					//currentWorld->addStructure(FOUNTAIN, player->loc.x, player->loc.y, NULL);
					break;
				case SDLK_v:
					//currentWorld->addVillage(player->loc.x, player->loc.y, 5, 10, 100, NULL);
					break;
				case SDLK_b:
					currentWorld->addStructure(FIRE_PIT, player->loc.x, player->loc.y, NULL, NULL);
					currentWorld->addLight({player->loc.x + SCREEN_WIDTH/2, player->loc.y},{1.0f,1.0f,1.0f});
					break;
				case SDLK_F12:
					// Make the BYTE array, factor of 3 because it's RBG.
					static GLubyte* pixels;
					pixels = new GLubyte[ 3 * SCREEN_WIDTH * SCREEN_HEIGHT];
					glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels);

					//static std::thread scr;
					//scr = std::thread(takeScreenshot,pixels);
					//scr.detach();
					takeScreenshot(pixels);

					std::cout << "Took screenshot" << std::endl;
					break;
				default:
					break;
				}
				
				if(!left&&!right)
					player->vel.x=0;
					
				break;
			default:
				break;
			}
		}
		
		if(!dialogBoxExists&&AIpreaddr.size()){	// Flush preloaded AI functions if necessary
			while(!AIpreaddr.empty()){
				AIpreaddr.front()->addAIFunc(AIpreload.front(),false);
				AIpreaddr.erase(AIpreaddr.begin());
				AIpreload.erase(AIpreload.begin());
			}
		}
	}
	
	/**
	 * Toggle a slow fade to/from black.
	 */
	
	void toggleBlack(void){
		fadeEnable ^= true;
		fadeWhite = false;
		fadeFast = false;
	}
	
	/**
	 * Toggle a fast fade to/from black.
	 */
	
	void toggleBlackFast(void){
		fadeEnable ^= true;
		fadeWhite = false;
		fadeFast = true;
	}
	
	/**
	 * Toggle a slow fade to/from white.
	 */
	
	void toggleWhite(void){
		fadeEnable ^= true;
		fadeWhite = true;
		fadeFast = false;
	}
	
	/**
	 * Toggle a fast fade to/from white. This should only be used for battle
	 * initiations, so the 'battle start' sound is played as well.
	 */
	
	void toggleWhiteFast(void){
		fadeEnable ^= true;
		fadeWhite = true;
		fadeFast = true;
		
		Mix_PlayChannel(1, battleStart, 0);
	}
}
