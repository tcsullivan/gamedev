#include <world.h>

#define getWidth(w) ((w->lineCount-GEN_INC)*HLINE)

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
	x_start=0-getWidth(this)/2+GEN_INC/2*HLINE;
	behind=infront=NULL;
	toLeft=toRight=NULL;
}

World::~World(void){
	free(line);
}

void World::draw(vec2 *vec){
	static float yoff=50;
	static int shade=0;
	static World *root,*current;
	int i,ie,v_offset,cx_start;
	struct line_t *cline;
	root=current=this;
LOOP1:
	if(current->behind){
		yoff+=50;
		shade+=40;
		current=current->behind;
		goto LOOP1;
	}
LOOP2:
	v_offset=(vec->x-current->x_start)/HLINE;
	i=v_offset-SCREEN_WIDTH/(yoff/25);
	if(i<0)i=0;
	ie=v_offset+SCREEN_WIDTH/(yoff/25);
	if(ie>current->lineCount)ie=current->lineCount;
	//std::cout<<v_offset<<" "<<i<<" "<<ie<<yoff<<std::endl;
	cline=current->line;
	cx_start=current->x_start;
	glBegin(GL_QUADS);
		for(i=i;i<ie-GEN_INC;i++){
			cline[i].y+=(yoff-50);
			safeSetColor(shade,200+shade,shade);
			glVertex2i(cx_start+i*HLINE      ,cline[i].y);
			glVertex2i(cx_start+i*HLINE+HLINE,cline[i].y);
			glVertex2i(cx_start+i*HLINE+HLINE,cline[i].y-GRASS_HEIGHT);
			glVertex2i(cx_start+i*HLINE		,cline[i].y-GRASS_HEIGHT);
			safeSetColor(cline[i].color+shade,cline[i].color-50+shade,cline[i].color-100+shade);
			glVertex2i(cx_start+i*HLINE      ,cline[i].y-GRASS_HEIGHT);
			glVertex2i(cx_start+i*HLINE+HLINE,cline[i].y-GRASS_HEIGHT);
			glVertex2i(cx_start+i*HLINE+HLINE,0);
			glVertex2i(cx_start+i*HLINE		 ,0);
			cline[i].y-=(yoff-50);
		}
	glEnd();
	if(current->infront){
		yoff-=50;
		shade-=40;
		current=current->infront;
		goto LOOP2;
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
		vel->y-=.01;
	}
	// Horizontal checks
	if(v->x<x_start){
		vel->x=0;
		v->x=x_start+HLINE/2;
	}else if(v->x+width+HLINE>x_start+getWidth(this)){
		vel->x=0;
		v->x=x_start+getWidth(this)-width-HLINE;
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

World *World::goWorldLeft(vec2 *loc,const float width){
	if(toLeft&&loc->x<x_start+HLINE*10){
		loc->x=toLeft->x_start+getWidth(toLeft)-HLINE*10;
		loc->y=toLeft->line[0].y;
		return toLeft;
	}
	return this;
}

World *World::goWorldRight(vec2 *loc,const float width){
	if(toRight&&loc->x+width>x_start+getWidth(this)-HLINE*10){
		loc->x=toRight->x_start+HLINE*10;
		loc->y=toRight->line[toRight->lineCount-GEN_INC-1].y;
		return toRight;
	}
	return this;
}

World *World::goWorldBack(vec2 *loc,const float width){
	if(behind&&loc->x>(int)(0-getWidth(behind)/2)&&loc->x<getWidth(behind)/2){
		return behind;
	}
	return this;
}

World *World::goWorldFront(vec2 *loc,const float width){
	if(infront&&loc->x>(int)(0-getWidth(infront)/2)&&loc->x<getWidth(infront)/2){
		return infront;
	}
	return this;
}
