#include <world.h>

#define getWidth(w) ((w->lineCount-GEN_INC)*HLINE)	// Calculates the width of world 'w'

#define GEN_INC 10		// Defines at what interval y values should be calculated for the array 'line'.
						// As explained in World(), the last few lines in the array 'line' are incorrectly calculated
						// or not calculated at all, so GEN_INC is also used to decrease 'lineCount' in functions like draw()
						// and detect().

#define GRASS_HEIGHT 4	// Defines how long the grass layer of a line should be in multiples of HLINE.


#define DRAW_Y_OFFSET 50	// Defines how many pixels each layer should be offset from each other on the y axis when drawn.
#define DRAW_SHADE	  30	// Defines a shade increment for draw()

#define INDOOR_FLOOR_HEIGHT 100 // Defines how high the base floor of an IndoorWorld should be

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

float worldGetYBase(World *w){
	float base = 0;
	World *ptr = w;
	while(ptr->infront){
		base+=DRAW_Y_OFFSET;
		ptr=ptr->infront;
	}
	return base;
}

World::World(void){
}

void World::generate(unsigned int width){	// Generates the world and sets all variables contained in the World class.
	unsigned int i;
	float inc;
	
	/*
	 *	Calculate the world's real width. The current form of generation fails to generate
	 *	the last GEN_INC lines, so we offset those making the real real width what was passed
	 *	to this function.
	 * 
	 *	Abort if the width is invalid.
	 * 
	*/
	
	if((lineCount = width + GEN_INC) <= 0)
		abort();
	
	/*
	 *	Allocate enough memory for the world to be stored.
	*/
	
	line=(struct line_t *)calloc(lineCount,sizeof(struct line_t));
	
	/*
	 *	Set an initial y to base generation off of, as generation references previous lines.
	*/
	
	line[0].y=80;
	
	/*
	 *	Populate every GEN_INCth line structure. The remaining lines will be based off of these.
	*/
	
	for(i=GEN_INC;i<lineCount;i+=GEN_INC){
		
		/*
		 *	Generate a y value, ensuring it stays within a reasonable range.
		*/
		
		line[i].y=rand() % 8 - 4 + line[i-GEN_INC].y;	// Add +/- 4 to the previous line
			 if(line[i].y < 60)line[i].y =  60;			// Minimum bound
		else if(line[i].y > 90)line[i].y =  90;			// Maximum bound
		
	}
	
	/*
	 *	Generate values for the remaining lines here.
	*/
	
	for(i=0;i<lineCount-GEN_INC;i++){
		
		/*
		 *	Every GEN_INCth line calculate the slope between the current line and the one
		 *	GEN_INC lines before it. This value is then divided into an increment that is
		 *	added to lines between these two points resulting in a smooth slope.
		 * 
		*/
		
		if(!i||!(i%GEN_INC)){
			
			inc=(line[i + GEN_INC].y - line[i].y) / (float)GEN_INC;
			
		}else{
			
			/*
			 *	Add the increment to create the smooth slope.
			*/
			
			line[i].y=line[i - 1].y + inc;
			
		}
		
		/*
		 *	Generate a color value for the line. This will be referenced in World->draw(),
		 *	by setting an RGB value of color (red), color - 50 (green), color - 100 (blue).
		*/
		
		line[i].color=rand() % 20 + 100; // 100 to 120

		/*
		 *	Each line has two 'blades' of grass, here we generate random heights for them.
		*/
		
		line[i].gh[0]=(getRand() % 16) / 3.5 + 2;	// Not sure what the range resolves to here...
		line[i].gh[1]=(getRand() % 16) / 3.5 + 2;	//
		
		line[i].gs=true;							// Show the blades of grass (modified by the player)
		
	}
	
	/*
	 *	Calculate the x coordinate to start drawing this world from so that it is centered at (0,0).
	*/
	
	x_start=0 - getWidth(this) / 2 + GEN_INC / 2 * HLINE;
	
	/*
	 *	Nullify pointers to other worlds.
	*/
	
	behind	=
	infront	=
	toLeft	=
	toRight	= NULL;
}

