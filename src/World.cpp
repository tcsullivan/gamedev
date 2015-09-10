#include <World.h>

World::World(float width){
	unsigned int i;
	double f;
	lineCount=width/HLINE+1;
	if((line=(struct line_t *)calloc(lineCount,sizeof(struct line_t)))==NULL){
		std::cout<<"Failed to allocate memory!"<<std::endl;
		abort();
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
				v->y=line[i].start;
			}
		}else if(v->y>line[i].start+HLINE/2){
			v->y-=HLINE/4;
		}
	}
}
