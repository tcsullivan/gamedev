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
static GLuint       ftex;

static unsigned char fontColor[3] = {255,255,255};

/*
 *	Variables for dialog boxes / options.
*/

static char *dialogBoxText			= NULL;
static char *dialogOptText[4];
static int   dialogOptLoc[4][3];
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
		fontSize=12;
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
		fontSize=size;
		FT_Set_Pixel_Sizes(ftf,0,fontSize);
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
	
	float putChar(float x,float y,char c){
		unsigned int i;
		char *buf;
		float w,h;
		
		/*
		 *	Load the character from the font family library.
		*/
		
		if(FT_Load_Char(ftf,c,FT_LOAD_RENDER)){
			std::cout<<"Error! Unsupported character "<<c<<" ("<<(int)c<<")."<<std::endl;
			abort();
		}
		
		/*
		 *	Load the character into a texture for rendering.
		*/
		
		//glActiveTexture(GL_TEXTURE0);
		glGenTextures(1,&ftex);
		glBindTexture(GL_TEXTURE_2D,ftex);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		
		/*
		 *	The just-created texture will render red-on-black if we don't do anything to it, so
		 *	here we create a buffer 4 times the size and transform the texture into an RGBA array,
		 *	making it white-on-black.
		*/
		
		buf=(char *)malloc(ftf->glyph->bitmap.width*ftf->glyph->bitmap.rows*4);
		
		for(i=0;i<ftf->glyph->bitmap.width*ftf->glyph->bitmap.rows;i++){
			buf[i*4  ]=fontColor[0];
			buf[i*4+1]=fontColor[1];
			buf[i*4+2]=fontColor[2];
			buf[i*4+3]=ftf->glyph->bitmap.buffer[i]?255:0;
		}
		
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,ftf->glyph->bitmap.width,ftf->glyph->bitmap.rows,0,GL_RGBA,GL_UNSIGNED_BYTE,buf);
		
		/*
		 *	Get the width and height of the rendered character.
		*/
		
		w=ftf->glyph->bitmap.width;
		h=ftf->glyph->bitmap.rows;
		
		/*
		 *	Draw the character:
		*/
		
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,ftex);
		
		switch(c){						//		We're bad, so in here we adjust the y of
		case '^':						//		some letters so that they look like they
		case '*':						//		fit.
		case '`':
		case '\'':
		case '\"':
		case '-':y+=fontSize/3;break;
		case '~':
		case '<':
		case '>':
		case '+':
		case '=':y+=fontSize/5;break;
		case 'g':
		case 'q':
		case 'y':
		case 'p':
		case 'j':y-=fontSize/4;break;
		case 'Q':y-=fontSize/5;break;
		default:break;
		}
		
		glBegin(GL_QUADS);
			glColor3ub(255,255,255);
			glTexCoord2f(0,1);glVertex2f(x,y);
			glTexCoord2f(1,1);glVertex2f(x+w,y);
			glTexCoord2f(1,0);glVertex2f(x+w,y+h);
			glTexCoord2f(0,0);glVertex2f(x,y+h);
		glEnd();
		
		glDisable(GL_TEXTURE_2D);
		
		/*
		 *	Free resources, and return the width.
		*/
		
		free(buf);
		glDeleteTextures(1,&ftex);
		
		return w;
	}
	
	/*
	 *	Draw a string at the specified coordinates.
	*/
	
	float putString(const float x,const float y,const char *s){
		unsigned int i=0,j;
		float xo=x,yo=y,pw=0;
		
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
				xo-=pw;
			}else{
				pw=putChar(xo,yo,s[i])+fontSize*.1;
				xo+=pw;
			}
		}while(s[++i]);
		
		return xo;	// i.e. the string width
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
		
		if(!size || ((linc>15)&(strncmp(ret,str,15)))){
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
			
			setFontSize(16);
			putString(x+HLINE,y-fontSize-HLINE,rtext);
			
			for(i=0;i<dialogOptCount;i++){
				if(mouse.x > dialogOptLoc[i][0] &&
						   mouse.x < dialogOptLoc[i][2] &&
						   mouse.y > dialogOptLoc[i][1] &&
						   mouse.y < dialogOptLoc[i][1] + 16 ){ // fontSize
					  setFontColor(255,255,0);
				}else setFontColor(255,255,255);
				dialogOptLoc[i][0]=x+HLINE;
				dialogOptLoc[i][1]=y-SCREEN_HEIGHT/4+(fontSize+HLINE)*(i+1);
				dialogOptLoc[i][2]=
			
				putString(x+HLINE,y-SCREEN_HEIGHT/4+(fontSize+HLINE)*(i+1),dialogOptText[i]);
			}
			setFontColor(255,255,255);
		}
		
		setFontSize(14);
		putText(((SCREEN_WIDTH/2)+offset.x)-125,(offset.y+SCREEN_HEIGHT/2)-fontSize,"Health: %u/%u",player->health>0?(unsigned)player->health:0,
								
																							(unsigned)player->maxHealth);
		if(player->alive){
			glColor3ub(255,0,0);
			glRectf((SCREEN_WIDTH/2+offset.x)-125,
					(offset.y+SCREEN_HEIGHT/2)-32,
					((SCREEN_WIDTH/2+offset.x)-125)+((player->health/player->maxHealth)*100),
					(offset.y+SCREEN_HEIGHT/2)-32+12);
		}

		/*
		 * Lists all of the quests the player has
		*/
		putText(((SCREEN_WIDTH/2)+offset.x)-125,(offset.y+SCREEN_HEIGHT/2)-fontSize*4, "Current Quests:",NULL);

		for(auto &c : player->qh.current){
			putText(((SCREEN_WIDTH/2)+offset.x)-125,(offset.y+SCREEN_HEIGHT/2)-fontSize*5, "%s",c->title);
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