World::~World(void){
	free(line);
}

void World::draw(Player *p){
	static float yoff=DRAW_Y_OFFSET;	// Initialize stuff
	static int shade=0;
	static World *current;
	int i,is,ie,v_offset,cx_start;
	struct line_t *cline;
	glClearColor(.1,.3,.6,0);
	
	/*
	 *	World drawing is done recursively, meaning that this function jumps
	 *	back as many 'layers' as it can and then draws, eventually coming
	 *	back to the initial or 'root' layer. LOOP1 does the recursion back
	 *	to the furthest behind layer, modifying shade and y offsets as it
	 *	does.
	 * 
	*/
	
	current=this;
	
LOOP1:

	if(current->behind){
		
		/*
		 *	Add to the y offset and shade values (explained further below)
		 *	and recurse.
		 * 
		*/
		
		yoff+=DRAW_Y_OFFSET;
		shade+=DRAW_SHADE;
		current=current->behind;
		goto LOOP1;
	}
	
	/*
	 *	Here is where the actual world drawing begins. A goto is made to
	 *	LOOP2 once the current layer is drawn and the function shifts to
	 *	draw the next closest layer.
	*/
	
LOOP2:

	/*
	 *	Calculate the offset in the line array that the player is (or would)
	 *	currently be on. This function then calculates reasonable values for
	 *	the 'for' loop below that draws the layer.
	*/

	v_offset=(p->loc.x - current->x_start) / HLINE;
	
	// is -> i start
	
	is=v_offset - SCREEN_WIDTH / (yoff / (DRAW_Y_OFFSET / 2));
	if(is<0)is=0;												// Minimum bound

	ie=v_offset + SCREEN_WIDTH / (yoff / (DRAW_Y_OFFSET / 2));
	if(ie>current->lineCount)ie=current->lineCount;				// Maximum bound
	
	/*
	 *	Make more direct variables for quicker referencing.
	*/
	
	cline	=current->line;
	cx_start=current->x_start;
	
	/*
	 *	Invert shading if desired.
	*/
	
	//shade*=-1;
	
	/*
	 *	Draw entities in the current layer if we're on the one we started at.
	*/
	
	if(current==this){
		for(i=0;i<entity.size();i++){
			if(entity[i]->inWorld==this)
				entity[i]->draw();
		}
	}
	
	/*
	 *	Draw the layer up until the grass portion, which is done later.
	*/
	
	glBegin(GL_QUADS);
		for(i=is;i<ie-GEN_INC;i++){
			cline[i].y+=(yoff-DRAW_Y_OFFSET);														// Add the y offset
			safeSetColor(cline[i].color+shade,cline[i].color-50+shade,cline[i].color-100+shade);	// Set the shaded dirt color
			glVertex2i(cx_start+i*HLINE      ,cline[i].y-GRASS_HEIGHT);
			glVertex2i(cx_start+i*HLINE+HLINE,cline[i].y-GRASS_HEIGHT);
			glVertex2i(cx_start+i*HLINE+HLINE,0);
			glVertex2i(cx_start+i*HLINE		 ,0);
			cline[i].y-=(yoff-DRAW_Y_OFFSET);														// Restore the line's y value
		}
	glEnd();
	
	if(current==this){
		int ph;
		ph=(p->loc.x+p->width/2-x_start)/HLINE;	// Calculate what line the player is currently on
		for(i=0;i<lineCount-GEN_INC;i++){
			if(p->ground==1&&i<ph+6&&i>ph-6)cline[i].gs=false;
			else cline[i].gs=true;
		}
		p->draw();
	}
	float cgh[2];
	glBegin(GL_QUADS);
		for(i=is;i<ie-GEN_INC;i++){
			memcpy(cgh,cline[i].gh,2*sizeof(float));
			if(!cline[i].gs){
				cgh[0]/=4;
				cgh[1]/=4;
			}
			cline[i].y+=(yoff-DRAW_Y_OFFSET);
			safeSetColor(shade,150+shade,shade);
			glVertex2i(cx_start+i*HLINE        ,cline[i].y+cgh[0]);
			glVertex2i(cx_start+i*HLINE+HLINE/2,cline[i].y+cgh[0]);
			glVertex2i(cx_start+i*HLINE+HLINE/2,cline[i].y-GRASS_HEIGHT);
			glVertex2i(cx_start+i*HLINE		   ,cline[i].y-GRASS_HEIGHT);
			glVertex2i(cx_start+i*HLINE+HLINE/2,cline[i].y+cgh[1]);
			glVertex2i(cx_start+i*HLINE+HLINE  ,cline[i].y+cgh[1]);
			glVertex2i(cx_start+i*HLINE+HLINE  ,cline[i].y-GRASS_HEIGHT);
			glVertex2i(cx_start+i*HLINE+HLINE/2,cline[i].y-GRASS_HEIGHT);
			cline[i].y-=(yoff-DRAW_Y_OFFSET);
		}
	glEnd();
	//shade*=-1;
	safeSetColor(255+shade*2,0+shade,0+shade);
	for(i=0;i<current->platform.size();i++){
		glRectf(current->platform[i].p1.x,current->platform[i].p1.y+yoff-DRAW_Y_OFFSET,
				current->platform[i].p2.x,current->platform[i].p2.y+yoff-DRAW_Y_OFFSET);
	}
	if(current->infront){			// If there's a world in front of the one that was just drawn
		yoff-=DRAW_Y_OFFSET;		// draw it as well.
		shade-=DRAW_SHADE;
		current=current->infront;
		goto LOOP2;
	}else{							// Otherwise reset static values and return
		yoff=DRAW_Y_OFFSET;
		shade=0;
	}
}

