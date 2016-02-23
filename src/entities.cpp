#include <entities.h>
#include <ui.h>

#include <istream>

#define RAND_DIALOG_COUNT 13

extern std::istream *names;
extern unsigned int loops;

extern World *currentWorld;

extern Player *player;

extern const char *itemName;

GLuint waterTex;

void initEntity(){
	waterTex = Texture::loadTexture("assets/waterTex.png");
}

void getRandomName(Entity *e){
	unsigned int tempNum,max=0;
	char *bufs;
	
	std::ifstream names ("assets/names_en-us",std::ios::in);
	
	names.seekg(0,names.beg);
	
	bufs = new char[32];
	
	for(;!names.eof();max++)
		names.getline(bufs,32);
	
	tempNum = rand() % max;
	names.seekg(0,names.beg);

	for(unsigned int i=0;i<tempNum;i++)
		names.getline(bufs,32);
	
	names.close();
	
	switch(bufs[0]){
	default :
	case 'm': e->gender = MALE;  break;
	case 'f': e->gender = FEMALE;break;
	}
	
	strcpy(e->name,bufs+1);
	
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
	//canMove	= true;
	ground	= false;
	hit 	= false;
	
	ticksToUse = 0;
	
	if(!maxHealth)health = maxHealth = 1;
	
	if(type==MOBT){
		if(Mobp(this)->subtype == MS_BIRD){
			Mobp(this)->init_y=loc.y;
		}
	}
	
	name = new char[32];
	getRandomName(this);
}

Player::Player(){ //sets all of the player specific traits on object creation
	width = HLINE * 10;
	height = HLINE * 15;
	
	type = PLAYERT; //set type to player
	subtype = 0;	
	health = maxHealth = 100;
	speed = 1;
	canMove = true;

	tex = new Texturec(9, 	"assets/player/playerk.png",
							"assets/player/playerk1.png",
							"assets/player/playerk2.png",
							"assets/player/playerk3.png",
							"assets/player/playerk4.png",
							"assets/player/playerk5.png",
							"assets/player/playerk6.png",
							"assets/player/playerk7.png",
							"assets/player/playerk8.png");
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
	canMove = true;
	
	tex = new Texturec(1,"assets/NPC.png");
	inv = new Inventory(NPC_INV_SIZE);
	
	randDialog = rand() % RAND_DIALOG_COUNT - 1;
	dialogIndex = 0;
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
	health = maxHealth = 25;
	
	alive = false;
	near  = false;
	
	name = NULL;
	
	inv = NULL;
	canMove = false;
}
Structures::~Structures(){
	delete tex;
	if(name)
		delete[] name;
	if(inside)
		delete[] inside;
}

Mob::Mob(int sub){
	type = MOBT;
	aggressive = false;
	
	maxHealth = health = 50;
	canMove = true;
	
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
		break;
	case MS_TRIGGER:
		width = HLINE * 20;
		height = 2000;
		tex = new Texturec(0);
		break;
	case MS_DOOR:
		width = HLINE * 12;
		height = HLINE * 20;
		tex = new Texturec(1,"assets/door.png");
		break;
	case MS_PAGE:
		width = HLINE * 6;
		height = HLINE * 4;
		tex = new Texturec(1,"assets/items/ITEM_PAGE.png");
		break;
	}
	
	inv = new Inventory(NPC_INV_SIZE);
}
Mob::~Mob(){
	delete inv;
	delete tex;
	delete[] name;
}

Object::Object(){
	type = OBJECTT;
	alive = true;
	near = false;
	width  = 0;
	height = 0;
	canMove = false;

	maxHealth = health = 1;
	
	tex = NULL;
	inv = NULL;
}

Object::Object(std::string in, const char *pd){
	iname = in;

	if(pd){
		pickupDialog = new char[strlen(pd)+1];
		strcpy(pickupDialog,pd);
		questObject = true;
	}else{
		pickupDialog = new char[1];
		*pickupDialog = '\0';
		questObject = false;
	}

	type = OBJECTT;
	alive = true;
	near = false;
	width  = getItemWidth(in);
	height = getItemHeight(in);

	maxHealth = health = 1;
	tex = new Texturec(1,getItemTexturePath(in));
	inv = NULL;
}
Object::~Object(){
	delete[] pickupDialog;
	delete tex;
	delete[] name;
}

