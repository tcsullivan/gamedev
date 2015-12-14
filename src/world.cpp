#include <world.h>
#include <ui.h>

#define getWidth(w) ((w->lineCount-GEN_INC)*HLINE)	// Calculates the width of world 'w'

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
	World *tmp = w;
	float base = GEN_MIN;
	while(tmp->infront){
		tmp = tmp->infront;
		base -= DRAW_Y_OFFSET;
	}
	return base;
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

void World::save(FILE *s){
	fclose(s);
}

void World::load(FILE *s){
	fclose(s);
}

World::World(void){
	
	bgm = NULL;
	bgmObj = NULL;
	
	/*
	 *	Nullify pointers to other worlds.
	*/
	
	behind	=
	infront	=
	toLeft	=
	toRight	= NULL;
	
	/*
	 *	Allocate and clear an array for star coordinates.
	*/
	
	star = new vec2[100];
	memset(star,0,100 * sizeof(vec2));
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
	while(!entity.empty()){
		entity.pop_back();
	}

	while(!particles.empty()){
		delete particles.back();
		particles.pop_back();
	}//particles.clear();
}

World::~World(void){
	if(behind != NULL)
		delete behind;
	
	if(bgmObj)
		Mix_FreeMusic(bgmObj);
	if(bgm)
		delete[] bgm;
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
	
	if(width <= 0)
		abort();
		
	lineCount = width + GEN_INC;
	
	/*
	 *	Allocate enough memory for the world to be stored.
	*/
	
	line = new struct line_t[lineCount];
	memset(line,0,lineCount * sizeof(struct line_t));
	
	/*
	 *	Set an initial y to base generation off of, as generation references previous lines.
	*/
	
	line[0].y = GEN_INIT;
	
	/*
	 *	Populate every GEN_INCth line structure. The remaining lines will be based off of these.
	*/
	
	for(i = GEN_INC;i < lineCount;i += GEN_INC){
		
		/*
		 *	Generate a y value, ensuring it stays within a reasonable range.
		*/
		
		line[i].y = rand() % 8 - 4 + line[i - GEN_INC].y;	// Add +/- 4 to the previous line
		
			 if(line[i].y < GEN_MIN)line[i].y = GEN_MIN;	// Minimum bound
		else if(line[i].y > GEN_MAX)line[i].y = GEN_MAX;	// Maximum bound
		
	}
	
	/*
	 *	Generate values for the remaining lines here.
	*/
	
	for(i = 0;i < lineCount - GEN_INC;i++){
		
		/*
		 *	Every GEN_INCth line calculate the slope between the current line and the one
		 *	GEN_INC lines before it. This value is then divided into an increment that is
		 *	added to lines between these two points resulting in a smooth slope.
		 * 
		*/
		
		if(!(i % GEN_INC))
			
			inc = (line[i + GEN_INC].y - line[i].y) / (float)GEN_INC;
			
		/*
		 *	Add the increment to create the smooth slope.
		*/
			
		else line[i].y = line[i - 1].y + inc;
		
		/*
		 *	Generate a color value for the line. This will be referenced in World->draw(),
		 *	by setting an RGB value of color (red), color - 50 (green), color - 100 (blue).
		*/
		
		line[i].color = rand() % 20 + 100; // 100 to 120

		/*
		 *	Each line has two 'blades' of grass, here we generate random heights for them.
		*/
		
		line[i].gh[0] = (getRand() % 16) / 3.5 + 2;	// Not sure what the range resolves to here...
		line[i].gh[1] = (getRand() % 16) / 3.5 + 2;	//
		
		line[i].gs = true;							// Show the blades of grass (modified by the player)
		
	}
	
	/*
	 *	Calculate the x coordinate to start drawing this world from so that it is centered at (0,0).
	*/
	
	x_start = 0 - getWidth(this) / 2;
	
	/*
	 *	Assign coordinates for the stars, seen at nighttime.
	*/
	
	for(i = 0;i < 100;i++){
		star[i].x = getRand() % getTheWidth() - getTheWidth() / 2;
		star[i].y = getRand() % SCREEN_HEIGHT + 100;
	}
}