void World::singleDetect(Entity *e){
	unsigned int i;
	
	/*
	 *	Kill any dead entities.
	*/
	
	if(e->alive&&e->health<=0){
	  
		e->alive=false;
		std::cout<<"Killing entity..."<<std::endl;
		return;
		
	}
	
	/*
	 *	Handle only living entities.
	*/
	
	if(e->alive){
	  
		/*
		 *	Calculate the line that this entity is currently standing on.
		*/
		
		i=(e->loc.x + e->width / 2 - x_start) / HLINE;
		
		/*
		 *	If the entity is under the world/line, pop it back to the surface.
		*/
		
		if(e->loc.y < line[i].y){
		  
			e->ground=true;
			
			e->vel.y=0;
			e->loc.y=line[i].y - .001 * deltaTime;
		
		/*
		 *	Otherwise, if the entity is above the line... 
		*/
		
		}else if(e->loc.y > line[i].y - .002 * deltaTime){
		  
			/*
			 *	Check for any potential platform collision (i.e. landing on a platform) 
			*/
		  
			for(i=0;i<platform.size();i++){
			  
				if(((e->loc.x + e->width > platform[i].p1.x) & (e->loc.x + e->width < platform[i].p2.x)) ||	// Check X left bounds
				   ((e->loc.x < platform[i].p2.x) & (e->loc.x>platform[i].p1.x))){							// Check X right bounds
					if(e->loc.y > platform[i].p1.y && e->loc.y < platform[i].p2.y){							// Check Y bounds
					  
						/*
						 *	Check if the entity is falling onto the platform so
						 *	that it doesn't snap to it when attempting to jump
						 *	through it.
						 * 
						*/
					  
						if(e->vel.y<=0){
						  
							e->ground=2;
						  
							e->vel.y=0;
							e->loc.y=platform[i].p2.y;
							
							//return;	// May not be necessary
						}
					}
				}
			}
			
			/*
			 *	Handle gravity.
			*/
			
			e->vel.y-=.001 * deltaTime;
			
		}
		
		/*
		 *	Insure that the entity doesn't fall off either edge of the world.
		*/
		
		if(e->loc.x<x_start){												// Left bound
			
			e->vel.x=0;
			e->loc.x=x_start + HLINE / 2;
			
		}else if(e->loc.x + e->width + HLINE > x_start + getWidth(this)){	// Right bound
			
			e->vel.x=0;
			e->loc.x=x_start + getWidth(this) - e->width - HLINE;
			
		}
	}
}

