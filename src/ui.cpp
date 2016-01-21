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
static bool dialogPassive = false;
static bool typeOutDone = true;

Mix_Chunk *dialogClick;

extern void mainLoop(void);

/*
 *	Toggled by pressing 'q', disables some controls when true.
*/

bool fadeEnable = false;
bool fadeWhite = false;
bool fadeFast = false;
unsigned int fadeIntensity = 0;

bool inBattle = false;
Mix_Chunk *battleStart;

Mix_Chunk *sanic;

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
	
	bool dialogBoxExists = false;
	bool dialogImportant = false;
	unsigned char dialogOptChosen = 0;
	
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
		fontSize=0;
		memset(&ftex,0,93*sizeof(GLuint));
#ifdef DEBUG
		DEBUG_printf("Initialized FreeType2.\n",NULL);
#endif // DEBUG
		dialogClick = Mix_LoadWAV("assets/click.wav");
		battleStart = Mix_LoadWAV("assets/sounds/frig.wav");
		sanic = Mix_LoadWAV("assets/sounds/sanic.wav");
		Mix_Volume(1,50);
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
			if(i && ((i / 110.0) == (i / 110))){
				yo-=fontSize*1.05;
				xo=x;
				if(s[i] == ' ')
					i++;
			}
			if(s[i] == '\n' || s[i] == '\r' || s[i] == '\t'){
			/*if(s[i] == '\n'){
				yo-=fontSize*1.05;
				xo=x;
			*/}else if(s[i]==' '){	//	Handle spaces
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
		
		putString(x-width/2,y,s);
		return width;
	}
	
	/*
	 *	Draw a string in a typewriter-esque fashion. Each letter is rendered as calls are made
	 *	to this function. Passing a different string to the function will reset the counters.
	*/

	static char *ret = NULL;
	char *typeOut(char *str){
		static unsigned int sinc,	//	Acts as a delayer for the space between each character.
							linc=0,	//	Contains the number of letters that should be drawn.
							size=0;	//	Contains the full size of the current string.
		//static char *ret = NULL;
		
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
			else
				typeOutDone = true;
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
	void dialogBox(const char *name,const char *opt,bool passive,const char *text,...){
		va_list dialogArgs;
		unsigned int len;
		char *sopt,*soptbuf;
		
		dialogPassive = passive;
		
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

		dialogOptCount = 0;
		dialogOptChosen = 0;
		memset(&dialogOptLoc,0,sizeof(float)*12);
		
		if(opt != NULL){
			
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
		
		if(ret)
			ret[0] = '\0';
	}
	void waitForDialog(void){
		do{
			mainLoop();
		}while(ui::dialogBoxExists);
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
		float x,y,tmp;
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
			
				x=offset.x-SCREEN_WIDTH/2+HLINE*8;
				y=(offset.y+SCREEN_HEIGHT/2)-HLINE*8;
			
			
				glColor3ub(255,255,255);
				glBegin(GL_LINE_STRIP);
					glVertex2f(x-1						,y+1);
					glVertex2f(x+1+SCREEN_WIDTH-HLINE*16,y+1);
					glVertex2f(x+1+SCREEN_WIDTH-HLINE*16,y-1-SCREEN_HEIGHT/4);
					glVertex2f(x-1						,y-1-SCREEN_HEIGHT/4);
					glVertex2f(x						,y+1);
				glEnd();
			
				glColor3ub(0,0,0);
				glRectf(x,y,x+SCREEN_WIDTH-HLINE*16,y-SCREEN_HEIGHT/4);
			
				rtext=typeOut(dialogBoxText);
			
				putString(x+HLINE,y-fontSize-HLINE,rtext);
			
				for(i=0;i<dialogOptCount;i++){
					setFontColor(255,255,255);
					tmp = putStringCentered(offset.x,dialogOptLoc[i][1],dialogOptText[i]);
					dialogOptLoc[i][2] = offset.x + tmp;
					dialogOptLoc[i][0] = offset.x - tmp;
					dialogOptLoc[i][1] = y - SCREEN_HEIGHT / 4 + (fontSize + HLINE) * (i + 1);
					if(mouse.x > dialogOptLoc[i][0] &&
					   mouse.x < dialogOptLoc[i][2] &&
					   mouse.y > dialogOptLoc[i][1] &&
					   mouse.y < dialogOptLoc[i][1] + 16 ){ // fontSize
						  setFontColor(255,255,0);
						  putStringCentered(offset.x,dialogOptLoc[i][1],dialogOptText[i]);
					}
				}
				setFontColor(255,255,255);
			}
			
			if(strcmp(rtext,dialogBoxText)){
				Mix_PlayChannel(1,dialogClick,0);
			}
			
		}else if(!dialogImportant && !fadeIntensity){
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
						hub.x+(player->health/player->maxHealth?player->maxHealth:1)*130,
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
	void dialogAdvance(void){
		unsigned char i;
		if(!typeOutDone){
			typeOutDone = true;
			return;
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
	void handleEvents(void){
		static bool left=true,right=false;
		static vec2 premouse={0,0};
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
				if((e.button.button&SDL_BUTTON_RIGHT)&&dialogBoxExists){
					dialogAdvance();
				}
				if((e.button.button&SDL_BUTTON_LEFT)&&!dialogBoxExists){
					player->inv->usingi = true;
				}
				break;
			/*
				KEYDOWN
			*/
			case SDL_KEYDOWN:
				if(SDL_KEY == SDLK_ESCAPE){
					gameRunning = false;
					return;
				}else if(SDL_KEY == SDLK_SPACE){
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
					currentWorld->addStructure(FIRE_PIT, player->loc.x, player->loc.y, NULL);
					currentWorld->addLight({player->loc.x + SCREEN_WIDTH/2, player->loc.y},{1.0f,1.0f,1.0f});
					break;
				case SDLK_F12:
					std::cout << "Took screenshot" << std::endl;
					// Make the BYTE array, factor of 3 because it's RBG.
					static GLubyte* pixels = new GLubyte[ 3 * SCREEN_WIDTH * SCREEN_HEIGHT];
					glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels);

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
	
	void toggleBlack(void){
		fadeEnable ^= true;
		fadeWhite = false;
		fadeFast = false;
	}
	void toggleBlackFast(void){
		fadeEnable ^= true;
		fadeWhite = false;
		fadeFast = true;
	}
	void toggleWhite(void){
		fadeEnable ^= true;
		fadeWhite = true;
		fadeFast = false;
	}
	void toggleWhiteFast(void){
		fadeEnable ^= true;
		fadeWhite = true;
		fadeFast = true;
		Mix_PlayChannel(1,battleStart,0);
	}
}
