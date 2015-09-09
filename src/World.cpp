#include <World.h>

World::World(const char *l1,const char *l2,const char *l3,const char *bg){
	unsigned char i=0;
	SDL_Surface *l;
	const char *f[4]={l1,l2,l3,bg};
	memset(layer,0,sizeof(struct layer_t)*4);
	for(;i<4;i++){
		l=IMG_Load(f[i]);
		if(l!=NULL){
			glGenTextures(1,&layer[i].tex);
			glBindTexture(GL_TEXTURE_2D,layer[i].tex);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,l->w,l->h,0,GL_RGB,GL_UNSIGNED_BYTE,l->pixels);
			SDL_FreeSurface(l);
		}
	}
}
void World::draw(void){
	int i;
	float x;
	glEnable(GL_TEXTURE_2D);
	for(i=2;i>=0;i--){
		glBindTexture(GL_TEXTURE_2D,layer[i].tex);
		glBegin(GL_QUADS);
			for(x=-1;x<=1;x+=(TEX_SIZE/(float)(i+1))){
				glTexCoord2d(1,1);glVertex2f(x   	   ,LAYER0_Y-TEX_SIZE+(i*.2));
				glTexCoord2d(0,1);glVertex2f(x+TEX_SIZE,LAYER0_Y-TEX_SIZE+(i*.2));
				glTexCoord2d(0,0);glVertex2f(x+TEX_SIZE,LAYER0_Y			+(i*.2));
				glTexCoord2d(1,0);glVertex2f(x		   ,LAYER0_Y			+(i*.2));
			}
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
}
