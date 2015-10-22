#include <entities.h>
#include <ui.h>

extern std::vector<Entity*>entity;
//extern std::vector<NPC>npc;
extern std::vector<Structures>build;

extern FILE* names;
extern unsigned int loops;

void Entity::spawn(float x, float y){	//spawns the entity you pass to it based off of coords and global entity settings
	loc.x = x;
	loc.y = y;
	vel.x = 0;
	vel.y = 0;
	alive = true;
	right = true;
	left  = false;
	near  = false;
	canMove = true;
	ground  = false;
	ticksToUse = 0;
	if(!maxHealth)health = maxHealth = 1;
	name = (char*)malloc(16);
	getName();
}

Player::Player(){ //sets all of the player specific traits on object creation
	width = HLINE * 10;
	height = HLINE * 16;
	speed = 1;
	type = PLAYERT; //set type to player
	subtype = 0;
	maxHealth = 100;
	health = maxHealth;
	alive = true;
	ground = false;
	near = true;
	inv = new Inventory(PLAYER_INV_SIZE);
	tex = new Texturec(3, "assets/player.png", "assets/player1.png", "assets/player2.png");
}

NPC::NPC(){	//sets all of the NPC specific traits on object creation
	width = HLINE * 10;
	height = HLINE * 16;
	speed = 1;
	type = NPCT; //sets type to npc
	subtype = 0;
	alive = true;
	canMove = true;
	near = false;
	texture[0] = Texture::loadTexture("assets/NPC.png");
	texture[1] = 0;
	texture[2] = 0;
	//tex = new Texturec("assets/NPC.png");
	inv = new Inventory(NPC_INV_SIZE);
}

Structures::Structures(){ //sets the structure type
	type = STRUCTURET;
	speed = 0;
	alive = true;
	near = false;
	texture[0] = Texture::loadTexture("assets/house1.png");
	texture[1] = 0;
	texture[2] = 0;
}

Mob::Mob(){
	width = HLINE * 10;
	height = HLINE * 8;
	speed = 1;
	type = MOBT; //sets type to MOB
	subtype = 1; //SKIRL
	alive = true;
	canMove = true;
	near = false;
	texture[0] = Texture::loadTexture("assets/rabbit.png");
	texture[1] = Texture::loadTexture("assets/rabbit1.png");
	texture[2] = 0;
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
			}else if(!up){
				texState-=1;
				if(texState==0)up=true;
			}
		}
		if(ground == 0){
			glBindTexture(GL_TEXTURE_2D, tex->image[0]);
		}else if(vel.x != 0){
			switch(texState){
				case 0:
					glBindTexture(GL_TEXTURE_2D,tex->image[1]);
				break;
				case 1:
					glBindTexture(GL_TEXTURE_2D,tex->image[0]);
				break;
				case 2:
					glBindTexture(GL_TEXTURE_2D,tex->image[2]);
				break;
			}
		}
		else{
			glBindTexture(GL_TEXTURE_2D,tex->image[0]);
		}
	}else if(type == MOBT){
		switch(subtype){
			case 1: //RABBIT
				if(ground == 0){
					glBindTexture(GL_TEXTURE_2D, texture[1]);
				}else if(ground == 1){
					glBindTexture(GL_TEXTURE_2D, texture[0]);					
				}
				break;
			default:
			break;
		}
	}/*else if(type == NPCT){
		glBindTexture(GL_TEXTURE_2D, tex->image);
	}*/else{
		glBindTexture(GL_TEXTURE_2D,texture[0]);
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
std::vector<void *> AIpreaddr;			// A dynamic array of pointers to the NPC's that are being assigned the preloads

void NPC::addAIFunc(int (*func)(NPC *),bool preload){
	if(preload){										// Preload AI functions so that they're given after 
#ifdef DEBUG											// the current dialog box is closed
		DEBUG_printf("Preloading an AI %x.\n",func);
#endif // DEBUG
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
	health = maxHealth = 1;

	/*VILLAGE*/
	if(type == STRUCTURET){
		loc.y=100;
		width =  50 * HLINE;
		height = 40 * HLINE;

		/*
		 *	tempN is the amount of entities that will be spawned in the village. As of 10/21/2015 the village
		 *	can spawn bewteen 2 and 7 villagers for the starting hut.
		*/
		int tempN = (getRand() % 5 + 2); //amount of villagers that will spawn
		for(int i=0;i<tempN;i++){
			/*
			 *				This is where the entities actually spawn.
			 *		A new entity is created with type NPC so polymorphism can be used
			*/
			entity.push_back(new NPC()); //create a new entity of NPC type
			NPCp(entity[entity.size()-1])->spawn(loc.x + (float)(i - 5),100); //sets the position of the villager around the village
		}
		return entity.size();
	}
}

/*
 * 	Mob::wander this is the function that makes the mobs wander around
 *
 *	See NPC::wander for the explaination of the arguments variables
*/
void Mob::wander(int timeRun, vec2* v){
	switch(subtype){ //SKIRL
		case 1:
			static int direction;	//variable to decide what direction the entity moves
			if(ticksToUse == 0){
				ticksToUse = timeRun;
				direction = (getRand() % 3 - 1); 	//sets the direction to either -1, 0, 1
													//this lets the entity move left, right, or stay still
				if(direction==0)ticksToUse/=2;
				v->x *= direction;	//changes the velocity based off of the direction
			}
			if(ground && direction != 0){
				v->y=.15;
				loc.y+=HLINE*.25;
				ground=false;
				v->x = (.07*HLINE)*direction;	//sets the inital velocity of the entity
			}
			ticksToUse--; //removes one off of the entities timer
		break;
		default:break;
	}
}
