#include <World.h>
#include <cstdio>

World::World(void){
	line=NULL;
	lineCount=0;
	toLeft=toRight=NULL;
}
World::World(const float width,World *l,World *r){
	unsigned int i;
	double f;
	lineCount=width/HLINE+1;
	if((line=(struct line_t *)calloc(lineCount,sizeof(struct line_t)))==NULL){
		std::cout<<"Failed to allocate memory!"<<std::endl;
		abort();
	}
	toLeft=l;
	toRight=r;
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
	line[0].start=(rand()%100)/100.0f-0.8f; // lazy
	if(line[0].start>-0.5f)line[0].start=-0.7f;
	for(i=10;i<lineCount;i+=10){ 
		line[i].start=((double)(rand()%40+200))/1000.0f-1;
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
void World::draw(void){
	unsigned int i;
	glBegin(GL_QUADS);
		for(i=0;i<lineCount;i++){
			glColor3ub(0,255,0);
			glVertex2f((HLINE*i)-1      ,line[i].start);
			glVertex2f((HLINE*i)-1+HLINE,line[i].start);
			glVertex2f((HLINE*i)-1+HLINE,line[i].start-HLINE*2);
			glVertex2f((HLINE*i)-1      ,line[i].start-HLINE*2);
			glColor3ub(150,100,50);
			glVertex2f((HLINE*i)-1      ,line[i].start-HLINE*2);
			glVertex2f((HLINE*i)-1+HLINE,line[i].start-HLINE*2);
			glVertex2f((HLINE*i)-1+HLINE,-1);
			glVertex2f((HLINE*i)-1      ,-1);
		}
	glEnd();
}
void World::detect(vec2 *v,const float width){
	unsigned int i;
	for(i=0;i<lineCount;i++){
		if(v->y<line[i].start){
			if(v->x>(HLINE*i)-1&&v->x<(HLINE*i)-1+HLINE){
				v->x=(HLINE*i)-1+HLINE;
			}else if(v->x+width>(HLINE*i)-1&&v->x+width<(HLINE*i)-1+HLINE){
				v->x=(HLINE*i)-1-width;
			}else{
				v->y=line[i].start+HLINE/4;
			}
		}else if(v->y>line[i].start+HLINE/4){
			//v->y-=HLINE/8;
		}
	}
}
float World::getWidth(void){
	return (lineCount-1)*HLINE;
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

