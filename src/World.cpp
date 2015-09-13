#include <World.h>
#include <cstdio>

static float drawOffsetX=0,
			 drawOffsetY=0;

World::World(void){
	line=NULL;
	lineCount=entCount=0;
	toLeft=toRight=behind=infront=NULL;
}
World::World(const float width,World *l,World *r){
	unsigned int i;
	double f;
	lineCount=width/HLINE+11;
	if((line=(struct line_t *)calloc(lineCount,sizeof(struct line_t)))==NULL){
		std::cout<<"Failed to allocate memory!"<<std::endl;
		abort();
	}
	toLeft=l;
	toRight=r;
	behind=infront=NULL;
	entCount=0;
	if(toLeft){
		if(toLeft->toRight){
			std::cout<<"There's already a world to the left!"<<std::endl;
			abort();
		}else{
			toLeft->toRight=this;
		}
	}
	if(toRight){
		if(toRight->toLeft){
			std::cout<<"There's already a world to the right!"<<std::endl;
			abort();
		}else{
			toRight->toLeft=this;
		}
	}
	line[0].start=(grand()%100)/100.0f-0.8f; // lazy
	if(line[0].start>-0.5f)line[0].start=-0.7f;
	for(i=10;i<lineCount;i+=10){ 
		line[i].start=((double)((grand()%50)+400))/1000.0f-1;
	}
	for(i=0;i<lineCount;i++){
		if(!(i%10)||!i){
			f=line[i+10].start-line[i].start;
			f/=10.0f;
		}else{
			line[i].start=line[i-1].start+f;
		}
	}
}
void safeSetColor(int r,int g,int b){
	if(r>255)r=255;else if(r<0)r=0;
	if(g>255)g=255;else if(g<0)g=0;
	if(b>255)b=255;else if(b<0)b=0;
	glColor3ub(r,g,b);
}
void World::draw(void){
	unsigned int i;
	float x,y,hline=HLINE;
	static World *root,*cur;
	int shade;
	root=cur=this;
LOOP:
	if(cur->behind){
		drawOffsetX+=(cur->getWidth()-cur->behind->getWidth())/2;
		drawOffsetY+=.3;
		//hline/=2;
		cur=cur->behind;
		goto LOOP;
		//behind->draw();
	}
LOOP2:
	shade=30*(drawOffsetY/.3);
	glBegin(GL_QUADS);
		for(i=0;i<cur->lineCount-10;i++){
			x=(hline*i)-1+drawOffsetX;
			y=cur->line[i].start+drawOffsetY;
			safeSetColor(0,200+shade,0);
			glVertex2f(x      ,y);
			glVertex2f(x+hline,y);
			y-=hline*2;
			glVertex2f(x+hline,y);
			glVertex2f(x	  ,y);
			safeSetColor(150+shade,100+shade,50+shade);
			glVertex2f(x	  ,y);
			glVertex2f(x+hline,y);
			glVertex2f(x+hline,-1);
			glVertex2f(x	  ,-1);
		}
	glEnd();
	if(root!=cur){
		cur=cur->infront;
		drawOffsetX-=(cur->getWidth()-cur->behind->getWidth())/2;
		drawOffsetY-=.3;
		//hline*=2;
		goto LOOP2;
	}else{
		drawOffsetX=drawOffsetY=0;
		for(i=0;i<entCount;i++){
			((Entity **)entity)[i]->draw();
		}
	}
}
void World::detect(vec2 *v,const float width){
	unsigned int i;
	// hey
	// oh hai
	for(i=0;i<lineCount-10;i++){
		if(v->y<line[i].start){
			if(v->x>(HLINE*i)-1&&v->x<(HLINE*i)-1+HLINE){
				v->y=line[i].start;
				return;
			}else if(v->x+width>(HLINE*i)-1&&v->x+width<(HLINE*i)-1+HLINE){
				v->y=line[i].start;
				return;
			}
		}else if(v->y>line[i].start+HLINE/4){
			v->y-=HLINE/32;
		}
	}
}
float World::getWidth(void){
	return (lineCount-11)*HLINE;
}
void World::saveToFile(FILE *f,World *parent){
	fwrite(&lineCount,sizeof(unsigned int) ,1        ,f);
	fwrite(&line     ,sizeof(struct line_t),lineCount,f);
	if(toLeft!=NULL&&toLeft!=parent->toLeft){
		toLeft->saveToFile(f,toLeft);
	}
	if(toRight!=NULL&&toRight!=parent->toRight){
		toRight->saveToFile(f,toRight);
	}
}
void World::loadFromFile(FILE *f,World *parent){
	fread(&lineCount,sizeof(unsigned int) ,1        ,f);
	line=(struct line_t *)malloc(lineCount*sizeof(struct line_t *));
	fread(&line     ,sizeof(struct line_t),lineCount,f);
	if(toLeft!=NULL&&toLeft!=parent->toLeft){
		toLeft->loadFromFile(f,toLeft);
	}
	std::cout<<toRight<<" "<<parent->toRight<<std::endl;
	if(toRight!=NULL&&toRight!=parent->toRight){
		puts("A");
		toRight->loadFromFile(f,toRight);
	}
}
void World::addLayer(const float width){
	if(behind){
		behind->addLayer(width);
	}else{
		behind=new World(width,NULL,NULL);
		behind->infront=this;
	}
}
void World::addEntity(void *e){
	entity[entCount++]=e;
}
