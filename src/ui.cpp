#include <ui.h>

/*
 *	Create a macro to easily access SDL keypresses
*/

#define SDL_KEY e.key.keysym.sym

extern std::vector<menuItem> optionsMenu;

extern SDL_Window *window;

/*
 *	External references for updating player coords / current world.
 */

extern Player *player;
extern World  *currentWorld;

/*
 *	In the case of dialog, some NPC quests can be preloaded so that they aren't assigned until
 *	the dialog box closes. Reference variables for that here.
*/

extern std::vector<int (*)(NPC *)> AIpreload;
extern std::vector<NPC *> AIpreaddr;

/*
 *	Pressing ESC or closing the window will set this to false.
*/

extern bool gameRunning;
extern unsigned int tickCount;

/*
 *	Freetype variables, and a GLuint for referencing rendered letters.
*/

static FT_Library   ftl;
static FT_Face      ftf;

typedef struct {
	vec2 wh;
	vec2 bl;
	vec2 ad;
} FT_Info;

static std::vector<FT_Info> ftdat ( 93, { { 0, 0 }, { 0, 0 }, { 0, 0 } } );
static std::vector<GLuint>  ftex  ( 93, 0 );

static unsigned char fontColor[4] = {255,255,255,255};

/*
 *	Variables for dialog boxes / options.
 */

static std::vector<std::pair<std::string,vec3>> dialogOptText;
static std::string dialogBoxText;
static vec3 merchArrowLoc[2];
static bool typeOutDone = true;

/*
 * Menu-related objects
 */

extern Menu* currentMenu;
extern Menu pauseMenu;


static Mix_Chunk *dialogClick;

extern void mainLoop(void);

/*
 * Fade effect flags
 */

bool fadeEnable = false;
bool fadeWhite  = false;
bool fadeFast   = false;
unsigned int fadeIntensity = 0;

bool inBattle = false;
Mix_Chunk *battleStart;

Mix_Chunk *sanic;

static GLuint pageTex = 0;

void Menu::gotoParent(){
	if(parent == NULL){
		currentMenu = NULL;
		updateConfig();
	}else{
		currentMenu = parent;
	}
}

void Menu::gotoChild(){
	if(child == NULL){
		currentMenu = NULL;
	}else{
		currentMenu = child;
	}
}

namespace ui {

	/*
	 *	Mouse coordinates.
	*/

	vec2 mouse;
	static vec2 premouse={0,0};

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

	/*
	 *	Initialises the Freetype library, and sets a font size.
	*/

	void initFonts(void){
		if(FT_Init_FreeType(&ftl)){
			std::cout<<"Error! Couldn't initialize freetype."<<std::endl;
			abort();
		}
		fontSize = 0;
#ifdef DEBUG
		DEBUG_printf("Initialized FreeType2.\n",NULL);
#endif // DEBUG
		dialogClick = Mix_LoadWAV("assets/sounds/click.wav");
		battleStart = Mix_LoadWAV("assets/sounds/frig.wav");
		sanic = Mix_LoadWAV("assets/sounds/sanic.wav");
		//Mix_Volume(1,50);
	}

	void destroyFonts(void){
		FT_Done_Face(ftf);
		FT_Done_FreeType(ftl);

		Mix_FreeChunk(dialogClick);
		Mix_FreeChunk(battleStart);
		Mix_FreeChunk(sanic);
	}

	/*
	 *	Sets a new font family to use (*.ttf).
	*/

	void setFontFace(const char *ttf){
		if(FT_New_Face(ftl,ttf,0,&ftf)){
			std::cout<<"Error! Couldn't open "<<ttf<<"."<<std::endl;
			abort();
		}
#ifdef DEBUG
		DEBUG_printf("Using font %s\n",ttf);
#endif // DEBUG
	}

	/*
	 *	Sets a new font size (default: 12).
	*/

