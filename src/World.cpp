#include <World.h>
#include <cstdio>

World::World(void){
	line=NULL;
	lineCount=0;
	toLeft=toRight=behind=infront=NULL;
	root=false;
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
	root=false;
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
static float hline=HLINE;
static float back=0;
void World::draw(void){
	unsigned int i;
	if(behind){
		hline*=.5;
		back+=.2;
		behind->draw();
	}else{
		hline*=.5;
		back+=.2;
	}
	if(root){
		hline=HLINE;
		back=0;
	}else{
		hline*=2;
		back-=.2;
	}
	glBegin(GL_QUADS);
		for(i=0;i<lineCount-10;i++){
			glColor3ub(0,255,0);
			glVertex2f((hline*i)-1      ,line[i].start+back);
			glVertex2f((hline*i)-1+hline,line[i].start+back);
			glVertex2f((hline*i)-1+hline,line[i].start-hline*2+back);
			glVertex2f((hline*i)-1      ,line[i].start-hline*2+back);
			glColor3ub(150,100,50);
			glVertex2f((hline*i)-1      ,line[i].start-hline*2+back);
			glVertex2f((hline*i)-1+hline,line[i].start-hline*2+back);
			glVertex2f((hline*i)-1+hline,-1+back);
			glVertex2f((hline*i)-1      ,-1+back);
		}
	glEnd();
}
void World::detect(vec2 *v,const float width){
	unsigned int i;
	// hey
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
			v->y-=HLINE/8;
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
void World::addLayer(void){
	if(behind){
		behind->addLayer();
	}else{
		behind=new World((lineCount-1)*HLINE+LAYER_SCALE,NULL,NULL);
		behind->infront=this;
	}
}
void World::setRoot(void){
	root=true;
}
