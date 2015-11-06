#include <ui.h>

#define SDL_KEY e.key.keysym.sym	// Keeps the code neater :)

extern Player *player;			// 'player' should be (must be) defined in main.cpp
extern World  *currentWorld;	// should/must also be defined in main.cpp

extern std::vector<int (*)(NPC *)> AIpreload;	// see entities.cpp
extern std::vector<NPC *> AIpreaddr;			//

extern bool gameRunning;

static FT_Library   ftl;		// Variables for the FreeType library and stuff
static FT_Face      ftf;
static GLuint       ftex;

static char *dialogBoxText;

bool fadeEnable = false;

namespace ui {
	vec2 mouse;
	bool debug=false;
	bool posFlag=false;
	bool dialogBoxExists=false;
	unsigned int fontSize;

	/*
	 * initFonts(), setFontFace(), and setFontSize() are pretty self-explanatory
	*/

	void initFonts(void){
		if(FT_Init_FreeType(&ftl)){
			std::cout<<"Error! Couldn't initialize freetype."<<std::endl;
			abort();
		}
		fontSize=12; // to be safe
#ifdef DEBUG
		DEBUG_printf("Initialized FreeType2.\n",NULL);
#endif // DEBUG
	}
	void setFontFace(const char *ttf){
		if(FT_New_Face(ftl,ttf,0,&ftf)){
			std::cout<<"Error! Couldn't open "<<ttf<<"."<<std::endl;
			abort();
		}
#ifdef DEBUG
		DEBUG_printf("Using font %s\n",ttf);
#endif // DEBUG
	}
	void setFontSize(unsigned int size){
		fontSize=size;
		FT_Set_Pixel_Sizes(ftf,0,fontSize);
	}
	float putChar(float x,float y,char c){
		unsigned int j;
		char *buf;
		float w,h;
		// Load the first/next character (if possible)
		if(FT_Load_Char(ftf,c,FT_LOAD_RENDER)){
			std::cout<<"Error! Unsupported character "<<c<<" ("<<(int)c<<")."<<std::endl;
			abort();
		}
		// Load the bitmap with OpenGL
		//glActiveTexture(GL_TEXTURE0);
		glGenTextures(1,&ftex);
		glBindTexture(GL_TEXTURE_2D,ftex);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		/*
		 *	ftf->glyph->bitmap.buffer simply stores a bitmap of the character,
		 * 	and if OpenGL tries to load it directly it'll mistake it as a simple
		 * 	red on black texture. Here we allocate enough space to convert this
		 * 	bitmap to an RGBA-type buffer, also making the text white on black.
		 * 
		 * 	TODO: allow different colors
		*/
		buf=(char *)malloc(ftf->glyph->bitmap.width*ftf->glyph->bitmap.rows*4);
		for(j=0;j<ftf->glyph->bitmap.width*ftf->glyph->bitmap.rows;j++){
			buf[j*4]=255;
			buf[j*4+1]=255;
			buf[j*4+2]=255;
			buf[j*4+3]=ftf->glyph->bitmap.buffer[j]?255:0;
		}
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,ftf->glyph->bitmap.width,ftf->glyph->bitmap.rows,0,GL_RGBA,GL_UNSIGNED_BYTE,buf);
		// Draw the texture and move the cursor
		w=ftf->glyph->bitmap.width;
		h=ftf->glyph->bitmap.rows;
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,ftex);
		switch(c){	// Some characters are not properly spaced, make them so here
		case '^':
		case '*':
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
		// Free the RGBA buffer and the OpenGL texture
		free(buf);
		glDeleteTextures(1,&ftex);
		return w;
	}
	float putString(const float x,const float y,const char *s){
		unsigned int i=0,j;
		float xo=x,yo=y,pw=0;
		do{
			if(s[i]=='\n'){
				yo-=fontSize*1.05;
				xo=x;
			}else if(s[i]==' '){
				xo+=fontSize/2;
			}else if(s[i]=='\b'){
				xo-=pw;
			}else{
				pw=putChar(xo,yo,s[i])+fontSize*.1;
				xo+=pw;
			}
		}while(s[++i]);
		return xo;
	}
	char *typeOut(char *str){
		static unsigned int sinc,linc,size=0;
		static char *ret = NULL;
		unsigned int i;
		if(!ret)ret=(char *)calloc(512,sizeof(char));
		if(size != strlen(str)){
			memset(ret,0,512);
			size=strlen(str);
			linc=0;
			sinc=1;
		}
		if(++sinc==2){
			sinc=0;
			strncpy(ret+linc,str+linc,1);
			if(linc<size)linc++;
		}
		return ret;
	}
	float putText(const float x,const float y,const char *str,...){	// putText() simply runs 'str' and the extra arguments though
		va_list args;												// vsnprintf(), which'll store the complete string to a buffer
		char *buf;													// that's then passed to putString()
		float width;
		buf=(char *)calloc(128,sizeof(char));
		va_start(args,str);
		vsnprintf(buf,128,str,args);
		va_end(args);
		width=putString(x,y,buf);
		free(buf);
		return width;
	}
	void dialogBox(const char *name,const char *text,...){
		unsigned int name_len;
		va_list dialogArgs;
		va_start(dialogArgs,text);
		dialogBoxExists=true;
		if(dialogBoxText){
			free(dialogBoxText);
			dialogBoxText=NULL;
		}
		dialogBoxText=(char *)calloc(512,sizeof(char));
		name_len=strlen(name);
		strcpy(dialogBoxText,name);
		strcpy(dialogBoxText+strlen(name),": ");
		vsnprintf(dialogBoxText+name_len+2,512-name_len-2,text,dialogArgs);
		va_end(dialogArgs);
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
		float x,y;
		if(dialogBoxExists){
			glColor3ub(0,0,0);
			x=player->loc.x-SCREEN_WIDTH/2+HLINE*8;
			y=(offset.y+SCREEN_HEIGHT/2)-HLINE*8;
			glRectf(x,y,x+SCREEN_WIDTH-HLINE*16,y-SCREEN_HEIGHT/4);
			char *rtext;
			rtext=typeOut(dialogBoxText);
			setFontSize(16);
			putString(x+HLINE,y-fontSize-HLINE,rtext);
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
	}
	void handleEvents(void){
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
					dialogBoxExists=false;
					dialogBoxText=NULL;
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
				if(SDL_KEY==SDLK_c){
					dialogBox("","You pressed `c`, but nothing happened?");
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
		
		unsigned int i;
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