	void setFontSize(unsigned int size){
		mtx.lock();
		unsigned int i,j;

		fontSize=size;
		FT_Set_Pixel_Sizes(ftf,0,fontSize);

		/*
		 *	Pre-render 'all' the characters.
		*/

		glDeleteTextures(93,ftex.data());	//	delete[] any already-rendered textures
		glGenTextures(93,ftex.data());		//	Generate new texture name/locations?

		for(i=33;i<126;i++){

			/*
			 *	Load the character from the font family file.
			*/

			if(FT_Load_Char(ftf,i,FT_LOAD_RENDER)){
				std::cout<<"Error! Unsupported character "<<(char)i<<" ("<<i<<")."<<std::endl;
				abort();
			}

			/*
			 *	Transfer the character's bitmap (?) to a texture for rendering.
			*/

			glBindTexture(GL_TEXTURE_2D,ftex[i-33]);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S		,GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T		,GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER	,GL_LINEAR		 );
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER	,GL_LINEAR		 );
			glPixelStorei(GL_UNPACK_ALIGNMENT,1);

			/*
			 *	The just-created texture will render red-on-black if we don't do anything to it, so
			 *	here we create a buffer 4 times the size and transform the texture into an RGBA array,
			 *	making it white-on-black.
			*/


			std::vector<uint32_t> buf ( ftf->glyph->bitmap.width * ftf->glyph->bitmap.rows, 0 );

			for( j = 0; j < buf.size(); j++ )
				buf[j] = 0x00FFFFFF | (ftf->glyph->bitmap.buffer[j] ? (0xFF << 24) : 0);

