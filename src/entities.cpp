#include <entities.h>
#include <ui.h>

std::vector<Entity		*>	entity;
std::vector<NPC			*>	npc;
std::vector<Structures	*>	build;
std::vector<Mob			*>	mob;

extern FILE* names;
extern unsigned int loops;

void Entity::spawn(float x, float y){	//spawns the entity you pass to it based off of coords and global entity settings
	loc.x = x;
	loc.y = y;
	vel.x = 0;
	vel.y = 0;
	
	alive	= true;
	right	= true;
	left	= false;
	near	= false;
	canMove	= true;
	ground	= false;
	
	ticksToUse = 0;
	
	if(!maxHealth)health = maxHealth = 1;
	
	if(type==MOBT)
		Mobp(this)->init_y=loc.y;
	
	name = (char*)malloc(16);
	getName();
}

Player::Player(){ //sets all of the player specific traits on object creation
	width = HLINE * 10;
	height = HLINE * 16;
	
	type = PLAYERT; //set type to player
	subtype = 0;	
	health = maxHealth = 100;
	speed = 1;
	tex = new Texturec(3, "assets/player1.png", "assets/player.png", "assets/player2.png");
	inv = new Inventory(PLAYER_INV_SIZE);
}

NPC::NPC(){	//sets all of the NPC specific traits on object creation
	width = HLINE * 10;
	height = HLINE * 16;
	
	type	= NPCT; //sets type to npc
	subtype = 0;
	
	maxHealth = health = 100;
	
	tex = new Texturec(1,"assets/NPC.png");
	inv = new Inventory(NPC_INV_SIZE);
}

Structures::Structures(){ //sets the structure type
	health = maxHealth = 1;
	
	alive = false;
	near  = false;
	
	tex = new Texturec(1,"assets/house1.png");
}

Mob::Mob(int sub){
	width  = HLINE * 10;
	height = HLINE * 8;
	type   = MOBT;
	
	maxHealth = health = 50;
	
	switch((subtype = sub)){
	case MS_RABBIT:
		tex = new Texturec(2, "assets/rabbit.png", "assets/rabbit1.png");
		break;
	case MS_BIRD:
		break;
	}
	
	inv = new Inventory(NPC_INV_SIZE);
}

void Entity::draw(void){		//draws the entities
	glPushMatrix();
	if(type==NPCT){
		if(NPCp(this)->aiFunc.size()){
			glColor3ub(255,255,0);
			glRectf(loc.x+width/3,loc.y+height,loc.x+width*2/3,loc.y+height+width/3);
		}if(gender == MALE){
			glColor3ub(255,255,255);
		}else if(gender == FEMALE){
			glColor3ub(255,105,180);
		}
	}else{
		glColor3ub(255,255,255);
	}
	if(left){
		glScalef(-1.0f,1.0f,1.0f);
		glTranslatef(0-width-loc.x*2,0,0);
	}
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	if(type == PLAYERT){
		static int texState = 0;
		static bool up = true;
		if(loops % (int)((float)5 / (float)speed) == 0){
			if(up){
				texState+=1;
				if(texState==2)up=false;
				tex->bindNext();
			}else if(!up){
				texState-=1;
				if(texState==0)up=true;
				tex->bindPrev();
			}
		}
		if(ground == 0){
			tex->bind(0);
		}else if(vel.x != 0){
			switch(texState){
				case 0:
					tex->bind(0);
				break;
				case 1:
					tex->bind(1);
				break;
				case 2:
					tex->bind(2);
				break;
			}
		}
		else{
			tex->bind(1);
		}
	}else if(type == MOBT){
		switch(subtype){
			case 1: //RABBIT
				if(ground == 0){
					tex->bind(1);
				}else if(ground == 1){
					tex->bind(0);
				}
				break;
			default:
			break;
		}
	}else{
		tex->bind(0);
	}
	glColor3ub(255,255,255);
	glBegin(GL_QUADS);
		glTexCoord2i(0,1);glVertex2i(loc.x, loc.y);
		glTexCoord2i(1,1);glVertex2i(loc.x + width, loc.y);
		glTexCoord2i(1,0);glVertex2i(loc.x + width, loc.y + height);
		glTexCoord2i(0,0);glVertex2i(loc.x, loc.y + height);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	if(near){
		ui::setFontSize(14);
		ui::putText(loc.x,loc.y-ui::fontSize-HLINE/2,"%s",name);
	}
}

void Entity::getName(){
	rewind(names);
	char buf,*bufs = (char *)malloc(16);
	int tempNum,max = 0;
	for(;!feof(names);max++){
		fgets(bufs,16,(FILE*)names);
	}
	tempNum = rand()%max;
	rewind(names);
	for(int i=0;i<tempNum;i++){
		fgets(bufs,16,(FILE*)names);
	}
	switch(fgetc(names)){
	case 'm':
		gender = MALE;
		//std::puts("Male");
		break;
	case 'f':
		gender = FEMALE;
		//std::puts("Female");
		break;
	default:
		break;
	}
	if((fgets(bufs,16,(FILE*)names)) != NULL){
		//std::puts(bufs);
		bufs[strlen(bufs)-1] = '\0';
		strcpy(name,bufs);
	}
	free(bufs);
}

