#include <ui.h>

/*
 *	Create a macro to easily access SDL keypresses
*/

#define SDL_KEY e.key.keysym.sym

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

/*
 *	Freetype variables, and a GLuint for referencing rendered letters.
*/

static FT_Library   ftl;
static FT_Face      ftf;
static GLuint       ftex[93];
static vec2			ftexwh[93];
static vec2			ftexbl[93];
static vec2			ftexad[93];

static unsigned char fontColor[3] = {255,255,255};

/*
 *	Variables for dialog boxes / options.
*/

static char dialogBoxText[512];
static char *dialogOptText[4];
static float dialogOptLoc[4][3];
static unsigned char dialogOptCount = 0;
static bool typeOutDone = true;

static bool dialogImportant = false;

extern void mainLoop(void);

/*
 *	Toggled by pressing 'q', disables some controls when true.
*/

bool fadeEnable = false;
unsigned int fadeIntensity = 0;

bool inBattle = false;

namespace ui {
	
	/*
	 *	Mouse coordinates.
	*/
	
	vec2 mouse;

	/*
	 *	Variety of keydown bools
	*/
	bool edown;
	
	/*
	 *	Debugging flags.
	*/
	
	bool debug=false;
	bool posFlag=false;
	
	/*
	 *	Dialog stuff that needs to be 'public'.
	*/
	
	bool dialogBoxExists=false;
	unsigned char dialogOptChosen = 0;
	
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
		fontSize=0;
		memset(&ftex,0,93*sizeof(GLuint));
#ifdef DEBUG
		DEBUG_printf("Initialized FreeType2.\n",NULL);
#endif // DEBUG
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
		unsigned int i,j;
		char *buf;
		
		fontSize=size;
		FT_Set_Pixel_Sizes(ftf,0,fontSize);
		
		/*
		 *	Pre-render 'all' the characters.
		*/
		
		glDeleteTextures(93,ftex);	//	delete[] any already-rendered textures
		glGenTextures(93,ftex);		//	Generate new texture name/locations?
		
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
			
			buf = new char[ftf->glyph->bitmap.width * ftf->glyph->bitmap.rows * 4];	//(char *)malloc(ftf->glyph->bitmap.width*ftf->glyph->bitmap.rows*4);
		
			for(j=0;j<ftf->glyph->bitmap.width*ftf->glyph->bitmap.rows;j++){
				buf[j*4  ]=fontColor[0];
				buf[j*4+1]=fontColor[1];
				buf[j*4+2]=fontColor[2];
				buf[j*4+3]=ftf->glyph->bitmap.buffer[j];
			}
			
