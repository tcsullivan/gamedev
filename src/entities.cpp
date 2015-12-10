#include <entities.h>
#include <ui.h>

extern FILE* names;
extern unsigned int loops;

extern World *currentWorld;

extern Player *player;

extern const char *itemName;

void getRandomName(Entity *e){
	int tempNum,max=0;
	char *bufs;
	
	rewind(names);
	
	bufs = new char[16];	//(char *)malloc(16);
	
	for(;!feof(names);max++){
		fgets(bufs,16,(FILE*)names);
	}
	
	tempNum = rand() % max;
	rewind(names);
	
	for(int i=0;i<tempNum;i++){
		fgets(bufs,16,(FILE*)names);
	}
	
	switch(fgetc(names)){
	case 'm': e->gender = MALE;  break;
	case 'f': e->gender = FEMALE;break;
	default : break;
	}
	
	if((fgets(bufs,16,(FILE*)names)) != NULL){
		bufs[strlen(bufs)] = '\0';
		strcpy(e->name,bufs);
		if(e->name[strlen(e->name)-1] == '\n')
			e->name[strlen(e->name)-1] = '\0';
	}
	
	delete[] bufs;
}

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
	
	if(type==MOBT){
		if(Mobp(this)->subtype == MS_BIRD){
			Mobp(this)->init_y=loc.y;
		}
	}
	
	name = new char[16];
	getRandomName(this);
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
Player::~Player(){
	delete inv;
	delete tex;
	delete[] name;
}

NPC::NPC(){	//sets all of the NPC specific traits on object creation
	width = HLINE * 10;
	height = HLINE * 16;
	
	type	= NPCT; //sets type to npc
	subtype = 0;

	health = maxHealth = 100;
	
	maxHealth = health = 100;
	
	tex = new Texturec(1,"assets/NPC.png");
	inv = new Inventory(NPC_INV_SIZE);
	
	randDialog = rand() % 10 - 1;
}
NPC::~NPC(){
	while(!aiFunc.empty()){
		aiFunc.pop_back();
	}
	
	delete inv;
	delete tex;
	delete[] name;
}

Structures::Structures(){ //sets the structure type
	health = maxHealth = 1;
	
	alive = false;
	near  = false;
	
	tex = new Texturec(1,"assets/house1.png");
	
	inWorld = NULL;
	name = NULL;
}
Structures::~Structures(){
	delete tex;
	if(name)
		delete[] name;
}

Mob::Mob(int sub){
	type = MOBT;
	
	maxHealth = health = 50;
	
	switch((subtype = sub)){
	case MS_RABBIT:
		width  = HLINE * 10;
		height = HLINE * 8;
		tex = new Texturec(2, "assets/rabbit.png", "assets/rabbit1.png");
		break;
	case MS_BIRD:
		width = HLINE * 8;
		height = HLINE * 8;
		tex = new Texturec(1, "assets/robin.png");
	case MS_TRIGGER:
		width = HLINE * 8;
		height = 2000;
		tex = new Texturec(0);
		break;
	}
	
	inv = new Inventory(NPC_INV_SIZE);
}
Mob::~Mob(){
	delete inv;
	delete tex;
	delete[] name;
}

Object::Object(ITEM_ID id, bool qo, const char *pd){
	identifier = id;
	questObject = qo;
	
	pickupDialog = new char[strlen(pd)+1];
	strcpy(pickupDialog,pd);
	

	type = OBJECTT;
	alive = true;
	near = false;
	width  = getItemWidth(id);
	height = getItemHeight(id);

	maxHealth = health = 1;
	tex = new Texturec(1,getItemTexturePath(id));
}
Object::~Object(){
	delete[] pickupDialog;
	delete tex;
	delete[] name;
}

void Entity::draw(void){		//draws the entities
	glPushMatrix();
	glColor3ub(255,255,255);
	if(type==NPCT){
		if(NPCp(this)->aiFunc.size()){
			glColor3ub(255,255,0);
			glRectf(loc.x+width/3,loc.y+height,loc.x+width*2/3,loc.y+height+width/3);
		}
		if(gender == MALE){
			glColor3ub(255,255,255);
		}else if(gender == FEMALE){
			glColor3ub(255,105,180);
		}
	}
	if(left){
		glScalef(-1.0f,1.0f,1.0f);
		glTranslatef(0-width-loc.x*2,0,0);
	}
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	switch(type){
	case PLAYERT:
		static int texState = 0;
		static bool up = true;
		if(loops % (int)((float)4/(float)speed) == 0){
			if(up){
				if(++texState==2)up=false;
				tex->bindNext();
			}else{
				if(!--texState)up=true;
				tex->bindPrev();
			}
		}
		if(!ground){
			tex->bind(0);
		}else if(vel.x){
			tex->bind(texState);
		}else{
			tex->bind(1);
		}
		break;
	case MOBT:
		switch(subtype){
			case MS_RABBIT:
				tex->bind(!ground);
				break;
			case MS_TRIGGER:
				goto NOPE;
				break;
			case MS_BIRD:
			default:
				tex->bind(0);
				break;
		}
		break;
	default:
		tex->bind(0);
		break;
	}
	glColor3ub(255,255,255);
	glBegin(GL_QUADS);
		glTexCoord2i(0,1);glVertex2i(loc.x, loc.y);
		glTexCoord2i(1,1);glVertex2i(loc.x + width, loc.y);
		glTexCoord2i(1,0);glVertex2i(loc.x + width, loc.y + height);
		glTexCoord2i(0,0);glVertex2i(loc.x, loc.y + height);
	glEnd();
NOPE:
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	if(near)ui::putStringCentered(loc.x+width/2,loc.y-ui::fontSize-HLINE/2,name);
}

