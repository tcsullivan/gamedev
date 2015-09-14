#include <World.h>
#include <cstdio>

// Layer modulation in draw()
static float drawOffsetX=0,
			 drawOffsetY=0;

// Generates a blank world
World::World(void){
	line=NULL;
	lineCount=entCount=0;
	toLeft=toRight=behind=infront=NULL;
}
// Generates a legit world
World::World(const float width,World *l,World *r){
	unsigned int i;
	double f;
	lineCount=width/HLINE+11; 													// Last 10 lines won't be drawn
	if((line=(struct line_t *)calloc(lineCount,sizeof(struct line_t)))==NULL){	// Allocate buffer for lines
		std::cout<<"Failed to allocate memory!"<<std::endl;
		abort();
	}
	toLeft=l;																	// Set other variables
	toRight=r;
	behind=infront=NULL;
	entCount=0;
	if(toLeft){																	// Make sure linked worlds link back
		if(toLeft->toRight){
			std::cout<<"There's already a world to the left!"<<std::endl;
			abort();
		}else toLeft->toRight=this;
	}
	if(toRight){
		if(toRight->toLeft){
			std::cout<<"There's already a world to the right!"<<std::endl;
			abort();
		}else toRight->toLeft=this;
	}
	line[0].start=(grand()%100)/100.0f-0.8f;					// Set .start of first line
	if(line[0].start>-0.5f)line[0].start=-0.7f;					// Don't let the ground take up too much of the window
	for(i=10;i<lineCount;i+=10){ 								// Set heights for every 10 lines
		line[i].start=((double)((grand()%50)+400))/1000.0f-1;
	}
	for(i=0;i<lineCount;i++){									// Set heights for other lines based on those
		if(!(i%10)||!i){
			f=line[i+10].start-line[i].start; 					// 1/10th of difference between the two heights
			f/=10.0f;
		}else{
			line[i].start=line[i-1].start+f;
		}
		line[i].color=(grand()%2)*10+130;
	}
}
// Set RGB color with potentially not 8-bit values
void safeSetColor(int r,int g,int b){
	if(r>255)r=255;else if(r<0)r=0;
	if(g>255)g=255;else if(g<0)g=0;
	if(b>255)b=255;else if(b<0)b=0;
	glColor3ub(r,g,b);
}
void World::draw(void){
	unsigned int i;
	float x,y,hline=HLINE;
	World *root,*cur;
	int shade;
	root=cur=this;
LOOP:
	if(cur->behind){												// If there's a layer behind us,
		drawOffsetX+=(cur->getWidth()-cur->behind->getWidth())/2;	// set drawOffsetX so that it will be centered behind this one
		drawOffsetY+=.3;											// Push it back a bit for depth-feel
		//hline/=2;
		cur=cur->behind;											// Go back one
		goto LOOP;													// loop
	}
LOOP2:																// Should be in furthest back layer once this is first reached
	shade=30*(drawOffsetY/.3);										// Trash shaders
	glBegin(GL_QUADS);
		for(i=0;i<cur->lineCount-10;i++){							// Draw the layer
			x=(hline*i)-1+drawOffsetX;								// Pre-calculate x for 'optimization'
			y=cur->line[i].start+drawOffsetY;						// same, but y
			safeSetColor(0,200+shade,0);							// Set shaded green for grass
			glVertex2f(x      ,y);									// Doodle
			glVertex2f(x+hline,y);
			y-=hline*2;												// 'optimization'
			glVertex2f(x+hline,y);
			glVertex2f(x	  ,y);
			safeSetColor(line[i].color+shade,line[i].color-50+shade,line[i].color-100+shade);				// Set shaded brown for dirt
			glVertex2f(x	  ,y);
			glVertex2f(x+hline,y);
			glVertex2f(x+hline,-1);
			glVertex2f(x	  ,-1);
		}
	glEnd();
	if(root!=cur){													// If we're still in one of the behinds
		cur=cur->infront;											// Move one closer
		drawOffsetX-=(cur->getWidth()-cur->behind->getWidth())/2;	// Take off last layer's centering
		drawOffsetY-=.3;											// And back-pushing
		//hline*=2;
		goto LOOP2;													// Loop the draw
	}else{
		drawOffsetX=drawOffsetY=0;									// Reset for next draw() call
		for(i=0;i<entCount;i++){									// Draw any bound entities
			((Entity **)entity)[i]->draw();
		}
	}
}
extern World *spawn;
void World::detect(vec2 *v,vec2 *vel,const float width){
	unsigned int i;
	for(i=0;i<lineCount-10;i++){											// For every line in world
		if(v->y<line[i].start){												// If we're inside the line
			if(v->x>(HLINE*i)-1&&v->x<(HLINE*i)-1+HLINE){					// And we're inside it ;)
				vel->y=0;v->y=line[i].start+HLINE/8;							// Correct
				return; // :/
			}else if(v->x+width>(HLINE*i)-1&&v->x+width<(HLINE*i)-1+HLINE){ // Same as above, but coming from right side instead of left
				vel->y=0;v->y=line[i].start+HLINE/8;
				return; // ;)
			}
		}
		if(v->y>line[i].start+HLINE){									// Trashy gravity handling
			vel->y-=this==spawn?.0000001:.0000003;
		}
	}
}
// Calculate the world's width in coordinates
float World::getWidth(void){
	return (lineCount-11)*HLINE;
}
void World::addLayer(const float width){
	if(behind){									// If there's already a layer behind us
		behind->addLayer(width);				// Add it back there
	}else{
		behind=new World(width,NULL,NULL);		// Otherwise add it directly behind us
		behind->infront=this;
	}
}
void World::addEntity(void *e){
	entity[entCount++]=e;		// duh
}