			ftexwh[i-33].x=ftf->glyph->bitmap.width;
			ftexwh[i-33].y=ftf->glyph->bitmap.rows;
			ftexbl[i-33].x=ftf->glyph->bitmap_left;
			ftexbl[i-33].y=ftf->glyph->bitmap_top;
			ftexad[i-33].x=ftf->glyph->advance.x>>6;
			ftexad[i-33].y=ftf->glyph->advance.y>>6;
		
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,ftf->glyph->bitmap.width,ftf->glyph->bitmap.rows,0,GL_RGBA,GL_UNSIGNED_BYTE,buf);	
			
			delete[] buf;	//free(buf);
		}
	}
	
	/*
	 *	Set a color for font rendering (default: white).
	*/
	
	void setFontColor(unsigned char r,unsigned char g,unsigned char b){
		fontColor[0]=r;
		fontColor[1]=g;
		fontColor[2]=b;
	}
	
	/*
	 *	Draws a character at the specified coordinates, aborting if the character is unknown.
	*/
	
	vec2 putChar(float x,float y,char c){
		vec2 c1,c2;
		
		/*
		 *	Get the width and height of the rendered character.
		*/
		
		c1={x+ftexbl[c-33].x,
		    y+ftexbl[c-33].y};
		c2=ftexwh[c-33];
		
		/*
		 *	Draw the character:
		*/
		
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,ftex[c-33]);
		glPushMatrix();
		glTranslatef(0,-c2.y,0);
		glBegin(GL_QUADS);
			glColor3ub(fontColor[0],fontColor[1],fontColor[2]);
			glTexCoord2f(0,1);glVertex2f(c1.x     ,c1.y		);
			glTexCoord2f(1,1);glVertex2f(c1.x+c2.x,c1.y		);
			glTexCoord2f(1,0);glVertex2f(c1.x+c2.x,c1.y+c2.y);
			glTexCoord2f(0,0);glVertex2f(c1.x     ,c1.y+c2.y);
		glEnd();
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);
		
		/*
		 * return the width.
		*/
		
		return ftexad[c-33];//(vec2){c2.x,ftexad[c-33].y};
	}
	
	/*
	 *	Draw a string at the specified coordinates.
	*/
	
	float putString(const float x,const float y,const char *s){
		unsigned int i=0;
		float xo=x,yo=y;
		vec2 add;
		
		/*
		 *	Loop on each character:
		*/
		
		do{
			if(s[i]=='\n'){			//	Handle newlines
				yo-=fontSize*1.05;
				xo=x;
			}else if(s[i]==' '){	//	Handle spaces
				xo+=fontSize/2;
			}else if(s[i]=='\b'){	//	Handle backspaces?
				xo-=add.x;
			}else{
				add=putChar(xo,yo,s[i]);
				xo+=add.x;
				yo+=add.y;
			}
		}while(s[++i]);
		
		return xo;	// i.e. the string width
	}
	
	float putStringCentered(const float x,const float y,const char *s){
		unsigned int i = 0;
		float width = 0;
		
		do{
			if(s[i]=='\n'){			//	Handle newlines
				// TODO
			}else if(s[i]==' '){	//	Handle spaces
				width+=fontSize/2;
			}else if(s[i]=='\b'){	//	Handle backspaces?
				// Why?
				// Cuz
			}else{
				width+=ftexwh[i].x+fontSize*.1;
			}
		}while(s[++i]);
		
		return putString(x-width/2,y,s);
	}
	
	/*
	 *	Draw a string in a typewriter-esque fashion. Each letter is rendered as calls are made
	 *	to this function. Passing a different string to the function will reset the counters.
	*/
	
	char *typeOut(char *str){
		static unsigned int sinc,	//	Acts as a delayer for the space between each character.
							linc=0,	//	Contains the number of letters that should be drawn.
							size=0;	//	Contains the full size of the current string.
		static char *ret = NULL;
		
		/*
		 *	Create a well-sized buffer if we haven't yet.
		*/
		
		if(!ret){
			ret = new char[512];	//(char *)calloc(512,sizeof(char));
			memset(ret,0,512*sizeof(char));
		}
		
		/*
		 *	Reset values if a new string is being passed.
		*/
		
		if(strncmp(ret,str,linc-1)){
			memset(ret,0,512);		//	Zero the buffer
			size=strlen(str);		//	Set the new target string size
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
			
			strncpy(ret+linc,str+linc,1);	//	Get next character
			
			if(linc<size)
				linc++;
			else typeOutDone = true;
		}
		
		return ret;		//	The buffered string.
	}
	
	/*
	 *	Draw a formatted string to the specified coordinates.
	*/
	
	float putText(const float x,const float y,const char *str,...){
		va_list args;
		char *buf;
		float width;
		
		/*
		 *	Create a wimpy buffer.
		*/
		
		buf = new char[512];	//(char *)calloc(128,sizeof(char));
		memset(buf,0,512*sizeof(char));
		
		/*
		 *	Handle the formatted string, printing it to the buffer.
		*/
		
		va_start(args,str);
		vsnprintf(buf,512,str,args);
		va_end(args);
		
		/*
		 *	Draw the string, free resources, return the width of the string.
		*/
		
		width=putString(x,y,buf);
		delete[] buf;	//free(buf);
		
		return width;
	}
	void dialogBox(const char *name,const char *opt,const char *text,...){
		va_list dialogArgs;
		unsigned int len;
		char *sopt,*soptbuf;
		
		/*
		 *	Set up the text buffer.
		*/
		
		memset(dialogBoxText,0,512);
		
		/*
		 *	Get the text ready for rendering.
		*/
		
		len=strlen(name);
		strcpy(dialogBoxText    ,name);
		strcpy(dialogBoxText+len,": ");
		len+=2;
		
		va_start(dialogArgs,text);
		vsnprintf(dialogBoxText+len,512-len,text,dialogArgs);
		va_end(dialogArgs);
				
		/*
		 *	Set up option text.
		*/
		
		while(dialogOptCount){
			if(dialogOptText[dialogOptCount]){
				delete[] dialogOptText[dialogOptCount];	//free(dialogOptText[dialogOptCount]);
				dialogOptText[dialogOptCount] = NULL;
			}
			dialogOptCount--;
		};

		dialogOptChosen=0;
		dialogOptCount=0;
		
		if(opt){
			
			soptbuf = new char[strlen(opt)+1];
			strcpy(soptbuf,opt);
			
			sopt=strtok(soptbuf,":");
			while(sopt != NULL){
				dialogOptText[dialogOptCount] = new char[strlen(sopt)+1];	//(char *)malloc(strlen(sopt));
				strcpy(dialogOptText[dialogOptCount++],sopt);
				sopt=strtok(NULL,":");
			}
			
			delete[] soptbuf;

		}
		
		/*
		 *	Tell draw() that the box is ready. 
		*/
		
		dialogBoxExists = true;
		dialogImportant = false;
		
	}
	void waitForDialog(void){
		do{
			mainLoop();
		}while(ui::dialogBoxExists);
	}
	void importantText(const char *text,...){
		va_list textArgs;
		
		//if(!player->ground)return;
		
		memset(dialogBoxText,0,512);
				
		va_start(textArgs,text);
		vsnprintf(dialogBoxText,512,text,textArgs);
		va_end(textArgs);
				  
		dialogBoxExists = true;
		dialogImportant = true;
		toggleBlack();
	}
	void draw(void){
		unsigned char i;
		float x,y;
		char *rtext;
		
		if(dialogBoxExists){
			
			rtext=typeOut(dialogBoxText);
			
			if(dialogImportant){
				setFontColor(255,255,255);
				if(fadeIntensity == 255){
					setFontSize(24);
					putStringCentered(offset.x,offset.y,rtext);
				}
			}else{
			
				glColor3ub(0,0,0);
			
				x=offset.x-SCREEN_WIDTH/2+HLINE*8;
				y=(offset.y+SCREEN_HEIGHT/2)-HLINE*8;
			
				glRectf(x,y,x+SCREEN_WIDTH-HLINE*16,y-SCREEN_HEIGHT/4);
			
				rtext=typeOut(dialogBoxText);
			
				putString(x+HLINE,y-fontSize-HLINE,rtext);
			
				for(i=0;i<dialogOptCount;i++){
					setFontColor(255,255,255);
					dialogOptLoc[i][1]=y-SCREEN_HEIGHT/4+(fontSize+HLINE)*(i+1);
					dialogOptLoc[i][2]=
					putStringCentered(offset.x,dialogOptLoc[i][1],dialogOptText[i]);
					dialogOptLoc[i][0]=offset.x-dialogOptLoc[i][2]/2;
					if(mouse.x > dialogOptLoc[i][0] &&
					   mouse.x < dialogOptLoc[i][0] + dialogOptLoc[i][2] &&
					   mouse.y > dialogOptLoc[i][1] &&
					   mouse.y < dialogOptLoc[i][1] + 16 ){ // fontSize
						  setFontColor(255,255,0);
						  putStringCentered(offset.x,dialogOptLoc[i][1],dialogOptText[i]);
					}
				}
				setFontColor(255,255,255);
			}
		}else if(!dialogImportant){
		
			vec2 hub = {
				(SCREEN_WIDTH/2+offset.x)-fontSize*10,
				(offset.y+SCREEN_HEIGHT/2)-fontSize
			};
			
			putText(hub.x,hub.y,"Health: %u/%u",player->health>0?(unsigned)player->health:0,
												(unsigned)player->maxHealth);
			if(player->alive){
				glColor3ub(255,0,0);
				hub.y-=fontSize*1.15;
				glRectf(hub.x,
						hub.y,
						hub.x+(player->health/player->maxHealth)*130,
						hub.y+12);
			}
			
			/*
			 *	Lists all of the quests the player is currently taking.
			*/
			
			if(player->inv->invOpen){
				hub.y = player->loc.y + fontSize * 8;
				hub.x = player->loc.x;
				
				putStringCentered(hub.x,hub.y,"Current Quests:");
				
				for(auto &c : player->qh.current){
					hub.y -= fontSize * 1.15;
					putString(hub.x,hub.y,c->title);
				}	
			}
		}
	}
	void handleEvents(void){
		static vec2 premouse={0,0};
		static int heyOhLetsGo = 0;
		unsigned char i;
		World *tmp;
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
				if((e.button.button&SDL_BUTTON_RIGHT)&&dialogBoxExists){
				
					if(!typeOutDone){
						typeOutDone = true;
						break;
					}
				
					for(i=0;i<dialogOptCount;i++){
						if(mouse.x > dialogOptLoc[i][0] &&
						   mouse.x < dialogOptLoc[i][2] &&
						   mouse.y > dialogOptLoc[i][1] &&
						   mouse.y < dialogOptLoc[i][1] + 16 ){ // fontSize
							dialogOptChosen = i + 1;
							goto DONE;
						}
					}
DONE:
					if(dialogImportant){
						dialogImportant = false;
						setFontSize(16);
						toggleBlack();
					}
					dialogBoxExists = false;
				}
				break;
			/*
				KEYDOWN
			*/
			case SDL_KEYDOWN:
				if(SDL_KEY == SDLK_ESCAPE){
					gameRunning = false;
					break;
				}else if(!dialogBoxExists){//&&!fadeEnable){
					switch(SDL_KEY){
					case SDLK_ESCAPE:
						gameRunning=false;
						break;
					case SDLK_a:
						player->vel.x=-.15;
						player->left = true;
						player->right = false;
						currentWorld=currentWorld->goWorldLeft(player);
						break;
					case SDLK_d:
						player->vel.x=.15;
						player->right = true;
						player->left = false;
						currentWorld=currentWorld->goWorldRight(player);
						break;
					case SDLK_s:
						if(player->ground == 2){
							player->ground=false;
							player->loc.y-=HLINE*1.5;
						}
						break;
					case SDLK_w:
						if(inBattle)
							 currentWorld=((Arena *)currentWorld)->exitArena(player);
						else currentWorld=currentWorld->goInsideStructure(player);
						break;
					case SDLK_SPACE:
						if(player->ground){
							player->vel.y=.4;
							player->loc.y+=HLINE*2;
							player->ground=false;
						}
						break;
					case SDLK_i:
						tmp=currentWorld;
						currentWorld=currentWorld->goWorldBack(player);	// Go back a layer if possible	
						if(tmp!=currentWorld){
							currentWorld->detect(player);
							player->vel.y=.2;
							player->loc.y+=HLINE*5;
							player->ground=false;
						}
						break;
					case SDLK_k:
						tmp=currentWorld;
						currentWorld=currentWorld->goWorldFront(player);	// Go forward a layer if possible
						if(tmp!=currentWorld){
							player->loc.y=0;
							currentWorld->behind->detect(player);
							player->vel.y=.2;
							player->ground=false;
						}
						break;
					case SDLK_LSHIFT:
						player->speed = debug ? 4 : 3;
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
				}
				break;
			/*
			 *	KEYUP
			*/
			
			case SDL_KEYUP:
				switch(SDL_KEY){
				case SDLK_a:
					player->left = false;
					break;
				case SDLK_d:
					player->right = false;
					break;
				case SDLK_LSHIFT:
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
				case SDLK_LEFT:
					if(player->inv->sel)player->inv->sel--;
					break;
				case SDLK_RIGHT:
					player->inv->sel++;
					break;
				default:
					break;
				}
				
				if(!player->left&&!player->right)
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
	
	void toggleBlack(void){
		fadeEnable ^= true;
	}
}
