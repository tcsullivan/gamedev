#include <World.h>

World::World(float width){
	unsigned int i;
	lineCount=width/HLINE+1;
	if((line=(struct line_t *)calloc(lineCount,sizeof(struct line_t)))==NULL){
		std::cout<<"Failed to allocate memory!"<<std::endl;
		abort();
	}
	line[0].start=(rand()%100)/100.0f-1; // lazy
	for(i=1;i<lineCount;i++){
		line[i].start=line[i-1].start+(float)((rand()%20)-10)/1000.0f;
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
