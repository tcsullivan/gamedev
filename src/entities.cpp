#include <entities.h>
#include <ui.h>

extern std::vector<Entity*>entity;
extern std::vector<NPC>npc;
extern std::vector<Structures>build;

void Entity::spawn(float x, float y){	//spawns the entity you pass to it based off of coords and global entity settings
	loc.x = x;
	loc.y = y;
	vel.x = 0;
	vel.y = 0;
	right = false;
	left = false;
	near = false;
	ticksToUse = 0;
	canMove = false;
	ground = false;
	name = (char*)malloc(16);
	getName();
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
		}if(ground == 0){
			glBindTexture(GL_TEXTURE_2D, texture[1]);
		}
		else if(vel.x != 0){
			switch(texState){
				case 0:
					glBindTexture(GL_TEXTURE_2D,texture[1]);
				break;
				case 1:
					glBindTexture(GL_TEXTURE_2D,texture[0]);
				break;
				case 2:
					glBindTexture(GL_TEXTURE_2D,texture[2]);
				break;
			}
		}else{
			glBindTexture(GL_TEXTURE_2D,texture[0]);
		}
	}else{
		glBindTexture(GL_TEXTURE_2D,texture[0]);
	}
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

Player::Player(){ //sets all of the player specific traits on object creation
	width = HLINE * 10;
	height = HLINE * 16;
	speed = 1;
	type = PLAYERT; //set type to player
	subtype = 5;
	alive = true;
	ground = false;
	near = true;
	texture[0] = loadTexture("assets/player.png");
	texture[1] = loadTexture("assets/player1.png");
	texture[2] = loadTexture("assets/player2.png");
	inv = new Inventory(PLAYER_INV_SIZE);
}

void Player::interact(){ //the function that will cause the player to search for things to interact with
	
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
	texture[0] = loadTexture("assets/NPC.png");
	texture[1] = 0;
	texture[2] = 0;
	inv = new Inventory(NPC_INV_SIZE);
}

void NPC::wander(int timeRun, vec2 *v){ //this makes the entites wander about
	static int direction;	//variable to decide what direction the entity moves
	if(ticksToUse == 0){
		ticksToUse = timeRun;
		v->x = .008*HLINE;	//sets the inital velocity of the entity
		direction = (getRand() % 3 - 1); 	//sets the direction to either -1, 0, 1
											//this lets the entity move left, right, or stay still
		v->x *= direction;	//changes the velocity based off of the direction
	}
	ticksToUse--; //removes one off of the entities timer
}


void NPC::addAIFunc(int (*func)(NPC *)){
	aiFunc.push_back(func);
}

void NPC::interact(){ //have the npc's interact back to the player
	int (*func)(NPC *);
	loc.y += 5;
	if(aiFunc.size()){
		func=aiFunc.front();
		if(!func(this)){
			aiFunc.erase(aiFunc.begin());
		}
	}
}

Structures::Structures(){ //sets the structure type
	type = STRUCTURET;
	speed = 0;
	alive = true;
	near = false;
	texture[0] = loadTexture("assets/house1.png");
	texture[1] = 0;
	texture[2] = 0;
}

unsigned int Structures::spawn(_TYPE t, float x, float y){ //spawns a structure based off of type and coords
	loc.x = x;
	loc.y = y;
	type = t;

	/*VILLAGE*/
	//spawns a village
	//spawns between 1 and 5 villagers around the village
	if(type == STRUCTURET){
		loc.y=100;
		width =  50 * HLINE;
		height = 40 * HLINE;

		int tempN = (getRand() % 5 + 2); //amount of villagers that will spawn
		for(int i=0;i<tempN;i++){
			entity.push_back(new NPC()); //create a new entity of NPC type
			npc.push_back(NPC()); //create new NPC
			entity[entity.size()] = &npc[npc.size()-1]; //set the new entity to have the same traits as an NPC
			entity[entity.size()-1]->spawn(loc.x + (float)(i - 5),100); //sets the position of the villager around the village
		}
		return entity.size();
	}
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
	texture[0] = loadTexture("assets/NPC.png");
	texture[1] = 0;
	texture[2] = 0;
	inv = new Inventory(NPC_INV_SIZE);
}

void Mob::wander(int timeRun, vec2* v){
	if(subtype == 1){ //SKIRL
		static int direction;	//variable to decide what direction the entity moves
		if(ticksToUse == 0){
			ticksToUse = timeRun;
			if(ground && direction != 0){
				v->y=.08;
				loc.y+=HLINE*1;
				ground=false;
				v->x = .008*HLINE;	//sets the inital velocity of the entity
			}
			direction = (getRand() % 3 - 1); 	//sets the direction to either -1, 0, 1
												//this lets the entity move left, right, or stay still
			v->x *= direction;	//changes the velocity based off of the direction
		}
		ticksToUse--; //removes one off of the entities timer
	}
}