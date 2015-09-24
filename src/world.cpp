#include <world.h>

#define getWidth(w) ((w->lineCount-GEN_INC)*HLINE)	// Calculates the width of world 'w'

#define GEN_INC 10		// Defines at what interval y values should be calculated for the array 'line'.
						// As explained in World(), the last few lines in the array 'line' are incorrectly calculated
						// or not calculated at all, so GEN_INC is also used to decrease 'lineCount' in functions like draw()
						// and detect().
#define GRASS_HEIGHT 4	// Defines how long the grass layer of a line should be in multiples of HLINE.


#define DRAW_Y_OFFSET 50	// Defines how many pixels each layer should be offset from each other on the y axis when drawn.
#define DRAW_SHADE	  40	// Defines a shade increment for draw()

extern std::vector<Entity *>entity;

void safeSetColor(int r,int g,int b){	// safeSetColor() is an alternative to directly using glColor3ub() to set
	if(r>255)r=255;						// the color for OpenGL drawing. safeSetColor() checks for values that are
	if(g>255)g=255;						// outside the range of an unsigned character and sets them to a safer value.
	if(b>255)b=255;
	if(r<0)r=0;
	if(g<0)g=0;
	if(b<0)b=0;
	glColor3ub(r,g,b);
}

World::World(unsigned int width){		// Generates the world and sets all variables contained in the World class.
	unsigned int i;						// Used for 'for' loops 
	float inc;							// See line 40
	lineCount=width+GEN_INC;			// Sets line count to the desired width plus GEN_INC to remove incorrect line calculations.
	
	line=(struct line_t *)calloc(lineCount,sizeof(struct line_t));	// Allocate memory for the array 'line'
	
	line[0].y=80;								// Sets a starting value for the world generation to be based off of
	for(i=GEN_INC;i<lineCount;i+=GEN_INC){		// For every GEN_INCth line in the array 'line'
		line[i].y=rand()%8-4+line[i-GEN_INC].y;	// Generate a y value for it, and correct it if it is too high or low.
		if(line[i].y<60)line[i].y=60;	// y value minimum
		if(line[i].y>110)line[i].y=110;	// y value maximum
	}
	for(i=0;i<lineCount-GEN_INC;i++){							// Calculate the rest of the lines as well as set color values for them.
		if(!i||!(i%GEN_INC)){									// If this is one of the GEN_INCth lines that are already calculated
			inc=(line[i+GEN_INC].y-line[i].y)/(float)GEN_INC;	// Calculate the slope between this line and the line ahead of it, then
																// divide it by the number of lines inbetween the two.
		}else{							// If this line's y hasn't been set yet
			line[i].y=line[i-1].y+inc;	// Set it by incrementing the previous line's y by 'inc'.
		}
		line[i].color=rand()%20+130;	// Generate a color for the dirt area of this line. This value will be used
										// in the form (where n represents the color) glColor3ub(n,n-50,n-100)
	}
	x_start=0-getWidth(this)/2+GEN_INC/2*HLINE;	// Calculate x_start (explained in world.h)
	behind=infront=NULL;						// Set pointers to other worlds to NULL
	toLeft=toRight=NULL;						// to avoid accidental calls to goWorld... functions
	//peeps=(Entity **)calloc(WORLD_ENTITY_MAX+1,sizeof(Entity *));	// peeps[0] is reserved for the player when detect() is called
	//peepCount=0;
}

World::~World(void){
	free(line);	// Free (de-allocate) the array 'line'
	//free(peeps); // same for the entity array
}

