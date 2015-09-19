#include <ui.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define SDL_KEY e.key.keysym.sym

extern Player *player;

static FT_Library   ftl;
static FT_Face      ftf;
static GLuint       ftex;
static unsigned int fontSize;

namespace ui {
	void initFonts(void){
		if(FT_Init_FreeType(&ftl)){
			std::cout<<"Error! Couldn't initialize freetype."<<std::endl;
			abort();
		}
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
	void putString(const float x,const float y,const char *s){
		unsigned int i=0,j;
		float xo=x,yo=y,w,h;
		char *buf;
		do{
			if(FT_Load_Char(ftf,s[i],FT_LOAD_RENDER)){
				std::cout<<"Error! Unsupported character "<<s[i]<<" ("<<(int)s[i]<<")."<<std::endl;
				return;
			}
			glActiveTexture(GL_TEXTURE0);
			glGenTextures(1,&ftex);
			glBindTexture(GL_TEXTURE_2D,ftex);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glPixelStorei(GL_UNPACK_ALIGNMENT,1);
			buf=(char *)malloc(ftf->glyph->bitmap.width*ftf->glyph->bitmap.rows*4);
			for(j=0;j<ftf->glyph->bitmap.width*ftf->glyph->bitmap.rows;j++){
				buf[j*4]=255;
				buf[j*4+1]=255;
				buf[j*4+2]=255;
				buf[j*4+3]=ftf->glyph->bitmap.buffer[j]?255:0;
			}
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,ftf->glyph->bitmap.width,ftf->glyph->bitmap.rows,0,GL_RGBA,GL_UNSIGNED_BYTE,buf);
			w=ftf->glyph->bitmap.width;
			h=ftf->glyph->bitmap.rows;
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,ftex);
			glBegin(GL_QUADS);
				glColor3ub(255,255,255);
				glTexCoord2f(0,1);glVertex2f(xo,yo);
				glTexCoord2f(1,1);glVertex2f(xo+w,yo);
				glTexCoord2f(1,0);glVertex2f(xo+w,yo+h);
				glTexCoord2f(0,0);glVertex2f(xo,yo+h);
			glEnd();
			glDisable(GL_TEXTURE_2D);
			xo+=w;
			free(buf);
		}while(s[i++]);
	}
	void putText(const float x,const float y,const char *str,...){
		va_list args;
		char *buf;
		buf=(char *)calloc(128,sizeof(char));
		va_start(args,str);
		vsnprintf(buf,128,str,args);
		va_end(args);
		putString(x,y,buf);
		free(buf);
	}
	void handleEvents(void){
		SDL_Event e;
		while(SDL_PollEvent(&e)){
			switch(e.type){
			case SDL_QUIT:
				gameRunning=false;
				break;
			case SDL_KEYDOWN:
				if(SDL_KEY==SDLK_ESCAPE)gameRunning=false;
				if(SDL_KEY==SDLK_a)player->vel.x=-2;
				if(SDL_KEY==SDLK_d)player->vel.x=2;
				if(SDL_KEY==SDLK_SPACE)player->vel.y=2;
				break;
			case SDL_KEYUP:
				if(SDL_KEY==SDLK_a)player->vel.x=0;
				if(SDL_KEY==SDLK_d)player->vel.x=0;
				break;
			default:
				break;
			}
		}
	}
}
