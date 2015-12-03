#include <world.h>

#define getWidth(w) ((w->lineCount-GEN_INC)*HLINE)	// Calculates the width of world 'w'

#define GEN_MIN  80
#define GEN_MAX  110
#define GEN_INIT 60

#define GRASS_HEIGHT 4 			// Defines how long the grass layer of a line should be in multiples of HLINE.


#define DRAW_Y_OFFSET 50		// Defines how many pixels each layer should be offset from each other on the y axis when drawn.
#define DRAW_SHADE	  30		// Defines a shade increment for draw()

#define INDOOR_FLOOR_HEIGHT 100 // Defines how high the base floor of an IndoorWorld should be

bool worldInside = false;		// True if player is inside a structure

WEATHER weather = SUNNY;

const char *bgPaths[2][6]={
	{"assets/bg.png",				// Daytime background
	"assets/bgn.png",				// Nighttime background
	"assets/bgFarMountain.png",		// Furthest layer
	"assets/forestTileBack.png",	// Closer layer
	"assets/forestTileMid.png",		// Near layer
	"assets/forestTileFront.png"},	// Closest layer
	{"assets/bgWoodTile.png",
	 NULL,
	 NULL,
	 NULL,
	 NULL,
	 NULL}
};

const float bgDraw[3][3]={
	{100,240,.6 },
	{150,250,.4 },
	{255,255,.25}
};

float worldGetYBase(World *w){
	/*float base = 0;
	World *ptr = w;
	while(ptr->infront){
		base+=DRAW_Y_OFFSET;
		ptr=ptr->infront;
	}*/
	return /*base*/ GEN_MIN;
}

void World::setBackground(WORLD_BG_TYPE bgt){
	switch(bgt){
	case BG_FOREST:
		bgTex = new Texturec(6,bgPaths[0]);
		break;
	case BG_WOODHOUSE:
		bgTex = new Texturec(1,bgPaths[1]);
		break;
	}
}

World::World(void){
	/*
	 *	Nullify pointers to other worlds.
	*/
	
	behind	=
	infront	=
	toLeft	=
	toRight	= NULL;
	
	star = new vec2[100];				//(vec2 *)calloc(100,sizeof(vec2));
	memset(star,0,100*sizeof(vec2));
}

void World::deleteEntities(void){
	while(!mob.empty()){
		delete mob.back();
		mob.pop_back();
	}
	while(!npc.empty()){
		delete npc.back();
		npc.pop_back();
	}
	while(!build.empty()){
		delete build.back();
		build.pop_back();
	}
	while(!object.empty()){
		delete object.back();
		object.pop_back();
	}
	while(!entity.empty()) entity.pop_back();
}

