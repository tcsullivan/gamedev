#include <UIClass.h>

#include <ft2build.h>
#include FT_FREETYPE_H

extern Player player;
extern World *currentWorld;

static FT_Library ftl;
static FT_Face ftf;
static GLuint ftex;

void UIClass::init(const char *ttf){
	if(FT_Init_FreeType(&ftl)){
		std::cout<<"Error! Couldn't initialize freetype."<<std::endl;
		abort();
	}
	if(FT_New_Face(ftl,ttf,0,&ftf)){
		std::cout<<"Error! Couldn't open "<<ttf<<"."<<std::endl;
		abort();
	}
	
}
void UIClass::setFontSize(unsigned int fs){
	fontSize=fs;
	FT_Set_Pixel_Sizes(ftf,0,fontSize);
}
void UIClass::putString(const float x,const float y,const char *s){
	unsigned int i=0,j;
	float xo=x,yo=y,w,h;
	char *buf;
	do{
		if(s[i]=='\n'){
			xo=x;
			yo-=fontSize*.0022;
		}else{
			FT_Load_Char(ftf,s[i],FT_LOAD_RENDER);
			glGenTextures(1,&ftex);
			glBindTexture(GL_TEXTURE_2D,ftex);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			buf=(char *)malloc(ftf->glyph->bitmap.width*ftf->glyph->bitmap.rows*4);
			for(j=0;j<ftf->glyph->bitmap.width*ftf->glyph->bitmap.rows;j++){
				buf[j*4]=255;
				buf[j*4+1]=255;
				buf[j*4+2]=255;
				buf[j*4+3]=ftf->glyph->bitmap.buffer[j]?255:0;
			}
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,ftf->glyph->bitmap.width,ftf->glyph->bitmap.rows,0,GL_RGBA,GL_UNSIGNED_BYTE,buf);
			w=ftf->glyph->bitmap.width*(2.0/SCREEN_WIDTH);
			h=ftf->glyph->bitmap.rows *(2.0/SCREEN_HEIGHT); 
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,ftex);
			if(s[i]=='\''||
			   s[i]=='\"'||
			   s[i]=='-'||
			   s[i]=='*'){
				yo+=fontSize*.001;
			}
			glBegin(GL_QUADS);
				glColor3ub(255,255,255);
				glTexCoord2f(0,1);glVertex2f(xo,yo);
				glTexCoord2f(1,1);glVertex2f(xo+w,yo);
				glTexCoord2f(1,0);glVertex2f(xo+w,yo+h);
				glTexCoord2f(0,0);glVertex2f(xo,yo+h);
			glEnd();
			if(s[i]=='\''||
			   s[i]=='\"'||
			   s[i]=='-'||
			   s[i]=='*'){
				yo-=fontSize*.001;
			}
			glDisable(GL_TEXTURE_2D);
			xo+=w+(fontSize*.0002);
			free(buf);
		}
	}while(s[i++]);
}
void UIClass::putText(const float x,const float y,const char *str,...){
	va_list args;
	char *buf;
	buf=(char *)calloc(128,sizeof(char));
	va_start(args,str);
	vsnprintf(buf,128,str,args);
	va_end(args);
	putString(x,y,buf);
	free(buf);
}
void UIClass::msgBox(const char *str,...){
	va_list args;
	va_start(args,str);
	glColor3ub(0,0,0);
	glRectf(-1,.6,1,1);
	setFontSize(24);
	putText(-1,1-24*.0022,str,args);
	va_end(args);
}

void UIClass::handleEvents(){
	static bool space=false;
	float thing;
	SDL_Event e;
	while(SDL_PollEvent(&e)){
		switch(e.type){
		case SDL_MOUSEMOTION:
			mousex=e.motion.x;
			mousey=e.motion.y;
			break;
		case SDL_WINDOWEVENT:
			switch(e.window.event){
				case SDL_WINDOWEVENT_CLOSE:
					gameRunning = false;
				break;
			}
		case SDL_KEYDOWN:
			if(e.key.keysym.sym == SDLK_d) player.right = true;
			if(e.key.keysym.sym == SDLK_a) player.left = true;
			if(e.key.keysym.sym == SDLK_LSHIFT) player.speed = 3;
			if(e.key.keysym.sym == SDLK_SPACE){
				if(!space&&player.vel.y<=0){
					space=true;
					player.loc.y += HLINE*1.2;
					player.vel.y += .003;
				}
			}
			if(e.key.keysym.sym == SDLK_i){
				if(currentWorld->behind){
					thing=(currentWorld->getWidth()-currentWorld->behind->getWidth())/2;
					if(player.loc.x>thing-1&&
					   player.loc.x<thing-1+currentWorld->behind->getWidth()){
						player.loc.x-=thing;
						memset(&player.vel,0,sizeof(vec2));
						currentWorld=currentWorld->behind;
					}
				}
			}
			if(e.key.keysym.sym == SDLK_k){
				if(currentWorld->infront){
					player.loc.x+=(currentWorld->infront->getWidth()-currentWorld->getWidth())/2;
					memset(&player.vel,0,sizeof(vec2));
					currentWorld=currentWorld->infront;
				}
			}
			if(e.key.keysym.sym == SDLK_F3){
				debug = !debug;
			}
			break;
		case SDL_KEYUP:
			if(e.key.keysym.sym == SDLK_d) player.right = false;
			if(e.key.keysym.sym == SDLK_a) player.left = false;
			if(e.key.keysym.sym == SDLK_LSHIFT) player.speed = 1.0;
			if(e.key.keysym.sym == SDLK_SPACE)
				if(player.vel.y<=.001)space=false;
		
			if(e.key.keysym.sym == SDLK_ESCAPE) gameRunning = false;
			break;
		}	
	}
}