void Object::reloadTexture(void){
	if(tex)
		delete tex;
		
	tex = new Texturec(1,getItemTexturePath(iname));
	width  = getItemWidth(iname);
	height = getItemHeight(iname);
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
		if(speed && !(loops % ((2.0f/speed) < 1 ? 1 : (int)((float)2.0f/(float)speed)))){
			if(++texState==9)texState=1;
			glActiveTexture(GL_TEXTURE0);
			tex->bind(texState);
		}
		if(!ground){
			glActiveTexture(GL_TEXTURE0 + 0);
			tex->bind(0);
		}else if(vel.x){
			glActiveTexture(GL_TEXTURE0 + 0);
			tex->bind(texState);
		}else{
			glActiveTexture(GL_TEXTURE0 + 0);
			tex->bind(0);
		}
		break;
	case MOBT:
		switch(subtype){
			case MS_RABBIT:
				glActiveTexture(GL_TEXTURE0 + 0);
				tex->bind(!ground);
				break;
			case MS_TRIGGER:
				goto NOPE;
				break;
			case MS_BIRD:
			case MS_DOOR:
			case MS_PAGE:
			default:
				glActiveTexture(GL_TEXTURE0);
				tex->bind(0);
				break;
		}
		break;
	case STRUCTURET:
		for(auto &strt : currentWorld->build){
			if(this == strt){
				glActiveTexture(GL_TEXTURE0);
				tex->bind(0);
				break;
			}
		} 
		break;
	default:
		glActiveTexture(GL_TEXTURE0);
		tex->bind(0);
		break;
	}
	glColor3ub(255,255,255);
	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "sampler"), 0);
	glBegin(GL_QUADS);
		glTexCoord2i(0,1);glVertex2i(loc.x, loc.y);
		glTexCoord2i(1,1);glVertex2i(loc.x + width, loc.y);
		glTexCoord2i(1,0);glVertex2i(loc.x + width, loc.y + height);
		glTexCoord2i(0,0);glVertex2i(loc.x, loc.y + height);
	glEnd();
	glUseProgram(0);
NOPE:
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	if(near)ui::putStringCentered(loc.x+width/2,loc.y-ui::fontSize-HLINE/2,name);
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

void NPC::clearAIFunc(void){
	aiFunc.clear();
}

const char *randomDialog[RAND_DIALOG_COUNT] = {
	"What a beautiful day it is.",
	"Have you ever went fast? I have.",
	"I heard if you complete a quest, you'll get a special reward.",
	"How much wood could a woodchuck chuck if a woodchuck could chuck wood?",
	"I don\'t think anyone has ever been able to climb up that hill.",
	"If you ever see a hole in the ground, watch out; it could mean the end for you.",
	"Did you know this game has over 5000 lines of code? I didn\'t. I didn't even know I was in a game until now...",
	"HELP MY CAPS LOCK IS STUCK",
	"You know, if anyone ever asked me who I wanted to be when I grow up, I would say Abby Ross.",
	"I want to have the wallpaper in our house changed. It doesn\'t really fit the environment.",
	"Frig.",
	"The sine of theta equals the opposite over the hypotenuese.",
	"Did you know the developers spelt brazier as brazzier."
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
		ui::dialogBox(name,NULL,false,randomDialog[randDialog]);
	}
	ui::waitForDialog();
	canMove=true;
}