			ftdat[i-33].wh.x=ftf->glyph->bitmap.width;
			ftdat[i-33].wh.y=ftf->glyph->bitmap.rows;
			ftdat[i-33].bl.x=ftf->glyph->bitmap_left;
			ftdat[i-33].bl.y=ftf->glyph->bitmap_top;
			ftdat[i-33].ad.x=ftf->glyph->advance.x>>6;
			ftdat[i-33].ad.y=ftf->glyph->advance.y>>6;

			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,ftf->glyph->bitmap.width,ftf->glyph->bitmap.rows,0,GL_RGBA,GL_UNSIGNED_BYTE,buf.data());
		}
		mtx.unlock();
	}

	/*
	 *	Set a color for font rendering (default: white).
	*/

	void setFontColor(unsigned char r,unsigned char g,unsigned char b){
		fontColor[0]=r;
		fontColor[1]=g;
		fontColor[2]=b;
		fontColor[3]=255;
	}

	void setFontColor(unsigned char r,unsigned char g,unsigned char b, unsigned char a){
		fontColor[0]=r;
		fontColor[1]=g;
		fontColor[2]=b;
		fontColor[3]=a;
	}

	/*
	 *	Draws a character at the specified coordinates, aborting if the character is unknown.
	*/

	vec2 putChar(float xx,float yy,char c){
		vec2 c1,c2;

		int x = xx, y = yy;

		/*
		 *	Get the width and height of the rendered character.
		*/

		c1={(float)floor(x)+ftdat[c-33].bl.x,
		    (float)floor(y)+ftdat[c-33].bl.y};
		c2=ftdat[c-33].wh;

		/*
		 *	Draw the character:
		*/

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,ftex[c-33]);
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
		return ftdat[c-33].ad;
	}

	/*
	 *	Draw a string at the specified coordinates.
	*/

	float putString( const float x, const float y, std::string s ) {
		unsigned int i=0;
		vec2 add, o = {x, y};

		/*
		 *	Loop on each character:
		*/

		do{
			if(i && ((i / 110.0) == (i / 110))){
				o.y -= fontSize * 1.05f;
				o.x = x;
				if(s[i] == ' ')
					i++;
			}

			if(i && (i / (float)textWrapLimit == i / textWrapLimit)){
 				o.y -= fontSize * 1.05f;
 				o.x = x;

				// skip a space if it's there since we just newline'd
  				if(s[i] == ' ')
  					i++;
  			}

			switch ( s[i] ) {
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
				add = putChar( floor(o.x), floor(o.y), s[i] );
				o.x += add.x;
				o.y += add.y;
				break;
			}

		}while(s[++i]);

		return o.x;	// i.e. the string width
	}

	float putStringCentered( const float x, const float y, std::string s ) {
		unsigned int i = 0;
		float width = 0;

		do {
			switch ( s[i] ) {
			case '\n':
				// TODO
				break;
			case '\b':
				break;
			case ' ':
				width += fontSize / 2;
				break;
			default:
				width += ftdat[i].wh.x + fontSize * 0.1f;
				break;
			}
		} while(s[++i]);
		putString(floor(x-width/2),y,s);
		return width;
	}

	/*
	 *	Draw a string in a typewriter-esque fashion. Each letter is rendered as calls are made
	 *	to this function. Passing a different string to the function will reset the counters.
	*/

	std::string ret;
	std::string typeOut( std::string str ) {
		static unsigned int sinc,	//	Acts as a delayer for the space between each character.
							linc=0,	//	Contains the number of letters that should be drawn.
							size=0;	//	Contains the full size of the current string.

		/*
		 *	Reset values if a new string is being passed.
		*/

		if(strncmp(ret.c_str(),str.c_str(),linc-1)){
			ret.clear();			//	Zero the buffer
			size=str.size();		//	Set the new target string size
			linc=0;					//	Reset the incrementers
			sinc=1;
			typeOutDone = false;
		}

		/*
		 *	Draw the next letter if necessary.
		*/

		if(typeOutDone)
			return str;
		else if(++sinc==2){
			sinc=0;

			ret.append(str, linc, 1);

			if(linc<size)
				linc++;
			else
				typeOutDone = true;
		}

		return ret;		//	The buffered string.
	}

	/*
	 *	Draw a formatted string to the specified coordinates.
	*/

	float putText( const float x, const float y, const char *str, ... ) {
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
		return putString( x, y, buf.get() );
	}

	void dialogBox( const char *name, const char *opt, bool passive, const char *text, ... ) {
		va_list dialogArgs;
		std::unique_ptr<char[]> printfbuf (new char[512]);

		textWrapLimit = 110;
		dialogPassive = passive;

		// reset & add speaker prefix
		dialogBoxText.clear();
		dialogBoxText = (std::string)name + ": ";

		// handle the formatted string
		va_start(dialogArgs,text);
		vsnprintf(printfbuf.get(),512,text,dialogArgs);
		va_end(dialogArgs);
		dialogBoxText += printfbuf.get();

		// setup option text
		dialogOptText.clear();

		dialogOptChosen = 0;

		if ( opt ) {
			std::string soptbuf = opt;
			char *sopt = strtok(&soptbuf[0], ":");

			// cycle through options
			while(sopt){
				dialogOptText.push_back(std::make_pair((std::string)sopt, vec3 {0,0,0}) );
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


	void merchantBox(const char *name,Trade trade,const char *opt,bool passive,const char *text,...){
		va_list dialogArgs;
		std::unique_ptr<char[]> printfbuf (new char[512]);

		std::cout << "Buying and selling on the bi-weekly!" << std::endl;

		dialogPassive = passive;

		std::cout << "Market Trading: " << trade.quantity[0] << " " << trade.item[0] << " for " << trade.quantity[1] << " " << trade.item[1] << std::endl;

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
		if(opt){
			std::string soptbuf = opt;
			char *sopt = strtok(&soptbuf[0], ":");

			// cycle through options
			while(sopt){
				dialogOptText.push_back(std::make_pair((std::string)sopt, vec3 {0,0,0}) );
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

	void merchantBox(){
		textWrapLimit = 50;
		dialogMerchant = true;
	}

	/**
	 * Wait for a dialog box to be dismissed.
	 */

	void waitForDialog(void){
		do{
			//std::thread(dialogAdvance);
			//mainLoop();
		}while(dialogBoxExists);
	}
	void waitForCover(void){
		do{
			mainLoop();
		}while(fadeIntensity < 255);
		fadeIntensity = 255;
	}
	void waitForNothing(unsigned int ms){
		unsigned int target = millis() + ms;
		do{
			mainLoop();
		}while(millis() < target);
	}
	void importantText(const char *text,...){
		va_list textArgs;
		char *printfbuf;

		//if(!player->ground)return;

		//memset(dialogBoxText,0,512);
		dialogBoxText.clear();

		printfbuf = new char[ 512 ];
		va_start(textArgs,text);
		vsnprintf(printfbuf,512,text,textArgs);
		va_end(textArgs);
		dialogBoxText = printfbuf;
		delete[] printfbuf;

		dialogBoxExists = true;
		dialogImportant = true;
		//toggleBlack();
	}

	void passiveImportantText(int duration, const char *text,...){
		va_list textArgs;
		char *printfbuf;

		//if(!player->ground)return;

		//memset(dialogBoxText,0,512);
		dialogBoxText.clear();

		printfbuf = new char[ 512 ];
		va_start(textArgs,text);
		vsnprintf(printfbuf,512,text,textArgs);
		va_end(textArgs);
		dialogBoxText = printfbuf;
		delete[] printfbuf;

		dialogBoxExists = true;
		dialogImportant = true;
		dialogPassive = true;
		dialogPassiveTime = duration;
	}


	void drawPage( std::string path ) {
		pageTex = Texture::loadTexture( path );
	}

	void draw(void){
		unsigned char i;
		float x,y,tmp;
		std::string rtext;

		if ( pageTex ) {

			glEnable( GL_TEXTURE_2D);
			glBindTexture( GL_TEXTURE_2D, pageTex );
			glBegin( GL_QUADS );
				glTexCoord2i( 0, 0 ); glVertex2i( offset.x - 300, SCREEN_HEIGHT - 100 );
				glTexCoord2i( 1, 0 ); glVertex2i( offset.x + 300, SCREEN_HEIGHT - 100 );
				glTexCoord2i( 1, 1 ); glVertex2i( offset.x + 300, SCREEN_HEIGHT - 600 );
				glTexCoord2i( 0, 1 ); glVertex2i( offset.x - 300, SCREEN_HEIGHT - 600 );
			glEnd();
			glDisable( GL_TEXTURE_2D);

		} else if (dialogBoxExists){

			rtext=typeOut(dialogBoxText);

			if(dialogImportant){
				setFontColor(255,255,255);
				if(dialogPassive){
					dialogPassiveTime -= deltaTime;
					if(dialogPassiveTime < 0){
						dialogPassive = false;
						dialogImportant = false;
						dialogBoxExists = false;
					}
				}
				if(fadeIntensity == 255 || dialogPassive){
					//setFontSize(24);
					putStringCentered(offset.x,offset.y,rtext.c_str());
					//setFontSize(16);
				}
			}else if(dialogMerchant){
				//static int dispItem;

				x=offset.x-SCREEN_WIDTH/6;
				y=(offset.y+SCREEN_HEIGHT/2)-HLINE*8;


				glColor3ub(255,255,255);
				glBegin(GL_LINE_STRIP);
					glVertex2f(x-1 				   ,y+1);
					glVertex2f(x+1+(SCREEN_WIDTH/3),y+1);
					glVertex2f(x+1+(SCREEN_WIDTH/3),y-1-SCREEN_HEIGHT*.6);
					glVertex2f(x-1,y-1-SCREEN_HEIGHT*.6);
					glVertex2f(x - 1,y+1);
				glEnd();

				glColor3ub(0,0,0);
				glRectf(x,y,x+SCREEN_WIDTH/3,y-SCREEN_HEIGHT*.6);

				// draw typeOut'd text
				putString(x + HLINE, y - fontSize - HLINE, (rtext = typeOut(dialogBoxText)));

				std::string itemString1 = std::to_string(merchTrade.quantity[0]);
				itemString1 += "x";

				std::string itemString2 = std::to_string(merchTrade.quantity[1]);
				itemString2 += "x";

				putStringCentered(offset.x - (SCREEN_WIDTH / 10) + 20, offset.y + (SCREEN_HEIGHT / 5) + 40 + (fontSize*2), itemString1.c_str());
				putStringCentered(offset.x - (SCREEN_WIDTH / 10) + 20, offset.y + (SCREEN_HEIGHT / 5) + 40 + fontSize, merchTrade.item[0].c_str());

				putStringCentered(offset.x + (SCREEN_WIDTH / 10) - 20, offset.y + (SCREEN_HEIGHT / 5) + 40 + (fontSize*2), itemString2.c_str());
				putStringCentered(offset.x + (SCREEN_WIDTH / 10) - 20, offset.y + (SCREEN_HEIGHT / 5) + 40 + fontSize, merchTrade.item[1].c_str());

				putStringCentered(offset.x,offset.y + (SCREEN_HEIGHT / 5) + 60, "for");

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

				for(i = 0; i < 2; i++){
					if(((merchArrowLoc[i].x < merchArrowLoc[i].z) ?
						(mouse.x > merchArrowLoc[i].x     && mouse.x < merchArrowLoc[i].z) :
						(mouse.x < merchArrowLoc[i].x     && mouse.x > merchArrowLoc[i].z)    ) &&
					     mouse.y > merchArrowLoc[i].y - 8 && mouse.y < merchArrowLoc[i].y + 8 ) {
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
				for(i = 0; i < dialogOptText.size(); i++){
					setFontColor(255, 255, 255);

					// draw option
					tmp = putStringCentered(offset.x, dialogOptText[i].second.y, dialogOptText[i].first);

					// get coordinate information on option
					dialogOptText[i].second.z = offset.x + tmp;
					dialogOptText[i].second.x = offset.x - tmp;
					dialogOptText[i].second.y = y - SCREEN_HEIGHT / 2 - (fontSize + HLINE) * (i + 1);

					// make text yellow if the mouse hovers over the text
					if(mouse.x > dialogOptText[i].second.x && mouse.x < dialogOptText[i].second.z &&
					   mouse.y > dialogOptText[i].second.y && mouse.y < dialogOptText[i].second.y + 16 ){
						  setFontColor(255, 255, 0);
						  putStringCentered(offset.x, dialogOptText[i].second.y, dialogOptText[i].first);
					}
				}

				setFontColor(255, 255, 255);
			}else{ //normal dialog box

				x=offset.x-SCREEN_WIDTH/2+HLINE*8;
				y=(offset.y+SCREEN_HEIGHT/2)-HLINE*8;

				// draw white border
				glColor3ub(255, 255, 255);

				glBegin(GL_LINE_STRIP);
					glVertex2i(x-1						,y+1);
					glVertex2i(x+1+SCREEN_WIDTH-HLINE*16,y+1);
					glVertex2i(x+1+SCREEN_WIDTH-HLINE*16,y-1-SCREEN_HEIGHT/4);
					glVertex2i(x-1						,y-1-SCREEN_HEIGHT/4);
					glVertex2i(x-1						,y+1);
				glEnd();

				glColor3ub(0,0,0);
				glRectf(x,y,x+SCREEN_WIDTH-HLINE*16,y-SCREEN_HEIGHT/4);

				rtext=typeOut(dialogBoxText);

				putString(x+HLINE,y-fontSize-HLINE,rtext);

				for(i=0;i<dialogOptText.size();i++){
					setFontColor(255,255,255);
					tmp = putStringCentered(offset.x,dialogOptText[i].second.y,dialogOptText[i].first);
					dialogOptText[i].second.z = offset.x + tmp;
					dialogOptText[i].second.x = offset.x - tmp;
					dialogOptText[i].second.y = y - SCREEN_HEIGHT / 4 + (fontSize + HLINE) * (i + 1);
					if(mouse.x > dialogOptText[i].second.x &&
					   mouse.x < dialogOptText[i].second.z &&
					   mouse.y > dialogOptText[i].second.y &&
					   mouse.y < dialogOptText[i].second.y + 16 ){ // fontSize
						  setFontColor(255,255,0);
						  putStringCentered(offset.x,dialogOptText[i].second.y,dialogOptText[i].first);
					}
				}
				setFontColor(255,255,255);
			}

			if ( rtext != dialogBoxText )
				Mix_PlayChannel(1,dialogClick,0);

		}if(!fadeIntensity){
			vec2 hub = {
				(SCREEN_WIDTH/2+offset.x)-fontSize*10,
				(offset.y+SCREEN_HEIGHT/2)-fontSize
			};

			putText(hub.x,hub.y,"Health: %u/%u",player->health>0?(unsigned)player->health:0,
												(unsigned)player->maxHealth
												);
			if(player->alive){
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
			if(player->inv->invOpen){
				hub.y = player->loc.y + fontSize * 8;
				hub.x = player->loc.x;// + player->width / 2;

				putStringCentered(hub.x,hub.y,"Current Quests:");

				for(auto &c : player->qh.current){
					hub.y -= fontSize * 1.15;
					putStringCentered(hub.x,hub.y,c.title.c_str());
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

	void quitGame(){
		dialogBoxExists = false;
		currentMenu = NULL;
		delete[] currentMenu;
		gameRunning = false;
		updateConfig();
		saveConfig();
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
		std::vector<GLubyte> bgr (SCREEN_WIDTH * SCREEN_HEIGHT * 3, 0);

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

	void closeBox(){
		dialogBoxExists = false;
		dialogMerchant = false;
	}

	void dialogAdvance(void){
		unsigned char i;

		if ( pageTex ) {
			glDeleteTextures( 1, &pageTex );
			pageTex = 0;
			return;
		}

		if(!typeOutDone){
			typeOutDone = true;
			return;
		}

		for(i=0;i<dialogOptText.size();i++){
			if(mouse.x > dialogOptText[i].second.x &&
			   mouse.x < dialogOptText[i].second.z &&
			   mouse.y > dialogOptText[i].second.y &&
			   mouse.y < dialogOptText[i].second.y + 16 ){ // fontSize
				dialogOptChosen = i + 1;
				goto EXIT;
			}
		}
		if(dialogMerchant){
			for(i=0;i<2;i++){
				if(((merchArrowLoc[i].x < merchArrowLoc[i].z) ?
					(mouse.x > merchArrowLoc[i].x && mouse.x < merchArrowLoc[i].z) :
				    (mouse.x < merchArrowLoc[i].x && mouse.x > merchArrowLoc[i].z) &&
					 mouse.y > merchArrowLoc[i].y - 8 && mouse.y < merchArrowLoc[i].y + 8)){
						merchOptChosen = i + 1;
						goto EXIT;
				}
			}
		}


EXIT:
		//if(!dialogMerchant)closeBox();
		dialogBoxExists = false;
		dialogMerchant = false;

		//DONE:

		// handle important text
		if(dialogImportant){
			dialogImportant = false;
			setFontSize(16);
		}
	}

	void handleEvents(void){
		static bool left=true,right=false;
		static int heyOhLetsGo = 0;
		static int mouseWheelUpCount = 0, mouseWheelDownCount = 0;
		World *tmp;
		vec2 oldpos,tmppos;
		SDL_Event e;

		// update mouse coords
		mouse.x = premouse.x + offset.x - ( SCREEN_WIDTH / 2 );
		mouse.y = ( offset.y + SCREEN_HEIGHT / 2 ) - premouse.y;

		while(SDL_PollEvent(&e)){
			switch(e.type){

			// escape - quit game
			case SDL_QUIT:
				gameRunning=false;
				break;

			// mouse movement - update mouse vector
			case SDL_MOUSEMOTION:
				premouse.x=e.motion.x;
				premouse.y=e.motion.y;
				break;

			// mouse clicks
			case SDL_MOUSEBUTTONDOWN:
				// right click advances dialog
				if ( ( e.button.button & SDL_BUTTON_RIGHT ) && (dialogBoxExists | pageTex) )
					dialogAdvance();
				// left click uses item
				if ( ( e.button.button & SDL_BUTTON_LEFT ) && !dialogBoxExists )
					player->inv->usingi = true;
				break;
			case SDL_MOUSEWHEEL:
				if (e.wheel.y < 0){
					if(mouseWheelUpCount++ && mouseWheelUpCount%5==0){
						player->inv->setSelectionUp();
						mouseWheelUpCount = 0;
					}
				}else{
					if(mouseWheelDownCount-- && mouseWheelDownCount%5==0){
						player->inv->setSelectionDown();
						mouseWheelDownCount = 0;
					}
				}
				break;
			// key presses
			case SDL_KEYDOWN:

				// space - make player jump
				if ( SDL_KEY == SDLK_SPACE ) {
					if ( player->ground ) {
						player->loc.y += HLINE * 2;
						player->vel.y = .4;
						player->ground = false;
					}
					break;

				// only let other keys be handled if dialog allows it
				} else if ( !dialogBoxExists || dialogPassive ) {
					tmp = currentWorld;
					switch(SDL_KEY){
					case SDLK_t:
						tickCount += 50;
						break;
					case SDLK_a:
						if(fadeEnable)break;
						player->vel.x=-.15;
						player->left = true;
						player->right = false;
						left = true;
						right = false;
						if ( !currentWorld->toLeft.empty() ) {
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
						if ( !currentWorld->toRight.empty() ) {
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
						if ( inBattle ) {
							tmp = currentWorld;
							currentWorld = ((Arena *)currentWorld)->exitArena( player );
							if ( tmp != currentWorld )
								toggleBlackFast();
						} else if( (tmp = currentWorld->goInsideStructure( player )) != currentWorld )
								currentWorld = tmp;
						break;
					case SDLK_i:
						player->health -= 5;
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
						edown=true;
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
					edown=false;
					if(player->inv->invHover){
						player->inv->invHover = false;
					}else{
						if(!player->inv->selected)player->inv->invOpening ^= true;
						else player->inv->selected = false;
						player->inv->mouseSel = false;
					}
					heyOhLetsGo = 0;
					break;
				case SDLK_l:
					currentWorld->addLight({player->loc.x + SCREEN_WIDTH/2, player->loc.y},{1.0f,1.0f,1.0f});
					currentWorld->light.back().belongsTo = true;
					currentWorld->light.back().following = player;
					currentWorld->light.back().flame = true;
					break;
				case SDLK_f:
					currentWorld->addLight({player->loc.x + SCREEN_WIDTH/2, player->loc.y},{1.0f,1.0f,1.0f});
					std::cout << currentWorld->light.back().belongsTo << std::endl;
					currentWorld->light.back().belongsTo = false;
					std::cout << currentWorld->light.back().belongsTo << std::endl;
					currentWorld->light.back().following = nullptr;
					currentWorld->light.back().flame = true;
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
					currentWorld->addStructure(FIRE_PIT, player->loc.x, player->loc.y, "", "");
					currentWorld->addLight({player->loc.x + SCREEN_WIDTH/2, player->loc.y},{1.0f,1.0f,1.0f});
					currentWorld->light.back().belongsTo = false;
					currentWorld->light.back().following = nullptr;
					currentWorld->light.back().flame = true;
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

			// Flush preloaded AI functions if necessary
		if ( !dialogBoxExists && AIpreaddr.size() ) {
			while ( !AIpreaddr.empty() ) {
				AIpreaddr.front()->addAIFunc( AIpreload.front(), false );
				AIpreaddr.erase( AIpreaddr.begin() );
				AIpreload.erase( AIpreload.begin() );
			}
		}
	}

	void toggleBlack(void){
		fadeEnable ^= true;
		fadeWhite   = false;
		fadeFast    = false;
	}
	void toggleBlackFast(void){
		fadeEnable ^= true;
		fadeWhite   = false;
		fadeFast    = true;
	}
	void toggleWhite(void){
		fadeEnable ^= true;
		fadeWhite   = true;
		fadeFast    = false;
	}
	void toggleWhiteFast(void){
		fadeEnable ^= true;
		fadeWhite   = true;
		fadeFast    = true;

		Mix_PlayChannel( 1, battleStart, 0 );
	}
}
