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

static char *dialogBoxText			= NULL;
static char *dialogOptText[4];
static float dialogOptLoc[4][3];
static unsigned char dialogOptCount = 0;

/*
 *	Toggled by pressing 'q', disables some controls when true.
*/

bool fadeEnable = false;

namespace ui {
	
	/*
	 *	Mouse coordinates.
	*/
	
	vec2 mouse;
	
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
		
		glDeleteTextures(93,ftex);	//	Delete any already-rendered textures
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
			
			buf=(char *)malloc(ftf->glyph->bitmap.width*ftf->glyph->bitmap.rows*4);
		
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
			
			free(buf);
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
		unsigned int i=0,j;
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
		float width = 0, prev = 0;
		
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
		unsigned int i;
		
		/*
		 *	Create a well-sized buffer if we haven't yet.
		*/
		
		if(!ret) ret=(char *)calloc(512,sizeof(char));
		
		/*
		 *	Reset values if a new string is being passed.
		*/
		
		if(strncmp(ret,str,linc-1)){
			memset(ret,0,512);		//	Zero the buffer
			size=strlen(str);		//	Set the new target string size
			linc=0;					//	Reset the incrementers
			sinc=1;
		}
		
		/*
		 *	Draw the next letter if necessary.
		*/
		
		if(++sinc==2){
			sinc=0;
			
			strncpy(ret+linc,str+linc,1);	//	Get next character
			
			if(linc<size)linc++;
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
		
		buf=(char *)calloc(128,sizeof(char));
		
		/*
		 *	Handle the formatted string, printing it to the buffer.
		*/
		
		va_start(args,str);
		vsnprintf(buf,128,str,args);
		va_end(args);
		
		/*
		 *	Draw the string, free resources, return the width of the string.
		*/
		
		width=putString(x,y,buf);
		free(buf);
		
		return width;
	}
	
	void dialogBox(const char *name,char *opt,const char *text,...){
		va_list dialogArgs;
		unsigned int len;
		char *sopt;
		
		/*
		 *	Set up the text buffer.
		*/
		
		if(!dialogBoxText) dialogBoxText=(char *)malloc(512);
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
			if(dialogOptText[dialogOptCount])
				free(dialogOptText[dialogOptCount]);
			dialogOptCount--;
		};
		dialogOptChosen=0;
		dialogOptCount=0;
		
		sopt=strtok(opt,":");
		while(sopt != NULL){
			dialogOptText[dialogOptCount]=(char *)malloc(strlen(sopt));
			strcpy(dialogOptText[dialogOptCount++],sopt);
			sopt=strtok(NULL,":");
		}
		
		/*
		 *	Tell draw() that the box is ready. 
		*/
		