void World::generateFunc(unsigned int width,float(*func)(float)){
	unsigned int i;
	
	/*
	 *	Check for a valid width.
	*/
	
	if((lineCount = width) <= 0)
		abort();
	
	/*
	 *	Allocate memory for the line array.
	*/
	
	line = new struct line_t[lineCount];
	memset(line,0,lineCount*sizeof(struct line_t));
	
	/*
	 *	Populate entries in the line array, using `func` to get y values.
	*/
	
	for(i = 0;i < lineCount;i++){
		line[i].y = func(i);
		
		if(line[i].y <    0)line[i].y = 0;		// Minimum bound
		if(line[i].y > 2000)line[i].y = 2000;	// Maximum bound
		
		line[i].color = rand() % 20 + 100;
		
		line[i].gh[0] = (getRand() % 16) / 3.5 + 2;
		line[i].gh[1] = (getRand() % 16) / 3.5 + 2;
		
		line[i].gs = true;
	}
	
	x_start = 0 - getWidth(this) / 2;
	
	for(i = 0;i < 100;i++){
		star[i].x = getRand() % getTheWidth() - getTheWidth() / 2;
		star[i].y = getRand() % SCREEN_HEIGHT + 100;
	}
}

void World::update(Player *p,unsigned int delta){
	
	/*
	 *	Update the player's coordinates.
	*/
	
	p->loc.y += p->vel.y			 * delta;
	p->loc.x +=(p->vel.x * p->speed) * delta;
	if(p->inv->usingi){
		p->inv->useItem();
	}

	/*
	 *	Update coordinates of all entities except for structures.
	*/

	for(auto &e : entity){
		e->loc.y += e->vel.y * delta;
		if(e->type != STRUCTURET && e->canMove){
			e->loc.x += e->vel.x * delta;
			if(e->vel.x < 0)e->left = true;
	   else if(e->vel.x > 0)e->left = false;
		}
	}
	uint oh = 0;
	for(auto &pa : particles){
		if(pa->kill(deltaTime)){
			delete pa;
			particles.erase(particles.begin()+oh);
		}else if(pa->canMove){
			pa->loc.y += pa->vely * deltaTime;
			pa->loc.x += pa->velx * deltaTime;

			for(auto &b : build){
				if(b->bsubtype==FOUNTAIN){
					if(pa->loc.x >= b->loc.x && pa->loc.x <= b->loc.x+b->width){
						if(pa->loc.y <= b->loc.y + b->height*.25){
							delete pa;
							particles.erase(particles.begin()+oh);
						}
					}
				}
			}
		}
		oh++;
	}oh=0;
	
	if(ui::dialogImportant){
		Mix_FadeOutMusic(2000);
	}else if(!Mix_PlayingMusic()){
		Mix_FadeInMusic(bgmObj,-1,2000);
	}
}

void World::setBGM(const char *path){
	if(!bgm) delete[] bgm;
	if(path != NULL){
		bgm = new char[strlen(path) + 1];
		strcpy(bgm,path);
		bgmObj = Mix_LoadMUS(bgm);
	}else{
		bgm = new char[1];
		bgm[0] = '\0';
	}
}

void World::bgmPlay(World *prev){	
	if(!prev || strcmp(bgm,prev->bgm)){
		Mix_VolumeMusic(50);
		Mix_PlayMusic(bgmObj,-1);	// Loop infinitely
	}/*else{
		Mix_FadeOutMusic(800);
	}*/
}

int worldShade = 0;

extern vec2 offset;
extern unsigned int tickCount;