void World::draw(vec2 *vec){
	static float yoff=DRAW_Y_OFFSET;	// Initialize stuff
	static int shade=0;
	static World *current;
	int i,ie,v_offset,cx_start;
	struct line_t *cline;
	current=this;	// yeah
LOOP1:								// Check for worlds behind the current one and set 'current' to them if they exist
	if(current->behind){			// so that once LOOP1 is exited 'current' contains the furthest back world.
		yoff+=DRAW_Y_OFFSET;
		shade+=DRAW_SHADE;
		current=current->behind;
		goto LOOP1;
	}
LOOP2:													// Draw each world
	v_offset=(vec->x-current->x_start)/HLINE;			// Calculate the player's offset in the array 'line' using the player's location 'vec'
	i=v_offset-SCREEN_WIDTH/(yoff/(DRAW_Y_OFFSET/2));	// Set 'i' to that somehow
	if(i<0)i=0;											// If the player is past the start of that world 'i' should start at the beginning
														// of the world
	ie=v_offset+SCREEN_WIDTH/(yoff/(DRAW_Y_OFFSET/2));	// Set how many lines should be drawn (the drawing for loop loops from 'i' to 'ie')
	if(ie>current->lineCount)ie=current->lineCount;		// If the player is past the end of that world 'ie' should contain the end of that world
	cline=current->line;								// 'cline' and 'cx_start' only exist to make the for loop clear (and maybe make it faster)
	cx_start=current->x_start;
	glBegin(GL_QUADS);
		for(i=i;i<ie-GEN_INC;i++){								// For lines in array 'line' from 'i' to 'ie'
			cline[i].y+=(yoff-DRAW_Y_OFFSET);			// 'yoff' is always one incrementation ahead of what it should be
			safeSetColor(shade,200+shade,shade);		// Safely set the grass color
			glVertex2i(cx_start+i*HLINE      ,cline[i].y);											// Draw the grass area of the line
			glVertex2i(cx_start+i*HLINE+HLINE,cline[i].y);
			glVertex2i(cx_start+i*HLINE+HLINE,cline[i].y-GRASS_HEIGHT);
			glVertex2i(cx_start+i*HLINE		,cline[i].y-GRASS_HEIGHT);
			safeSetColor(cline[i].color+shade,cline[i].color-50+shade,cline[i].color-100+shade);	// Set the shaded dirt color (safely)
			glVertex2i(cx_start+i*HLINE      ,cline[i].y-GRASS_HEIGHT);								// Draw the dirt area of the line
			glVertex2i(cx_start+i*HLINE+HLINE,cline[i].y-GRASS_HEIGHT);
			glVertex2i(cx_start+i*HLINE+HLINE,0);
			glVertex2i(cx_start+i*HLINE		 ,0);
			cline[i].y-=(yoff-DRAW_Y_OFFSET); // Reset 'cline[i]'`s y to what it was
		}
	glEnd();
	if(current->infront){			// If there's a world in front of the one that was just drawn
		yoff-=DRAW_Y_OFFSET;		// draw it as well.
		shade-=DRAW_SHADE;
		current=current->infront;
		goto LOOP2;
	}else{							// Otherwise reset static values and return
		yoff=DRAW_Y_OFFSET;
		shade=0;
		/*if(peepCount){
			for(i=1;i<peepCount;i++){
				peeps[i]->draw();
			}
		}*/
	}
}

void World::singleDetect(Entity *e){
	unsigned int i;
	if(e->alive){
		i=(e->loc.x+e->width/2-x_start)/HLINE;	// Calculate what line the player is currently on
		if(e->loc.y<=line[i].y){			// Snap the player to the top of that line if the player is inside it
			e->vel.y=0;
			e->loc.y=line[i].y+HLINE/2;
		}else{							// If the player is above the ground do some gravity stuff
			e->vel.y-=.01;
		}
		if(e->loc.x<x_start){				// Keep the player inside world bounds (ui.cpp handles world jumping)
			e->vel.x=0;
			e->loc.x=x_start+HLINE/2;
		}else if(e->loc.x+e->width+HLINE>x_start+getWidth(this)){
			e->vel.x=0;
			e->loc.x=x_start+getWidth(this)-e->width-HLINE;
		}
	}
}
void World::detect(Player *p){
	unsigned int i;
	singleDetect(p);
	for(i=0;i<=entity.size();i++){
		singleDetect(entity[i]);
	}
}

/*
 *	The rest of these functions are explained well enough in world.h ;)
*/

void World::addLayer(unsigned int width){
	if(behind){
		behind->addLayer(width);
		return;
	}
	behind=new World(width);
	behind->infront=this;
}

World *World::goWorldLeft(Player *p){
	if(toLeft&&p->loc.x<x_start+HLINE*10){
		p->loc.x=toLeft->x_start+getWidth(toLeft)-HLINE*10;
		p->loc.y=toLeft->line[0].y;
		return toLeft;
	}
	return this;
}

World *World::goWorldRight(Player *p){
	if(toRight&&p->loc.x+p->width>x_start+getWidth(this)-HLINE*10){
		p->loc.x=toRight->x_start+HLINE*10;
		p->loc.y=toRight->line[toRight->lineCount-GEN_INC-1].y;
		return toRight;
	}
	return this;
}

World *World::goWorldBack(Player *p){
	if(behind&&p->loc.x>(int)(0-getWidth(behind)/2)&&p->loc.x<getWidth(behind)/2){
		return behind;
	}
	return this;
}

World *World::goWorldFront(Player *p){
	if(infront&&p->loc.x>(int)(0-getWidth(infront)/2)&&p->loc.x<getWidth(infront)/2){
		return infront;
	}
	return this;
}

/*void World::addEntity(Entity *e){
	if(peepCount!=WORLD_ENTITY_MAX){
		peeps[1+peepCount++]=e; // See peeps's allocation in World() for explanation of the 1+...
	}else{
		std::cout<<"Warning: can't add any more entities"<<std::endl;
	}
}*/