void Player::interact(){ //the function that will cause the player to search for things to interact with
	
}

/*
 *	NPC::wander, this makes the npcs wander around the near area
 *
 *	timeRun					This variable is the amount of gameloop ticks the entity will wander for
 *
 *	*v 						This is a pointer to whatever vec2 value is passed to it, usually the value
 *							passed is the entities vec2 for coordinates. Since the value is a pointer
 *							the memory address passed to it is directly modified.
*/

void NPC::wander(int timeRun, vec2 *v){ //this makes the entites wander about
	/*
	 *	Direction is the variable that decides what direction the entity will travel in
	 *	the value is either -1, 0, or 1. -1 being left, 0 means that the npc will stay still
	 *	and a value of 1 makes the entity move to the right
	*/
	static int direction;	//variable to decide what direction the entity moves
	/*
	 *	Ticks to use is a variable in the entity class that counts the total ticks that need to be used
	 *
	 *	This loop only runs when ticksToUse is 0, this means that the speed, direction, etc... Will be
	 *	calculated only after the npc has finished his current walking state
	*/
	if(ticksToUse == 0){
		ticksToUse = timeRun;
		v->x = .008*HLINE;	//sets the inital velocity of the entity
		direction = (getRand() % 3 - 1); 	//sets the direction to either -1, 0, 1
											//this lets the entity move left, right, or stay still
		if(direction==0)ticksToUse*=2;
		v->x *= direction;	//changes the velocity based off of the direction
	}
	ticksToUse--; //removes one off of the entities timer
}

std::vector<int (*)(NPC *)> AIpreload;	// A dynamic array of AI functions that are being preloaded
std::vector<NPC *> AIpreaddr;			// A dynamic array of pointers to the NPC's that are being assigned the preloads

void NPC::addAIFunc(int (*func)(NPC *),bool preload){
	if(preload){										// Preload AI functions so that they're given after 
														// the current dialog box is closed
		AIpreload.push_back(func);
		AIpreaddr.push_back(this);
	}
	else aiFunc.push_back(func);
}

void NPC::interact(){ //have the npc's interact back to the player
	int (*func)(NPC *);
	loc.y += 5;
	if(aiFunc.size()){
		func=aiFunc.front();
		canMove=false;
		if(!func(this)){
			aiFunc.erase(aiFunc.begin());
		}
		canMove=true;
	}
}

/*
 *	This spawns the structures
 *
 * Structures::spawn		This allows us to spawn buildings and large amounts of entities with it.
 *							Passed are the type and x and y coordinates. These set the x and y coords
 *							of the structure being spawned, the type pass just determines what rules to follow
 *							when spawing the structure and entities. In theory this function can also spawn
 *							void spawn points for large amounts of entities. This would allow for the spawn
 *							point to have non-normal traits so it could be invisible or invincible...
*/
unsigned int Structures::spawn(_TYPE t, float x, float y){ //spawns a structure based off of type and coords
	loc.x = x;
	loc.y = y;
	type = t;
	
	alive = true;

	/*VILLAGE*/
	switch(type){
	case STRUCTURET:
		width =  50 * HLINE;
		height = 40 * HLINE;

		/*
		 *	tempN is the amount of entities that will be spawned in the village. Currently the village
		 *	will spawn bewteen 2 and 7 villagers for the starting hut.
		*/
		
		unsigned int tempN = (getRand() % 5 + 2);
		
		for(int i=0;i<tempN;i++){
			
			/*
			 *	This is where the entities actually spawn. A new entity is created
			 *	with type NPC by using polymorphism.
			*/
			
			npc.push_back(new NPC());
			npc.back()->spawn(loc.x+(i-5),100);
			
			entity.push_back(npc.back());
			
		}
		break;
	}
	return 0;
}

/*
 * 	Mob::wander this is the function that makes the mobs wander around
 *
 *	See NPC::wander for the explaination of the argument's variable
*/

void Mob::wander(int timeRun){
	static int direction;	//variable to decide what direction the entity moves
	switch(subtype){
	case MS_RABBIT:
		if(!ticksToUse){
			ticksToUse = timeRun;
			direction = (getRand() % 3 - 1); 	//sets the direction to either -1, 0, 1
												//this lets the entity move left, right, or stay still
			if(direction==0)ticksToUse/=2;
			vel.x *= direction;	//changes the velocity based off of the direction
		}
		if(ground && direction){
			vel.y=.15;
			loc.y+=HLINE*.25;
			ground=false;
			vel.x = (.07*HLINE)*direction;	//sets the inital velocity of the entity
		}
		ticksToUse--; //removes one off of the entities timer
		break;
	case MS_BIRD:
		if(loc.y<=init_y-.2)vel.y+=.005*deltaTime;	// TODO handle direction
		break;
	default:
		break;
	}
}
