#include <world.h>

#define getWidth() ((lineCount-GEN_INC)*HLINE)

#define GEN_INC 10
#define GRASS_HEIGHT 4

void safeSetColor(int r,int g,int b){
	if(r>255)r=255;
	if(g>255)g=255;
	if(b>255)b=255;
	if(r<0)r=0;
	if(g<0)g=0;
	if(b<0)b=0;
	glColor3ub(r,g,b);
}

World::World(unsigned int width){
	unsigned int i;
	float inc;
	lineCount=width+GEN_INC;
	line=(struct line_t *)calloc(lineCount,sizeof(struct line_t));	// allocate space for the array of lines
	line[0].y=80;
	for(i=GEN_INC;i<lineCount;i+=GEN_INC){
		line[i].y=rand()%8-4+line[i-GEN_INC].y;
		if(line[i].y<60)line[i].y=60;
		if(line[i].y>110)line[i].y=110;
	}
	for(i=0;i<lineCount-GEN_INC;i++){
		if(!i||!(i%GEN_INC)){
			inc=(line[i+GEN_INC].y-line[i].y)/(float)GEN_INC;
		}else{
			line[i].y=line[i-1].y+inc;
		}
		line[i].color=rand()%20+130;
	}
	x_start=0-getWidth()/2+GEN_INC/2*HLINE;
	behind=infront=NULL;
}

World::~World(void){
	free(line);
}

void World::draw(vec2 *vec){
	static float yoff=50;
	static int shade=0;
	int i,ie,v_offset;
	if(behind){
		yoff+=50;
		shade+=40;
		behind->draw(vec);
	}
	v_offset=(vec->x-x_start)/HLINE;
	i=v_offset-SCREEN_WIDTH/(yoff/25);
	if(i<0)i=0;
	ie=v_offset+SCREEN_WIDTH/(yoff/25);
	if(ie>lineCount)ie=lineCount;
	glBegin(GL_QUADS);
		for(i=i;i<ie;i++){
			line[i].y+=(yoff-50);
			safeSetColor(shade,200+shade,shade);
			glVertex2i(x_start+i*HLINE      ,line[i].y);
			glVertex2i(x_start+i*HLINE+HLINE,line[i].y);
			glVertex2i(x_start+i*HLINE+HLINE,line[i].y-GRASS_HEIGHT);
			glVertex2i(x_start+i*HLINE		,line[i].y-GRASS_HEIGHT);
			safeSetColor(line[i].color+shade,line[i].color-50+shade,line[i].color-100+shade);
			glVertex2i(x_start+i*HLINE      ,line[i].y-GRASS_HEIGHT);
			glVertex2i(x_start+i*HLINE+HLINE,line[i].y-GRASS_HEIGHT);
			glVertex2i(x_start+i*HLINE+HLINE,0);
			glVertex2i(x_start+i*HLINE		,0);
			line[i].y-=(yoff-50);
		}
	glEnd();
	if(infront){
		yoff-=50;
		shade-=40;
	}else{
		yoff=50;
		shade=40;
	}
}

void World::detect(vec2 *v,vec2 *vel,const float width){
	unsigned int i;
	// Vertical checks
	i=(v->x+width/2-x_start)/HLINE;
	if(v->y<=line[i].y){
		vel->y=0;
		v->y=line[i].y+HLINE/2;
	}else{
		vel->y-=.05;
	}
	// Horizontal checks
	if(v->x<x_start){
		vel->x=0;
		v->x=x_start+HLINE/2;
	}else if(v->x+width+HLINE>x_start+getWidth()){
		vel->x=0;
		v->x=x_start+getWidth()-width-HLINE;
	}
}

void World::addLayer(unsigned int width){
	if(behind){
		behind->addLayer(width);
		return;
	}
	behind=new World(width);
	behind->infront=this;
}