void Object::interact(void){
	if(questObject && alive){
		ui::dialogBox(player->name,":Yes:No",false,pickupDialog);		
		ui::waitForDialog();
		if(ui::dialogOptChosen == 1){
			player->inv->addItem(/*(ITEM_ID)(identifier)*/iname, 1);
			alive = false;
		}
	}else{
		alive = false;
		player->inv->addItem(iname, 1);
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

unsigned int Structures::spawn(BUILD_SUB sub, float x, float y){
	loc.x = x;
	loc.y = y;
	type = STRUCTURET;

	alive = true;

	bsubtype = sub;
	dim2 dim;
	
	/*
	 *	tempN is the amount of entities that will be spawned in the village. Currently the village
	 *	will spawn bewteen 2 and 7 villagers for the starting hut.
	*/

	//unsigned int tempN = (getRand() % 5 + 2);
	switch(sub){
		default:
			tex = new Texturec(1, textureLoc ? textureLoc : inWorld->sTexLoc[sub].c_str());
			dim = Texture::imageDim(textureLoc ? textureLoc : inWorld->sTexLoc[sub].c_str());
			width = dim.x;
			height = dim.y;
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
	static unsigned int heya=0,hi=0;
	static bool YAYA = false;
	
	if(aggressive && !YAYA &&
	   player->loc.x + (width / 2)  > loc.x && player->loc.x + (width / 2)  < loc.x + width  &&
	   player->loc.y + (height / 3) > loc.y && player->loc.y + (height / 3) < loc.y + height ){
		Arena *a = new Arena(currentWorld,player,this);
		a->setBackground(BG_FOREST);
		a->setBGM("assets/music/embark.wav");
		ui::toggleWhiteFast();
		YAYA = true;
		ui::waitForCover();
		YAYA = false;
		currentWorld = a;
		ui::toggleWhiteFast();
	}
	
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
		   player->loc.x + player->width / 2 < loc.x + width )
			hey(this);
		break;
	case MS_PAGE:
		if(player->loc.x > loc.x - 100		 &&
		   player->loc.x < loc.x + 100		 &&
		   ui::mouse.x > loc.x				 &&
		   ui::mouse.x < loc.x + width		 &&
		   ui::mouse.y > loc.y - width / 2	 &&
		   ui::mouse.y < loc.y + width * 1.5 &&
		   SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)){
			if(speed != 666){
				speed = 666;
				hey(this);
				speed = 0;
			}
		}
		break;
	default:
		break;
	}
}

void Player::save(void){
	std::string data;
	std::ofstream out ("xml/main.dat",std::ios::out | std::ios::binary);
	std::cout<<"Saving player data..."<<std::endl;
	data.append(std::to_string((int)loc.x) + "\n");
	data.append(std::to_string((int)loc.y) + "\n");
	data.append(std::to_string((int)health) + "\n");
	data.append(std::to_string((int)maxHealth) + "\n");
	
	data.append(std::to_string((int)inv->items.size()) + "\n");
	for(auto &i : inv->items)
		data.append(std::to_string((int)i.count) + "\n" + std::to_string((int)i.id) + "\n");
	
	data.append((std::string)(currentXML+4) + "\n");
	
	data.append("dOnE\0");
	out.write(data.c_str(),data.size());
	out.close();	
}

void Player::sspawn(float x,float y){
	unsigned int i;
	uint count;
	std::ifstream in ("xml/main.dat",std::ios::in | std::ios::binary);
	spawn(x,y);
	
	if(in.good()){
		std::istringstream data;
		std::string ddata;
		std::streampos len;
		
		in.seekg(0,std::ios::end);
		len = in.tellg();
		in.seekg(0,std::ios::beg);
		
		std::vector<char> buf (len,'\0');
		in.read(buf.data(),buf.size());
		
		data.rdbuf()->pubsetbuf(buf.data(),buf.size());
		
		std::getline(data,ddata);
		loc.x = std::stoi(ddata);
		std::getline(data,ddata);
		loc.y = std::stoi(ddata);
		std::getline(data,ddata);
		health = std::stoi(ddata);
		std::getline(data,ddata);
		maxHealth = std::stoi(ddata);
	
		std::getline(data,ddata);
		for(i = std::stoi(ddata);i;i--){
			std::getline(data,ddata);
			count = std::stoi(ddata);
			std::getline(data,ddata);
			inv->items.push_back((item_t){count,(uint)std::stoi(ddata)});
		}
		
		std::getline(data,ddata);
		currentWorld = loadWorldFromXMLNoSave(ddata.c_str());
		
		in.close();
	}
}