void Player::interact(){ //the function that will cause the player to search for things to interact with
	
}

/*
 *	NPC::wander, this makes the npcs wander around the near area
 *
 *	timeRun					This variable is the amount of gameloop ticks the entity will wander for
*/

void NPC::wander(int timeRun){
	
	/*
	 *	Direction is the variable that decides what direction the entity will travel in
	 *	the value is either -1, 0, or 1. -1 being left, 0 means that the npc will stay still
	 *	and a value of 1 makes the entity move to the right
	*/
	
	static int direction;
	
	/*
	 *	Ticks to use is a variable in the entity class that counts the total ticks that need to be used
	 *
	 *	This loop only runs when ticksToUse is 0, this means that the speed, direction, etc... Will be
	 *	calculated only after the npc has finished his current walking state
	*/
	
	if(ticksToUse == 0){
		ticksToUse = timeRun;
		vel.x = .008*HLINE;					//sets the inital velocity of the entity
		direction = (getRand() % 3 - 1); 	//sets the direction to either -1, 0, 1
											//this lets the entity move left, right, or stay still
		if(direction==0)ticksToUse*=2;
		vel.x *= direction;					//changes the velocity based off of the direction
	}
	ticksToUse--;							 //removes one off of the entities timer
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

const char *randomDialog[] = {
	"What a beautiful day it is.",
	"Have you ever went fast? I have.",
	"I heard if you complete a quest, you'll get a special reward.",
	"How much wood could a woodchuck chuck if a woodchuck could chuck wood?",
	"I don\'t think anyone has ever been able to climb up that hill.",
	"If you ever see a hole in the ground, watch out; it could mean the end for you.",
	"Did you know this game has over 4000 lines of code? I didn\'t. I didn't even know I was in a game until now...",
	"HELP MY CAPS LOCK IS STUCK",
	"You know, if anyone ever asked me who I wanted to be when I grow up, I would say Abby Ross.",
	"I want to have the wallpaper in our house changed. It doesn\'t really fit the environment.",
	"Frig."
};

void NPC::interact(){ //have the npc's interact back to the player
	int (*func)(NPC *);
	loc.y += 5;
	
	canMove=false;
	left = (player->loc.x < loc.x);
	right = !left;
	
	if(aiFunc.size()){
		func=aiFunc.front();
		
		if(!func(this)){
			if(aiFunc.size())aiFunc.erase(aiFunc.begin());
		}
	}else{
		ui::dialogBox(name,NULL,randomDialog[randDialog]);
	}
	ui::waitForDialog();
	canMove=true;
}

void Object::interact(void){
	if(questObject && alive){
		ui::dialogBox(player->name,":Yes:No",pickupDialog);		
		ui::waitForDialog();
		if(ui::dialogOptChosen == 1){
			player->inv->addItem((ITEM_ID)(identifier), (char)1);
			alive = false;
		}
	}else{
		alive = false;
		player->inv->addItem((ITEM_ID)(identifier), (char)1);
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

unsigned int Structures::spawn(_TYPE t, float x, float y){
	loc.x = x;
	loc.y = y;
	type = t;
	
	alive = true;

	width =  50 * HLINE;
	height = 40 * HLINE;

	/*
	 *	tempN is the amount of entities that will be spawned in the village. Currently the village
	 *	will spawn bewteen 2 and 7 villagers for the starting hut.
	*/
	
	unsigned int tempN = (getRand() % 5 + 2);
	
	for(unsigned int i = 0;i < tempN;i++){
		
		/*
		 *	This is where the entities actually spawn. A new entity is created
		 *	with type NPC.
		*/
		
		currentWorld->addNPC(loc.x + i * HLINE ,100);
		
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
	static unsigned int heya=0,hi=0;
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
		if(loc.y<=init_y-.2)vel.y=.02*deltaTime;	// TODO handle direction
		vel.x=.02*deltaTime;
		if(++heya==200){heya=0;hi^=1;}
		if(hi)vel.x*=-1;
		break;
	case MS_TRIGGER:
		if(player->loc.x + player->width / 2 > loc.x		 &&
		   player->loc.x + player->width / 2 < loc.x + width ){
			if(player->left)player->loc.x = loc.x + width;
			else if(player->right) player->loc.x = loc.x - player->width;
			hey(this);
		}
		break;
	default:
		break;
	}
}