void World::detect(Player *p){
	unsigned int i;
	
	/*
	 *	Handle the player. 
	*/
	
	singleDetect(p);
	
	/*
	 *	Handle all remaining entities in this world. 
	*/
	
	for(i=0;i<entity.size();i++){
		
		if(entity[i]->inWorld==this){
			
			singleDetect(entity[i]);
			
		}
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
	behind=new World();
	behind->generate(width);
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

void World::addPlatform(float x,float y,float w,float h){
	platform.push_back((Platform){{x,y},{x+w,y+h}});
}

World *World::goInsideStructure(Player *p){
	unsigned int i;
	for(i=0;i<entity.size();i++){
		if(entity[i]->type==-1){
			if(entity[i]->inWorld==this){
				if(p->loc.x>entity[i]->loc.x&&p->loc.x+p->width<entity[i]->loc.x+entity[i]->width){
					return (World *)((Structures *)entity[i])->inside;
				}
			}else if(((Structures *)entity[i])->inside==this){
				p->loc.x=entity[i]->loc.x+entity[i]->width/2-p->width/2;
				p->loc.y=entity[i]->loc.y+HLINE;
				return (World *)entity[i]->inWorld;
			}
		}
	}
	return this;
}

void World::addHole(unsigned int start,unsigned int end){
	unsigned int i;
	for(i=start;i<end;i++){
		line[i].y=0;
	}
}



IndoorWorld::IndoorWorld(void){
}

IndoorWorld::~IndoorWorld(void){
	free(line);
}

void IndoorWorld::generate(unsigned int width){		// Generates a flat area of width 'width'
	unsigned int i;						// Used for 'for' loops 
	lineCount=width+GEN_INC;			// Sets line count to the desired width plus GEN_INC to remove incorrect line calculations.
	if(lineCount<=0)abort();
	
	line=(struct line_t *)calloc(lineCount,sizeof(struct line_t));	// Allocate memory for the array 'line'
	
	for(i=0;i<lineCount;i++){			// Indoor areas don't have to be directly on the ground (i.e. 0)...
		line[i].y=INDOOR_FLOOR_HEIGHT;
	}
	behind=infront=NULL;						// Set pointers to other worlds to NULL
	toLeft=toRight=NULL;						// to avoid accidental calls to goWorld... functions
	x_start=0-getWidth(this)/2+GEN_INC/2*HLINE;	// Calculate x_start (explained in world.h)
}

void IndoorWorld::draw(Player *p){
	int i,ie,v_offset;
	v_offset=(p->loc.x-x_start)/HLINE;					// Calculate the player's offset in the array 'line' using the player's location 'vec'
	i=v_offset-SCREEN_WIDTH/2;							// um
	if(i<0)i=0;											// If the player is past the start of that world 'i' should start at the beginning
														// of the world
	ie=v_offset+SCREEN_WIDTH/2;							// Set how many lines should be drawn (the drawing for loop loops from 'i' to 'ie')
	if(ie>lineCount)ie=lineCount;						// If the player is past the end of that world 'ie' should contain the end of that world
	glClearColor(.3,.1,0,0);
	glBegin(GL_QUADS);
		for(i=i;i<ie-GEN_INC;i++){						// For lines in array 'line' from 'i' to 'ie'
			safeSetColor(150,100,50);
			glVertex2i(x_start+i*HLINE      ,line[i].y);	// Draw the base floor
			glVertex2i(x_start+i*HLINE+HLINE,line[i].y);
			glVertex2i(x_start+i*HLINE+HLINE,line[i].y-50);
			glVertex2i(x_start+i*HLINE		,line[i].y-50);
		}
	glEnd();
	for(i=0;i<entity.size()+1;i++){
		if(entity[i]->inWorld==this)
			entity[i]->draw();
	}
	p->draw();
}
