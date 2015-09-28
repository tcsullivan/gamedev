#include <ui.h>
#include <world.h> // World-switching stuff
#include <ft2build.h> // FreeType stuff
#include FT_FREETYPE_H

#define SDL_KEY e.key.keysym.sym	// Keeps the code neater :)

extern Player *player;			// 'player' should be (must be) defined in main.cpp
extern World  *currentWorld;	// should/must also be defined in main.cpp

static FT_Library   ftl;		// Variables for the FreeType library and stuff
static FT_Face      ftf;
static GLuint       ftex;

static bool dialogBoxExists=false;
static const char *dialogBoxText=NULL;

namespace ui {
	bool debug=false;
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
	}
	void setFontFace(const char *ttf){
		if(FT_New_Face(ftl,ttf,0,&ftf)){
			std::cout<<"Error! Couldn't open "<<ttf<<"."<<std::endl;
			abort();
		}	
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
		glActiveTexture(GL_TEXTURE0);
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
	void putString(const float x,const float y,const char *s){
		unsigned int i=0,j;
		float xo=x,yo=y;
		do{
			if(s[i]=='\n'){
				yo-=fontSize*1.15;
				xo=x;
			}else if(s[i]==' '){
				xo+=fontSize/2;
			}else{
				xo+=putChar(xo,yo,s[i])+fontSize*.1;
			}
		}while(s[i++]);
	}
	void putText(const float x,const float y,const char *str,...){	// putText() simply runs 'str' and the extra arguments though
		va_list args;												// vsnprintf(), which'll store the complete string to a buffer
		char *buf;													// that's then passed to putString()
		buf=(char *)calloc(128,sizeof(char));
		va_start(args,str);
		vsnprintf(buf,128,str,args);
		va_end(args);
		putString(x,y,buf);
		free(buf);
	}
	void dialogBox(const char *text){
		dialogBoxExists=true;
		dialogBoxText=text;
	}
	void draw(void){
		if(dialogBoxExists){
			glColor3ub(0,0,0);
			glRectf(player->loc.x-SCREEN_WIDTH/2,SCREEN_HEIGHT,player->loc.x+SCREEN_WIDTH/2,SCREEN_HEIGHT-SCREEN_HEIGHT/4);
			putString(player->loc.x-SCREEN_WIDTH/2,SCREEN_HEIGHT-fontSize,dialogBoxText);
		}
	}
	void handleEvents(void){
		SDL_Event e;
		while(SDL_PollEvent(&e)){
			switch(e.type){
			case SDL_QUIT:
				gameRunning=false;
				break;
			case SDL_KEYDOWN:
				if(SDL_KEY==SDLK_ESCAPE)gameRunning=false;							// Exit the game with ESC
				if(SDL_KEY==SDLK_a){												// Move left
					player->vel.x=-.15;
					currentWorld=currentWorld->goWorldLeft(player);
				}
				if(SDL_KEY==SDLK_d){												// Move right
					player->vel.x=.15;
					currentWorld=currentWorld->goWorldRight(player);
				}
				if(SDL_KEY==SDLK_s && player->ground==2){
					player->ground=false;
					player->loc.y-=HLINE*1.5;
				}
				if(SDL_KEY==SDLK_SPACE){											// Jump
					if(player->ground){
						player->vel.y=.25;
						//player->loc.y+=HL
						player->ground=false;
					}
				}
				if(SDL_KEY==SDLK_i)currentWorld=currentWorld->goWorldBack(player);	// Go back a layer if possible
				if(SDL_KEY==SDLK_k)currentWorld=currentWorld->goWorldFront(player);	// Go forward a layer if possible
				if(SDL_KEY==SDLK_F3)debug^=true;
				
				// TEMPORARY UNTIL MOUSE
				if(SDL_KEY==SDLK_t){
					if(dialogBoxExists){
						dialogBoxExists=false;
						dialogBoxText=NULL;
					}else dialogBox("Hello");
				}
				
				break;
			case SDL_KEYUP:
				if(SDL_KEY==SDLK_a)player->vel.x=0;	// Stop the player if movement keys are released
				if(SDL_KEY==SDLK_d)player->vel.x=0;
				break;
			default:
				break;
			}
		}
	}
}