World::~World(void){
	if(behind)
		delete behind;
	
	delete bgTex;
	delete[] star;
	delete[] line;

	deleteEntities();
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
	
	line = new struct line_t[lineCount]; //(struct line_t *)calloc(lineCount,sizeof(struct line_t));
	memset(line,0,lineCount*sizeof(struct line_t));
	
	/*
	 *	Set an initial y to base generation off of, as generation references previous lines.
	*/
	
	line[0].y=GEN_INIT;
	
	/*
	 *	Populate every GEN_INCth line structure. The remaining lines will be based off of these.
	*/
	
	for(i=GEN_INC;i<lineCount;i+=GEN_INC){
		
		/*
		 *	Generate a y value, ensuring it stays within a reasonable range.
		*/
		
		line[i].y=rand() % 8 - 4 + line[i-GEN_INC].y;		// Add +/- 4 to the previous line
			 if(line[i].y < GEN_MIN)line[i].y =  GEN_MIN;	// Minimum bound
		else if(line[i].y > GEN_MAX)line[i].y =  GEN_MAX;	// Maximum bound
		
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
		
		if(!(i%GEN_INC)){
			
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
	
	x_start=0 - getWidth(this) / 2;
	
	for(int i=0;i<100;i++){
		star[i].x=getRand()%getTheWidth()-getTheWidth()/2;
		star[i].y=getRand()%SCREEN_HEIGHT+100;
	}
}

void World::generateFunc(unsigned int width,float(*func)(float)){
	unsigned int i;
	if((lineCount = width) <= 0)
		abort();
	line = new struct line_t[lineCount];	//(struct line_t *)calloc(lineCount,sizeof(struct line_t));
	memset(line,0,lineCount*sizeof(struct line_t));
	for(i=0;i<lineCount;i++){
		line[i].y=func(i);
		if(line[i].y<0)line[i].y=0;
		if(line[i].y>2000)line[i].y=2000;
		line[i].color=rand() % 20 + 100;
		line[i].gh[0]=(getRand() % 16) / 3.5 + 2;
		line[i].gh[1]=(getRand() % 16) / 3.5 + 2;
		line[i].gs=true;
	}
	x_start=0 - getWidth(this) / 2;
	
	for(int i=0;i<100;i++){
		star[i].x=getRand()%getTheWidth()-getTheWidth()/2;
		star[i].y=getRand()%SCREEN_HEIGHT+100;
	}
}

void World::update(Player *p,unsigned int delta){
	p->loc.y+= p->vel.y			 *delta;
	p->loc.x+=(p->vel.x*p->speed)*delta;

	for(auto &e : entity){
		if(e->type != STRUCTURET)
			e->loc.x += e->vel.x * delta;
			e->loc.y += e->vel.y * delta;
			if(e->vel.x < 0)e->left = true;
		else if(e->vel.x > 0)e->left = false;
	}
}

int worldShade = 0;

extern vec2 offset;
extern unsigned int tickCount;

void World::draw(Player *p){
	static float yoff=DRAW_Y_OFFSET;	// Initialize stuff
	static int shade,bgshade;
	static World *current;
	int i,is,ie,v_offset,cx_start,width;
	struct line_t *cline;
	
	bgshade = worldShade << 1; // *2
	width = (-x_start) << 1;
	
	/*
	 *	Draw the background images in the appropriate order.
	*/
	
	glEnable(GL_TEXTURE_2D);
	
	bgTex->bind(0);
	safeSetColorA(255,255,255,255 - worldShade * 4);
	
	glBegin(GL_QUADS);
		glTexCoord2i(0,0);glVertex2i( x_start,SCREEN_HEIGHT);
		glTexCoord2i(1,0);glVertex2i(-x_start,SCREEN_HEIGHT);
		glTexCoord2i(1,1);glVertex2i(-x_start,0);
		glTexCoord2i(0,1);glVertex2i( x_start,0);
	glEnd();
	
	bgTex->bindNext();
	safeSetColorA(255,255,255,worldShade * 4);
	
	glBegin(GL_QUADS);
		glTexCoord2i(0,0);glVertex2i( x_start,SCREEN_HEIGHT);
		glTexCoord2i(1,0);glVertex2i(-x_start,SCREEN_HEIGHT);
		glTexCoord2i(1,1);glVertex2i(-x_start,0);
		glTexCoord2i(0,1);glVertex2i( x_start,0);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	/*
	 *	Draws stars if it is an appropriate time of day for them.
	*/

	if(((weather==DARK )&(tickCount%DAY_CYCLE)<DAY_CYCLE/2)   ||
	   ((weather==SUNNY)&(tickCount%DAY_CYCLE)>DAY_CYCLE*.75) ){
		   
		if(tickCount % DAY_CYCLE){	// The above if statement doesn't check for exact midnight.
				
			safeSetColorA(255,255,255,bgshade + getRand() % 30 - 15);
			for(i = 0; i < 100; i++){
				glRectf(star[i].x+offset.x*.9,
						star[i].y,
						star[i].x+offset.x*.9+HLINE,
						star[i].y+HLINE
						);
			}
			
		}
	}
	
	glEnable(GL_TEXTURE_2D);

	/*
	 *	Draw the mountains.
	*/

	bgTex->bindNext();
	safeSetColorA(150-bgshade,150-bgshade,150-bgshade,220);
	
	glBegin(GL_QUADS);
		for(int i = 0; i <= width/1920; i++){
			glTexCoord2i(0,1);glVertex2i(width/-2+(1920*i    )+offset.x*.85,GEN_MIN);
			glTexCoord2i(1,1);glVertex2i(width/-2+(1920*(i+1))+offset.x*.85,GEN_MIN);
			glTexCoord2i(1,0);glVertex2i(width/-2+(1920*(i+1))+offset.x*.85,GEN_MIN+1080);
			glTexCoord2i(0,0);glVertex2i(width/-2+(1920*i    )+offset.x*.85,GEN_MIN+1080);
		}
	glEnd();
	
	/*
	 *	Draw three layers of trees.
	*/

	for(i = 0; i < 3; i++){
		bgTex->bindNext();
		safeSetColorA(bgDraw[i][0]-bgshade,bgDraw[i][0]-bgshade,bgDraw[i][0]-bgshade,bgDraw[i][1]);
	
		glBegin(GL_QUADS);
			for(int j = x_start; j <= -x_start; j += 600){
				glTexCoord2i(0,1);glVertex2i( j     +offset.x*bgDraw[i][2],GEN_MIN);
				glTexCoord2i(1,1);glVertex2i((j+600)+offset.x*bgDraw[i][2],GEN_MIN);
				glTexCoord2i(1,0);glVertex2i((j+600)+offset.x*bgDraw[i][2],GEN_MIN+400);
				glTexCoord2i(0,0);glVertex2i( j     +offset.x*bgDraw[i][2],GEN_MIN+400);
			}
		glEnd();
	}
	
	glDisable(GL_TEXTURE_2D);
	
	/*
	 *	World drawing is done recursively, meaning that this function jumps
	 *	back as many 'layers' as it can and then draws, eventually coming
	 *	back to the initial or 'root' layer. LOOP1 does the recursion back
	 *	to the furthest behind layer, modifying shade and y offsets as it
	 *	does.
	 * 
	*/
	
	current=this;
	shade=worldShade;
	
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

	v_offset=(offset.x + p->width / 2 - current->x_start) / HLINE;
	
	// is -> i start
	
	is=v_offset - (SCREEN_WIDTH / 2 / HLINE) - GEN_INC;
	if(is<0)is=0;												// Minimum bound
	
	// ie -> i end
	
	ie=v_offset + (SCREEN_WIDTH / 2 / HLINE) + GEN_INC + HLINE; 
	if(ie>current->lineCount)ie=current->lineCount;				// Maximum bound
	
	/*
	 *	Make more direct variables for quicker referencing.
	*/
	
	cline	=current->line;
	cx_start=current->x_start;
	
	/*
	 *	Invert shading if desired.
	*/
	
	shade*=-1;
	
	/*
	 *	Draw structures. We draw structures behind the dirt/grass so that the building's
	 *	corners don't stick out.
	*/
	
	for(auto &b : current->build){
		b->loc.y+=(yoff-DRAW_Y_OFFSET);
		b->draw();
		b->loc.y-=(yoff-DRAW_Y_OFFSET);
	}
	
	/*
	 *	Draw the layer up until the grass portion, which is done later.
	*/

	bool hey=false;
	glBegin(GL_QUADS);
		for(i=is;i<ie-GEN_INC;i++){
			cline[i].y+=(yoff-DRAW_Y_OFFSET);															// Add the y offset
			if(!cline[i].y){
				cline[i].y+=50;
				hey=true;
				safeSetColor(cline[i].color-100+shade,cline[i].color-150+shade,cline[i].color-200+shade);
			}else{
				safeSetColor(cline[i].color+shade,cline[i].color-50+shade,cline[i].color-100+shade);	// Set the shaded dirt color
			}
			glVertex2i(cx_start+i*HLINE      ,cline[i].y-GRASS_HEIGHT);
			glVertex2i(cx_start+i*HLINE+HLINE,cline[i].y-GRASS_HEIGHT);
			glVertex2i(cx_start+i*HLINE+HLINE,0);
			glVertex2i(cx_start+i*HLINE		 ,0);
			cline[i].y-=(yoff-DRAW_Y_OFFSET);															// Restore the line's y value
			if(hey){
				hey=false;
				cline[i].y=0;
			}
		}
	glEnd();
	
	/*
	 *	Draw grass on every line.
	*/
	
	float cgh[2];
	glBegin(GL_QUADS);
		for(i=is;i<ie-GEN_INC;i++){
			
			/*
			 *	Load the current line's grass values
			*/
			
			if(cline[i].y)memcpy(cgh,cline[i].gh,2*sizeof(float));
			else 		  memset(cgh,0			,2*sizeof(float));
			
			
			
			/*
			 *	Flatten the grass if the player is standing on it.
			*/
			
			if(!cline[i].gs){
				cgh[0]/=4;
				cgh[1]/=4;
			}
			
			/*
			 *	Actually draw the grass.
			*/
			
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
	
	/*
	 *	Draw non-structure entities.
	*/
		
	for(auto &n : current->npc){
		n->loc.y+=(yoff-DRAW_Y_OFFSET);
		n->draw();
		n->loc.y-=(yoff-DRAW_Y_OFFSET);
	}
	for(auto &m : current->mob){
		m->loc.y+=(yoff-DRAW_Y_OFFSET);
		m->draw();
		m->loc.y-=(yoff-DRAW_Y_OFFSET);
	}
	for(auto &o : current->object){
		if(o->alive){
			o->loc.y+=(yoff-DRAW_Y_OFFSET);
			o->draw();
			o->loc.y-=(yoff-DRAW_Y_OFFSET);
		}
	}
	
	/*
	 *	If we're drawing the closest/last world, handle and draw the player.
	*/
	
	if(current==this){
		
		/*
		 *	Calculate the line that the player is on
		*/
		
		int ph = (p->loc.x + p->width / 2 - x_start) / HLINE;
		
		/*
		 *	If the player is on the ground, flatten the grass where the player is standing
		 *	by setting line.gs to false.
		*/
		
		if(p->ground==1){
			for(i=0;i<lineCount-GEN_INC;i++){
				if(i < ph + 6 && 
				   i > ph - 6 )
					cline[i].gs=false;
				else cline[i].gs=true;
			}
		}else{
			for(i=0;i<lineCount-GEN_INC;i++){
				cline[i].gs=true;
			}
		}
		
		/*
		 *	Draw the player.
		*/
		
		p->draw();
		
	}
	
	/*
	 *	Restore the inverted shading if it was inverted above.
	*/
	
	shade*=-1;
	
	/*
	 *	Draw the next closest world if it exists.
	*/
	
	if(current->infront){
		yoff  -= DRAW_Y_OFFSET;
		shade -= DRAW_SHADE;
		
		current=current->infront;
		goto LOOP2;
		
	}else{
		
		/*
		 *	If finished, reset the yoff and shade variables for the next call.
		*/
		
		yoff=DRAW_Y_OFFSET;
		shade=0;
	}
}

void World::singleDetect(Entity *e){
	int i;
	unsigned int j;
	
	/*
	 *	Kill any dead entities.
	*/
	
	if(!e->alive||e->health<=0){
		
		for(i=0;i<entity.size();i++){
			if(entity[i]==e){
				switch(e->type){
				case STRUCTURET:
					for(j=0;j<build.size();j++){
						if(build[j]==e){
							delete build[j];
							build.erase(build.begin()+j);
							break;
						}
					}
					break;
				case NPCT:
					for(j=0;j<npc.size();j++){
						if(npc[j]==e){
							delete npc[j];
							npc.erase(npc.begin()+j);
							break;
						}
					}
					break;
				case MOBT:
					for(j=0;j<mob.size();j++){
						if(mob[j]==e){
							delete mob[j];
							mob.erase(mob.begin()+j);
							break;
						}
					}
					break;
				case OBJECTT:
					for(j=0;j<object.size();j++){
						if(object[j]==e){
							delete object[j];
							object.erase(object.begin()+j);
							break;
						}
					}
					break;
				}
				std::cout<<"Killed an entity..."<<std::endl;
				entity.erase(entity.begin()+i);
				return;
			}
		}

		std::cout<<"RIP "<<e->name<<"."<<std::endl;
		exit(0);

		return;
	}
	
	/*
	 *	Handle only living entities.
	*/
	
	if(e->alive){
	  
		if(e->type == MOBT && Mobp(e)->subtype == MS_TRIGGER)return;
	  
		/*
		 *	Calculate the line that this entity is currently standing on.
		*/
		
		i=(e->loc.x + e->width / 2 - x_start) / HLINE;
		if(i < 0) i=0;
		if(i > lineCount-1) i=lineCount-1;
		
		/*
		 *	If the entity is under the world/line, pop it back to the surface.
		*/
		
		if(e->loc.y < line[i].y){
			
			/*
			 *	Check that the entity isn't trying to run through a wall.
			*/
			
			if(e->loc.y + e->height > line[i-(int)e->width/2/HLINE].y &&
			   e->loc.y + e->height > line[i+(int)e->width/2/HLINE].y ){
			
				e->loc.y=line[i].y - .001 * deltaTime;
				e->ground=true;
				e->vel.y=0;
				
			}else{
				
				/*
				 *	Push the entity out of the wall if it's trying to go through it.
				*/
				
				do{
					e->loc.x+=.001 * e->vel.x>0?-1:1;
					
					i=(e->loc.x - e->width / 2 - x_start) / HLINE;
					if(i < 0){ e->alive = false; return; }
					if(i > lineCount-1){ e->alive = false; return; }
				}while(line[i].y>e->loc.y+ e->height);
				
			}
		
		/*
		 *	Handle gravity if the entity is above the line.
		*/
		
		}else{
			
			if(e->vel.y > -2)e->vel.y-=.003 * deltaTime;
			
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
	
	/*
	 *	Handle the player. 
	*/
	
	singleDetect(p);
		
	/*
	 *	Handle all remaining entities in this world. 
	*/
	
	for(auto &e : entity)
		singleDetect(e);
}

void World::addStructure(_TYPE t,float x,float y,World *outside,World *inside){
	build.push_back(new Structures());
	build.back()->spawn(t,x,y);
	build.back()->inWorld=outside;
	build.back()->inside=(void *)inside;
	
	entity.push_back(build.back());
}

/*template<class T>
void World::getEntityLocation(std::vector<T*>&vecBuf, unsigned int n){
	T bufVar = vecBuf.at(n);
	unsigned int i = 0;
	for(auto &e : entity){
		if(entity.at(i) == bufVar){
			entity.erase(entity.begin()+i);
		}
		i++;
	}
}*/
	
void World::addMob(int t,float x,float y){
	mob.push_back(new Mob(t));
	mob.back()->spawn(x,y);
	
	entity.push_back(mob.back());
}

void World::addMob(int t,float x,float y,void (*hey)(Mob *)){
	mob.push_back(new Mob(t));
	mob.back()->spawn(x,y);
	mob.back()->hey = hey;
	
	entity.push_back(mob.back());
}

void World::addNPC(float x,float y){
	npc.push_back(new NPC());
	npc.back()->spawn(x,y);
	
	entity.push_back(npc.back());
}

void World::addObject(ITEM_ID i, bool q, const char *p, float x, float y){
	object.push_back(new Object(i,q, p));
	object.back()->spawn(x,y);

	entity.push_back(object.back());
}

/*void World::removeObject(Object i){
	object.delete[](i);
}*/

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
	if(toLeft&&p->loc.x<x_start+HLINE*15){
		p->loc.x=toLeft->x_start+getWidth(toLeft)-HLINE*10;
		p->loc.y=toLeft->line[toLeft->lineCount-GEN_INC-1].y;
		return toLeft;
	}
	return this;
}

World *World::goWorldRight(Player *p){
	if(toRight&&p->loc.x+p->width>x_start+getWidth(this)-HLINE*10){
		p->loc.x=toRight->x_start+HLINE*10;
		p->loc.y=toRight->line[0].y;
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

std::vector<void *>thing;
World *World::goInsideStructure(Player *p){
	if(!thing.size()){
		for(auto &b : build){
			if(p->loc.x            > b->loc.x            &&
			   p->loc.x + p->width < b->loc.x + b->width ){
				thing.push_back(this);
				return (World *)b->inside;
			}
		}
	}else{
		for(auto &b : ((World *)thing.back())->build){
			if(b->inside == this){
				World *tmp = (World *)thing.back();
				p->loc.x = b->loc.x + (b->width / 2) - (p->width / 2);
				thing.erase(thing.end()-1);
				return tmp;
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

int World::getTheWidth(void){
	World *hey=this;
LOOP:
	if(hey->infront){
		hey=hey->infront;
		goto LOOP;
	}
	return -hey->x_start*2;
}

IndoorWorld::IndoorWorld(void){
}

IndoorWorld::~IndoorWorld(void){
	delete bgTex;
	delete[] star;
	delete[] line;

	deleteEntities();
}

void IndoorWorld::generate(unsigned int width){		// Generates a flat area of width 'width'
	unsigned int i;						// Used for 'for' loops 
	lineCount=width+GEN_INC;			// Sets line count to the desired width plus GEN_INC to remove incorrect line calculations.
	if(lineCount<=0)abort();
	
	line = new struct line_t[lineCount];	//(struct line_t *)calloc(lineCount,sizeof(struct line_t));	// Allocate memory for the array 'line'
	memset(line,0,lineCount*sizeof(struct line_t));
	
	for(i=0;i<lineCount;i++){			// Indoor areas don't have to be directly on the ground (i.e. 0)...
		line[i].y=INDOOR_FLOOR_HEIGHT;
	}
	behind=infront=NULL;						// Set pointers to other worlds to NULL
	toLeft=toRight=NULL;						// to avoid accidental calls to goWorld... functions
	x_start=0-getWidth(this)/2+GEN_INC/2*HLINE;	// Calculate x_start (explained in world.h)
}

void IndoorWorld::draw(Player *p){
	int i,ie,v_offset;
	
	glEnable(GL_TEXTURE_2D);
	bgTex->bind(0);
	glColor4ub(255,255,255,255);
	glBegin(GL_QUADS);
		for(i = x_start - SCREEN_WIDTH / 2;i < -x_start + SCREEN_WIDTH / 2; i += 1024){
			glTexCoord2i(1,1);glVertex2i(i     ,0);
			glTexCoord2i(0,1);glVertex2i(i+1024,0);
			glTexCoord2i(0,0);glVertex2i(i+1024,1024);
			glTexCoord2i(1,0);glVertex2i(i     ,1024);
		}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	
	v_offset=(p->loc.x-x_start)/HLINE;					// Calculate the player's offset in the array 'line' using the player's location 'vec'
	i=v_offset-SCREEN_WIDTH/2;							// um
	if(i<0)i=0;											// If the player is past the start of that world 'i' should start at the beginning
														// of the world
	ie=v_offset+SCREEN_WIDTH/2;							// Set how many lines should be drawn (the drawing for loop loops from 'i' to 'ie')
	if(ie>lineCount)ie=lineCount;						// If the player is past the end of that world 'ie' should contain the end of that world
	//glClearColor(.3,.1,0,0);
	
	glBegin(GL_QUADS);
		for(i=i;i<ie-GEN_INC;i++){						// For lines in array 'line' from 'i' to 'ie'
			safeSetColor(150,100,50);
			glVertex2i(x_start+i*HLINE      ,line[i].y);	// Draw the base floor
			glVertex2i(x_start+i*HLINE+HLINE,line[i].y);
			glVertex2i(x_start+i*HLINE+HLINE,line[i].y-50);
			glVertex2i(x_start+i*HLINE		,line[i].y-50);
		}
	glEnd();
	for(i=0;i<entity.size();i++)
		entity[i]->draw();
	p->draw();
}

extern bool inBattle;

Arena::Arena(World *leave,Player *p){
	generate(300);
	door.y = line[299].y;
	door.x = 100;
	exit = leave;
	
	npc.push_back(new NPC());
	entity.push_back(npc.back());
	entity.back()->spawn(door.x,door.y);
	entity.back()->width = HLINE * 12;
	entity.back()->height = HLINE * 16;
	
	inBattle = true;
	pxy = p->loc;
}

Arena::~Arena(void){
	delete bgTex;
	delete[] star;
	delete[] line;

	deleteEntities();
}

World *Arena::exitArena(Player *p){
	npc[0]->loc.x = door.x;
	npc[0]->loc.y = door.y;
	if(p->loc.x + p->width / 2 > door.x				 &&
	   p->loc.x + p->width / 2 < door.x + HLINE * 12 ){
		inBattle = false;
		p->loc = pxy;
		return exit;
	}else{
		return this;
	}
}