void World::draw(Player *p){
	static float yoff=DRAW_Y_OFFSET;	// Initialize stuff
	static int shade,bgshade;
	static World *current=this;
	unsigned int i;
	int is,ie,v_offset,cx_start,width;
	struct line_t *cline;
	float base;
	
	bgshade = worldShade << 1; // *2
	base = worldGetYBase(this);
	
	/*
	 *	Draw the background images in the appropriate order.
	*/
	
LLLOOP:
	if(current->infront){
		current=current->infront;
		goto LLLOOP;
	}
	cx_start = current->x_start * 1.5;
	width = (-x_start) << 1;
	
	glEnable(GL_TEXTURE_2D);
	
	bgTex->bind(0);
	safeSetColorA(255,255,255,255 - worldShade * 4);
	
	glBegin(GL_QUADS);
		glTexCoord2i(0,0);glVertex2i( cx_start,SCREEN_HEIGHT);
		glTexCoord2i(1,0);glVertex2i(-cx_start,SCREEN_HEIGHT);
		glTexCoord2i(1,1);glVertex2i(-cx_start,0);
		glTexCoord2i(0,1);glVertex2i( cx_start,0);
	glEnd();
	
	bgTex->bindNext();
	safeSetColorA(255,255,255,worldShade * 4);
	
	glBegin(GL_QUADS);
		glTexCoord2i(0,0);glVertex2i( cx_start,SCREEN_HEIGHT);
		glTexCoord2i(1,0);glVertex2i(-cx_start,SCREEN_HEIGHT);
		glTexCoord2i(1,1);glVertex2i(-cx_start,0);
		glTexCoord2i(0,1);glVertex2i( cx_start,0);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	/*
	 *	Draws stars if it is an appropriate time of day for them.
	*/

	if((((weather==DARK )&(tickCount%DAY_CYCLE))<DAY_CYCLE/2)   ||
	   (((weather==SUNNY)&(tickCount%DAY_CYCLE))>DAY_CYCLE*.75) ){
		   
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
			glTexCoord2i(0,1);glVertex2i(width/-2+(1920*i    )+offset.x*.85,base);
			glTexCoord2i(1,1);glVertex2i(width/-2+(1920*(i+1))+offset.x*.85,base);
			glTexCoord2i(1,0);glVertex2i(width/-2+(1920*(i+1))+offset.x*.85,base+1080);
			glTexCoord2i(0,0);glVertex2i(width/-2+(1920*i    )+offset.x*.85,base+1080);
		}
	glEnd();
	
	/*
	 *	Draw three layers of trees.
	*/

	for(i = 0; i < 3; i++){
		bgTex->bindNext();
		safeSetColorA(bgDraw[i][0]-bgshade,bgDraw[i][0]-bgshade,bgDraw[i][0]-bgshade,bgDraw[i][1]);
	
		glBegin(GL_QUADS);
			for(int j = cx_start; j <= -cx_start; j += 600){
				glTexCoord2i(0,1);glVertex2i( j     +offset.x*bgDraw[i][2],base);
				glTexCoord2i(1,1);glVertex2i((j+600)+offset.x*bgDraw[i][2],base);
				glTexCoord2i(1,0);glVertex2i((j+600)+offset.x*bgDraw[i][2],base+400);
				glTexCoord2i(0,0);glVertex2i( j     +offset.x*bgDraw[i][2],base+400);
			}
		glEnd();
	}
	
	glDisable(GL_TEXTURE_2D);
	
	glColor3ub(0,0,0);
	glRectf(cx_start,GEN_MIN,-cx_start,0);
	
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
	if(ie>(int)current->lineCount)ie=current->lineCount;		// Maximum bound
	else if(ie < GEN_INC)ie = GEN_INC;
	
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
		b->draw();
	}
	
	/*
	 *	Draw the layer up until the grass portion, which is done later.
	*/

	bool hey=false;
	glBegin(GL_QUADS);
		for(i=is;i<(unsigned)ie-GEN_INC;i++){
			cline[i].y+=(yoff-DRAW_Y_OFFSET);															// Add the y offset
			if(!cline[i].y){
				cline[i].y=base;
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
		for(i=is;i<(unsigned)ie-GEN_INC;i++){
			
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
	for(auto &part : particles){part->draw();}
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
		
		unsigned int ph = (p->loc.x + p->width / 2 - x_start) / HLINE;
		
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
	unsigned int i,j;
	int l;
	
	/*
	 *	Kill any dead entities.
	*/
	
	if(!e->alive||e->health<=0){
		for(i=0;i<entity.size();i++){
			if(entity[i]==e){
				switch(e->type){
				case STRUCTURET:
					std::cout<<"Killed a building..."<<std::endl;
					for(j=0;j<build.size();j++){
						if(build[j]==e){
							delete build[j];
							build.erase(build.begin()+j);
							break;
						}
					}
					break;
				case NPCT:
					std::cout<<"Killed an NPC..."<<std::endl;
					for(j=0;j<npc.size();j++){
						if(npc[j]==e){
							delete npc[j];
							npc.erase(npc.begin()+j);
							break;
						}
					}
					break;
				case MOBT:
					std::cout<<"Killed a mob..."<<std::endl;
					for(j=0;j<mob.size();j++){
						if(mob[j]==e){
							delete mob[j];
							mob.erase(mob.begin()+j);
							break;
						}
					}
					break;
				case OBJECTT:
					std::cout<<"Killed an object..."<<std::endl;
					for(j=0;j<object.size();j++){
						if(object[j]==e){
							delete object[j];
							object.erase(object.begin()+j);
							break;
						}
					}
					break;
				case PLAYERT:
					std::cout<<"RIP "<<e->name<<"."<<std::endl;
					exit(0);
					break;
				}
				entity.erase(entity.begin()+i);
				return;
			}
		}
	}
	
	/*
	 *	Handle only living entities.
	*/
	
	if(e->alive){
	  
		if(e->type == MOBT && Mobp(e)->subtype == MS_TRIGGER)return;
	  
		/*
		 *	Calculate the line that this entity is currently standing on.
		*/
		
		l=(e->loc.x + e->width / 2 - x_start) / HLINE;
		if(l < 0) l=0;
		i = l;
		if(i > lineCount-1) i=lineCount-1;
		
		/*
		 *	If the entity is under the world/line, pop it back to the surface.
		*/
		
		if(e->loc.y < line[i].y){
			
			/*
			 *	Check that the entity isn't trying to run through a wall.
			*/
			
			//if(e->loc.y + e->height > line[i-(int)e->width/2/HLINE].y &&
			 //  e->loc.y + e->height > line[i+(int)e->width/2/HLINE].y ){
			
				e->loc.y=line[i].y - .001 * deltaTime;
				e->ground=true;
				e->vel.y=0;
				
			//}else{
				
				/*
				 *	Push the entity out of the wall if it's trying to go through it.
				*/
				
				/*do{
					e->loc.x+=.001 * e->vel.x>0?-1:1;
					
					l=(e->loc.x - e->width / 2 - x_start) / HLINE;
					if(l < 0){
						std::cout<<"push kill lol "<<e->type<<std::endl;
						e->alive = false; return; }
					i = l;
					if(i > lineCount-1){
						std::cout<<"push kill lol "<<e->type<<std::endl;
						e->alive = false; return; }
				}while(line[i].y>e->loc.y+ e->height);
				
			}*/
		
		/*
		 *	Handle gravity if the entity is above the line.
		*/
		
		}else{
			
			if(e->type == STRUCTURET && e->loc.y > 2000){
				e->loc.y = line[i].y;
				e->vel.y = 0;
				e->ground = true;
				return;
			}else if(e->vel.y > -2)e->vel.y-=.003 * deltaTime;
					
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
	World *hey = this;
	
	/*
	 *	Handle the player. 
	*/
	
	singleDetect(p);
		
	/*
	 *	Handle all remaining entities in this world. 
	*/
	
LOOOOP:
	static int what = 0;
	for(auto &e : hey->entity)
		hey->singleDetect(e);
	for(auto &part : particles){
		int l;
		unsigned int i;
		l=(part->loc.x + part->width / 2 - x_start) / HLINE;
		if(l < 0) l=0;
		i = l;
		if(i > lineCount-1) i=lineCount-1;
		if(part->loc.y < line[i].y){
			part->loc.y = line[i].y;
			part->vely = 0;
			part->velx = 0;
			part->canMove = false;
		}else{
			if(part->vely > -2)part->vely-=.003 * deltaTime;
		}
		what++;
	}what=0;
	if(hey->infront){
		hey = hey->infront;
		goto LOOOOP;
	}
}
void World::addStructure(_TYPE t,BUILD_SUB sub, float x,float y,World *outside,World *inside){
	build.push_back(new Structures());
	build.back()->spawn(t,sub,x,y);
	build.back()->inWorld=outside;
	build.back()->inside=(void *)inside;
	
	entity.push_back(build.back());
}
	
void World::addVillage(int bCount, int npcMin, int npcMax,_TYPE t,float x,float y,World *outside,World *inside){
	std::cout << npcMin << ", " << npcMax << std::endl;
	int xwasd;
	for(int i = 0; i < bCount; i++){
		xwasd = (rand()%(int)x+1000*HLINE);
		HERE:
		for(auto &bu : build){
			if(xwasd > bu->loc.x && xwasd < bu->loc.x+bu->width)goto HERE;
		}
		addStructure(t,HOUSE,xwasd,y,outside,inside);
	}
}
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

void World::addParticle(float x, float y, float w, float h, float vx, float vy, Color color, int d){
	particles.push_back(new Particles(x,y,w,h,vx,vy,color,d));
	particles.back()->canMove = true;
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
	behind->star=star;
	behind->bgmObj=bgmObj;
	behind->bgTex=bgTex;
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

bool World::isWorldLeft(void){
	return toLeft ? true : false;
}

bool World::isWorldRight(void){
	return toRight ? true : false;
}

std::vector<void *>thing;
World *World::goInsideStructure(Player *p){
	if(!thing.size()){
		for(auto &b : build){
			if(p->loc.x            > b->loc.x            &&
			   p->loc.x + p->width < b->loc.x + b->width ){
				thing.push_back(this);
				ui::toggleBlackFast();
				ui::waitForCover();
				ui::toggleBlackFast();
				return (World *)b->inside;
			}
		}
	}else{
		for(auto &b : ((World *)thing.back())->build){
			if(b->inside == this){
				World *tmp = (World *)thing.back();
				p->loc.x = b->loc.x + (b->width / 2) - (p->width / 2);
				thing.erase(thing.end()-1);
				ui::toggleBlackFast();
				ui::waitForCover();
				ui::toggleBlackFast();
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
	unsigned int i,ie;
	int j,x,v_offset;
	
	/*
	 *	Draw the background.
	*/
	
	glEnable(GL_TEXTURE_2D);
	
	bgTex->bind(0);
	glColor4ub(255,255,255,255);
	
	glBegin(GL_QUADS);	
		for(j = x_start - SCREEN_WIDTH / 2;j < -x_start + SCREEN_WIDTH / 2; j += 512){
			glTexCoord2i(1,1);glVertex2i(j    ,0);
			glTexCoord2i(0,1);glVertex2i(j+512,0);
			glTexCoord2i(0,0);glVertex2i(j+512,512);
			glTexCoord2i(1,0);glVertex2i(j    ,512);
		}
	glEnd();
	
	glDisable(GL_TEXTURE_2D);
	
	/*
	 *	Calculate the starting and ending points to draw the ground from.
	*/
	
	v_offset = (p->loc.x - x_start) / HLINE;
	j = v_offset - (SCREEN_WIDTH / 2 / HLINE) - GEN_INC;
	if(j < 0)j = 0;
	i = j;
	
	ie = v_offset + (SCREEN_WIDTH / 2 / HLINE) - GEN_INC;
	if(ie > lineCount)ie = lineCount;
	
	/*
	 *	Draw the ground.
	*/
	
	glBegin(GL_QUADS);
		for(;i < ie - GEN_INC;i++){
			safeSetColor(150,100,50);
			
			x = x_start + i * HLINE;
			glVertex2i(x		,line[i].y);
			glVertex2i(x + HLINE,line[i].y);
			glVertex2i(x + HLINE,line[i].y - 50);
			glVertex2i(x		,line[i].y - 50);
		}
	glEnd();
	
	/*
	 *	Draw all entities.
	*/
	
	for(auto &e : entity) e->draw();
	p->draw();
}

extern bool inBattle;

Arena::Arena(World *leave,Player *p){
	generate(300);
	addMob(MS_DOOR,100,100);
	inBattle = true;
	exit = leave;
	pxy = p->loc;
	
	star = new vec2[100];
	memset(star,0,100 * sizeof(vec2));
}

Arena::~Arena(void){
	delete bgTex;
	delete[] star;
	delete[] line;

	deleteEntities();
}

World *Arena::exitArena(Player *p){
	if(p->loc.x + p->width / 2 > mob[0]->loc.x				&&
	   p->loc.x + p->width / 2 < mob[0]->loc.x + HLINE * 12 ){
		inBattle = false;
		ui::toggleBlackFast();
		ui::waitForCover();
		p->loc = pxy;
		return exit;
	}else{
		return this;
	}
}