		dialogBoxExists = true;
		
	}
	void importantText(const char *text,...){
		va_list textArgs;
		char *ttext;
		if(!player->ground)return;
		va_start(textArgs,text);
		ttext=(char *)calloc(512,sizeof(char));
		vsnprintf(ttext,512,text,textArgs);
		va_end(textArgs);
		setFontSize(24);
		char *rtext;
		rtext=typeOut(ttext);
		putString(offset.x-SCREEN_WIDTH/2,
				  offset.y+fontSize,
				  rtext);
		free(ttext);
	}
	void draw(void){
		unsigned char i;
		float x,y;
		char *rtext;
		
		if(dialogBoxExists){
			
			glColor3ub(0,0,0);
			x=player->loc.x-SCREEN_WIDTH/2+HLINE*8;
			y=(offset.y+SCREEN_HEIGHT/2)-HLINE*8;
			
			glRectf(x,y,x+SCREEN_WIDTH-HLINE*16,y-SCREEN_HEIGHT/4);
			
			rtext=typeOut(dialogBoxText);
			
			putString(x+HLINE,y-fontSize-HLINE,rtext);
			
			for(i=0;i<dialogOptCount;i++){
				if(mouse.x > dialogOptLoc[i][0] &&
						   mouse.x < dialogOptLoc[i][2] &&
						   mouse.y > dialogOptLoc[i][1] &&
						   mouse.y < dialogOptLoc[i][1] + 16 ){ // fontSize
					  setFontColor(255,255,0);
				}else setFontColor(255,255,255);
				dialogOptLoc[i][1]=y-SCREEN_HEIGHT/4+(fontSize+HLINE)*(i+1);
				dialogOptLoc[i][2]=
				putStringCentered(player->loc.x,dialogOptLoc[i][1],dialogOptText[i]);
				dialogOptLoc[i][0]=player->loc.x-dialogOptLoc[i][2]/2;
			}
			setFontColor(255,255,255);
		}
		
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
		 * Lists all of the quests the player has
		*/
		
		hub.y-=fontSize*1.15;
		
		putString(hub.x,hub.y,"Current Quests:");

		for(auto &c : player->qh.current){
			hub.y-=fontSize*1.15;
			putString(hub.x,hub.y,c->title);
		}
	}
	void handleEvents(void){
		unsigned char i;
		static bool left=false,right=false;
		static vec2 premouse={0,0};
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
					dialogBoxExists=false;
					//dialogBoxText=NULL;
				}
				break;
			/*
				KEYDOWN
			*/
			case SDL_KEYDOWN:
				if(SDL_KEY==SDLK_ESCAPE)gameRunning=false;							// Exit the game with ESC
			if(!dialogBoxExists&&!fadeEnable){
				if(SDL_KEY==SDLK_a){												// Move left
					left=true;
					player->vel.x=-.15;
					player->left = true;
					player->right = false;
					currentWorld=currentWorld->goWorldLeft(player);
				}
				if(SDL_KEY==SDLK_d){												// Move right
					right=true;
					player->vel.x=.15;
					player->right = true;
					player->left = false;
					currentWorld=currentWorld->goWorldRight(player);
				}
				if(SDL_KEY==SDLK_s && player->ground==2){
					player->ground=false;
					player->loc.y-=HLINE*1.5;
				}
				if(SDL_KEY==SDLK_w)currentWorld=currentWorld->goInsideStructure(player);
				if(SDL_KEY==SDLK_SPACE){											// Jump
					if(player->ground){
						player->vel.y=.4;
						player->loc.y+=HLINE*2;
						player->ground=false;
					}
				}
				World *tmp;
				if(SDL_KEY==SDLK_i){
					tmp=currentWorld;
					currentWorld=currentWorld->goWorldBack(player);	// Go back a layer if possible	
					if(tmp!=currentWorld){
						currentWorld->detect(player);
						player->vel.y=.2;
						player->loc.y+=HLINE*5;
						player->ground=false;
					}
				}
				if(SDL_KEY==SDLK_k){
					tmp=currentWorld;
					currentWorld=currentWorld->goWorldFront(player);	// Go forward a layer if possible
					if(tmp!=currentWorld){
						player->loc.y=0;
						currentWorld->behind->detect(player);
						player->vel.y=.2;
						player->ground=false;
					}
				}
				if(SDL_KEY==SDLK_q){
					player->inv->itemToss();
				}
				if(SDL_KEY==SDLK_e){
					player->inv->useItem();
				}
				if(SDL_KEY==SDLK_LSHIFT)player->speed = debug?4:3;							// Sprint
				if(SDL_KEY==SDLK_LCTRL)player->speed = .5;
			}
				if(SDL_KEY==SDLK_p)toggleBlack();
				if(SDL_KEY==SDLK_F3)debug^=true;
				if(SDL_KEY==SDLK_b & SDL_KEY==SDLK_F3)posFlag^=true;
				if(SDL_KEY==SDLK_UP)handAngle++;
				if(SDL_KEY==SDLK_DOWN)handAngle--;
				break;
			/*
				KEYUP
			*/	
			case SDL_KEYUP:
				if(SDL_KEY==SDLK_a){left=false;}// Stop the player if movement keys are released
				if(SDL_KEY==SDLK_d){right=false;}
				if(!left&&!right)player->vel.x=0;
				if(SDL_KEY==SDLK_LSHIFT)player->speed = 1;
				if(SDL_KEY==SDLK_LCTRL)player->speed = 1;
				if(SDL_KEY==SDLK_h)player->health-=5;
				if(SDL_KEY==SDLK_f)player->light ^= true;
				if(SDL_KEY==SDLK_UP)handAngle+=0;
				if(SDL_KEY==SDLK_DOWN)handAngle-=0;

				break;
			default:
				break;
			}
		}
		
		if(player->inv->tossd)player->inv->itemToss();
		
		if(!dialogBoxExists&&AIpreaddr.size()){	// Flush preloaded AI functions if necessary
			for(i=0;i<AIpreaddr.size();i++){
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
