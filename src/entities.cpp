#include <entities.hpp>

#include <istream>
#include <sstream>
#include <fstream>

#include <ui.hpp>
#include <world.hpp>
#include <gametime.hpp>

extern std::istream *names;

extern Player *player;			// main.cpp
extern World *currentWorld;		// main.cpp
extern unsigned int loops;		// main.cpp

extern std::string xmlFolder;

// a dynamic array of pointers to the NPC's that are being assigned the preloads
std::vector<NPC *> aipreload;

// the size of the player's inventory
const unsigned int PLAYER_INV_SIZE = 43;
// the size of an NPC's inventory
const unsigned int NPC_INV_SIZE = 3;

static const unsigned int RAND_DIALOG_COUNT = 14;
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
	"The sine of theta equals the opposite over the hdaypotenuese.",
	"Did you know the developers spelt brazier as brazzier.",
	"What's a bagel? I don't know because I'm mormon"
};

void randGetomName(Entity *e)
{
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

	// gender is a binary construct
	e->gender = (bufs[0] == 'm') ? MALE : FEMALE;

	strcpy(e->name, bufs + 1);

	delete[] bufs;
}

Entity::Entity(void)
{
	vel = 0;
	width = 0;
	height = 0;
	health = 0;
	maxHealth = 0;
	outnabout = 0;
	z = 1.0f;
	targetx = 0.9112001f;

	type = UNKNOWNT;

	// set flags
	alive      = true;
	right      = true;
	left       = false;
	near       = false;
	canMove    = true;
	ground     = false;
	forcedMove = false;
	z = -1.0f;

	// clear counters
	ticksToUse = 0;
	hitCooldown = 0;

	hitDuration = maxHitDuration = 0;

	inv = nullptr;
	name = nullptr;
}

// spawns the entity you pass to it based off of coords and global entity settings
void Entity::spawn(float x, float y)
{
	loc.x = x;
	loc.y = y;

	if (health == 0 && maxHealth == 0)
		health = maxHealth = 1;

	// generate a name
	name = new char[32];
	if (type == MOBT)
		name[0] = '\0';
	else
		randGetomName(this);

	setCooldown(0);
}

void Entity::takeHit(unsigned int _health, unsigned int cooldown)
{
	if (hitCooldown <= 1) {
		std::cout << "Taking hit " << std::endl;
		// modify variables
		health = fmax(health - _health, 0);
		forcedMove = true;
		hitCooldown = cooldown;

		hitDuration = maxHitDuration = 350.0f;

		// pushback
		vel.x = player->left ? -0.5f : 0.5f;
		vel.y = 0.2f;
	}
}

unsigned int Entity::coolDown()
{
	return hitCooldown;
}

void Entity::setCooldown(unsigned int c)
{
	hitCooldown = c;
}

void Entity::handleHits(void)
{
	hitCooldown = fmax(static_cast<int>(hitCooldown - game::time::getDeltaTime()), 0);
	hitDuration = fmax(hitDuration - game::time::getDeltaTime(), 0);

	if (!forcedMove)
		return;

	// reduce knockback
	if ((vel.x > 0.0005f) || (vel.x < -0.0005f))
		vel.x *= 0.6f;
	else
		forcedMove = false;
}

void Entity::die(void)
{
	alive = false;
	health = 0;
}

bool Entity::isAlive(void) const
{
	return alive;
}

bool Entity::isHit(void) const
{
	return forcedMove;
}

void Entity::moveTo(float dest_x)
{
	targetx = dest_x;
}

Player::Player() : Entity()
{
	width = HLINES(10);
	height = HLINES(16);

	type = PLAYERT; //set type to player
	subtype = 0;
	health = maxHealth = 100;
	speed = 1;
	canMove = true;

	tex = TextureIterator({"assets/player/playerk.png",
						   "assets/player/playerk1.png",
						   "assets/player/playerk2.png",
						   "assets/player/playerk3.png",
						   "assets/player/playerk4.png",
						   "assets/player/playerk5.png",
						   "assets/player/playerk6.png",
						   "assets/player/playerk7.png",
						   "assets/player/playerk8.png"
					       });

	inv = new Inventory(PLAYER_INV_SIZE);
	dim2 tmpDim = Texture::imageDim(tex.getTexturePath(0));
	width = HLINES(tmpDim.x/2);
	height = HLINES(tmpDim.y/2);
}

Player::~Player()
{
	delete inv;
	delete[] name;
}

NPC::NPC() : Entity()
{
	width = HLINES(10);
	height = HLINES(16);

	type	= NPCT; //sets type to npc
	subtype = 0;

	health = maxHealth = 100;

	maxHealth = health = 100;
	canMove = true;

	tex = TextureIterator({"assets/NPC.png"});
	inv = new Inventory(NPC_INV_SIZE);

	randDialog = rand() % RAND_DIALOG_COUNT - 1;
	dialogIndex = 0;
	dialogCount = 0;

	dim2 tmpDim = Texture::imageDim(tex.getTexturePath(0));
	width = HLINES(tmpDim.x/2);
	height = HLINES(tmpDim.y/2);
}

NPC::~NPC()
{
	delete inv;
	delete[] name;
}

Merchant::Merchant() : NPC()
{
	width = HLINES(10);
	height = HLINES(16);

	type	= MERCHT; //sets type to merchant
	subtype = 0;

	health = maxHealth = 100;

	canMove = true;

	trade.reserve(100);
	currTrade = 0;

	inside = nullptr;

	//tex = new Texturec(1,"assets/NPC.png");
	//inv = new Inventory(NPC_INV_SIZE);
	//inv = new Inventory(1);

	//randDialog = rand() % RAND_DIALOG_COUNT - 1;
	dialogIndex = 0;

	dim2 tmpDim = Texture::imageDim(tex.getTexturePath(0));
	width = HLINES(tmpDim.x/2);
	height = HLINES(tmpDim.y/2);
}

Merchant::~Merchant()
{
	//delete inv;
	delete[] name;
}

Structures::Structures() : Entity()
{
	canMove = false;
	health = maxHealth = 1;
}

Structures::~Structures()
{
	delete[] name;
}

Object::Object()
{
	type = OBJECTT;
}

Object::Object(std::string in, std::string pd)
{
	iname = in;

	pickupDialog = pd;
	questObject = !pd.empty();

	type = OBJECTT;
	width  = getItemWidth(in);
	height = getItemHeight(in);

	tex = TextureIterator({getItemTexturePath(in)});
}

Object::~Object()
{
	delete[] name;
}

void Object::reloadTexture(void)
{
	tex = TextureIterator({getItemTexturePath(iname)});
	width  = getItemWidth(iname);
	height = getItemHeight(iname);
}

bool Entity::isNear(Entity e) {
	return pow(e.loc.x - loc.x, 2) + pow(e.loc.y - loc.y, 2) <= pow(HLINES(40), 2);
}

void NPC::drawThingy(void) const
{
	if (dialogCount) {
		const auto w = width / 3;
		GLfloat tex_coord[] = {
			0, 0, 1, 0, 1, 1,
			1, 1, 0, 1, 0, 0
		};
		const GLfloat c[4] = {
			loc.x + w, loc.y + height, loc.x + w * 2, loc.y + height + w
		};
		GLfloat coords[] = {
			c[0], c[1], z, c[2], c[1], z, c[2], c[3], z,
			c[2], c[3], z, c[0], c[3], z, c[0], c[1], z
		};
		
		glUseProgram(worldShader);
		glEnableVertexAttribArray(worldShader_attribute_coord);
		glEnableVertexAttribArray(worldShader_attribute_tex);
		glVertexAttribPointer(worldShader_attribute_coord, 3, GL_FLOAT, GL_FALSE, 0, coords);
		glVertexAttribPointer(worldShader_attribute_tex, 2, GL_FLOAT, GL_FALSE, 0, tex_coord);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(worldShader_attribute_coord);
		glDisableVertexAttribArray(worldShader_attribute_tex);
		glUseProgram(0);
	}
}

void Entity::draw(void)
{
	GLfloat tex_coord[] = {0.0, 0.0,
						   1.0, 0.0,
						   1.0, 1.0,

						   1.0, 1.0,
						   0.0, 1.0,
						   0.0, 0.0};

	GLfloat tex_coordL[] = {1.0, 0.0,
						   	0.0, 0.0,
						   	0.0, 1.0,

						   	0.0, 1.0,
						   	1.0, 1.0,
						   	1.0, 0.0};

	GLfloat coords[] = {loc.x, loc.y, z,
						loc.x + width, loc.y, z,
						loc.x + width, loc.y + height, z,

						loc.x + width, loc.y + height, z,
						loc.x, loc.y + height, z,
						loc.x, loc.y, z};


	glActiveTexture(GL_TEXTURE0);

	if (!alive)
		return;

	if (type == NPCT) {
		NPCp(this)->drawThingy();

		if (gender == MALE)
			glColor3ub(255, 255, 255);
		else if (gender == FEMALE)
			glColor3ub(255, 105, 180);
	} else if (type == MOBT) {
		if (Mobp(this)->rider != nullptr) {
			Mobp(this)->rider->loc.x = loc.x + width * 0.25f;
	        Mobp(this)->rider->loc.y = loc.y + height - game::HLINE;
	        Mobp(this)->rider->vel.y = .12;
	    }
	}
	switch(type) {
	case PLAYERT:
		static int texState = 0;
		if (speed && !(game::time::getTickCount() % ((2.0f/speed) < 1 ? 1 : (int)((float)2.0f/(float)speed)))) {
			if (++texState == 9)
				texState = 1;
			glActiveTexture(GL_TEXTURE0);
			tex(texState);
		}
		if (!ground) {
			glActiveTexture(GL_TEXTURE0);
			tex(0);
		} else if (vel.x) {
			glActiveTexture(GL_TEXTURE0);
			tex(texState);
		} else {
			glActiveTexture(GL_TEXTURE0);
			tex(0);
		}
		break;
	case MOBT:
		if (!Mobp(this)->bindTex())
			goto NOPE;
		break;
	case STRUCTURET:
		/* fall through */
	default:
		glActiveTexture(GL_TEXTURE0);
		tex(0);
		break;
	}

	//TODO
	/*if (hitCooldown)
		glColor3ub(255,255,0);
	else
		glColor3ub(255,255,255);*/

	glUseProgram(worldShader);
	// make the entity hit flash red
	if (maxHitDuration-hitDuration) {
		float flashAmt = 1-(hitDuration/maxHitDuration);
		glUniform4f(worldShader_uniform_color, 1.0, flashAmt, flashAmt, 1.0);
	}

	glUniform1i(worldShader_uniform_texture, 0);
	glEnableVertexAttribArray(worldShader_attribute_coord);
	glEnableVertexAttribArray(worldShader_attribute_tex);

	glVertexAttribPointer(worldShader_attribute_coord, 3, GL_FLOAT, GL_FALSE, 0, coords);
	if (left)
		glVertexAttribPointer(worldShader_attribute_tex, 2, GL_FLOAT, GL_FALSE, 0 ,tex_coordL);
	else
		glVertexAttribPointer(worldShader_attribute_tex, 2, GL_FLOAT, GL_FALSE, 0 ,tex_coord);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUniform4f(worldShader_uniform_color, 1.0, 1.0, 1.0, 1.0);
NOPE:
if (near && type != MOBT)
	ui::putStringCentered(loc.x+width/2,loc.y-ui::fontSize-game::HLINE/2,name);
if (health != maxHealth) {

	static GLuint frontH = Texture::genColor(Color(255,0,0));
	static GLuint backH =  Texture::genColor(Color(150,0,0));
	glUniform1i(worldShader_uniform_texture, 0);

	GLfloat coord_back[] = {
		loc.x, 			loc.y + height, 			      z,
		loc.x + width,	loc.y + height, 			      z,
		loc.x + width, 	loc.y + height + game::HLINE * 2, z,

		loc.x + width, 	loc.y + height + game::HLINE * 2, z,
		loc.x, 			loc.y + height + game::HLINE * 2, z,
		loc.x, 			loc.y + height, 			      z,
	};

	GLfloat coord_front[] = {
		loc.x, 			                    loc.y + height,                   z,
		loc.x + health / maxHealth * width, loc.y + height,                   z,
		loc.x + health / maxHealth * width, loc.y + height + game::HLINE * 2, z,

		loc.x + health / maxHealth * width, loc.y + height + game::HLINE * 2, z,
		loc.x,                              loc.y + height + game::HLINE * 2, z,
		loc.x,                              loc.y + height,                   z,
	};

	glBindTexture(GL_TEXTURE_2D, backH);
	GLfloat tex[] = { 0.0, 0.0,
						   1.0, 0.0,
						   1.0, 1.0,

						   1.0, 1.0,
						   0.0, 1.0,
						   0.0, 0.0,
	};
	glVertexAttribPointer(worldShader_attribute_coord, 3, GL_FLOAT, GL_FALSE, 0, coord_back);
	glVertexAttribPointer(worldShader_attribute_tex, 2, GL_FLOAT, GL_FALSE, 0, tex);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, frontH);
	glVertexAttribPointer(worldShader_attribute_coord, 3, GL_FLOAT, GL_FALSE, 0, coord_front);
	glVertexAttribPointer(worldShader_attribute_tex, 2, GL_FLOAT, GL_FALSE, 0, tex);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

glDisableVertexAttribArray(worldShader_attribute_coord);
glDisableVertexAttribArray(worldShader_attribute_tex);

glUseProgram(0);
}

/**
 * Handles NPC movement, usually just random walking.
 */

void NPC::
wander(int timeRun)
{
	static int direction;

	if (forcedMove)
		return;

	if (hitCooldown)
		hitCooldown--;

	if (targetx != 0.9112001f) {
		if (loc.x > targetx + HLINES(5))
			vel.x = HLINES(-0.018);
		else if (loc.x < targetx - HLINES(5))
			vel.x = HLINES(0.018);
		else
			targetx = 0.9112001f;
	} else if (ticksToUse == 0) {
		ticksToUse = timeRun;

		vel.x = HLINES(0.008);
		direction = (randGet() % 3 - 1);

		if (direction == 0)
			ticksToUse *= 2;

		vel.x *= direction;
	}

	if (vel.x < 0)
		currentWorld->goWorldLeft(this);

	ticksToUse--;
}

void NPC::addAIFunc(bool preload)
{
	if (preload)
		aipreload.push_back(this);
	else
		++dialogCount;
}

extern int commonAIFunc(NPC *speaker);

void NPC::interact() { //have the npc's interact back to the player
	std::thread([this]{
		std::vector<XMLElement *> dopt;
		XMLDocument xml;
		XMLElement *exml,*oxml;

		static unsigned int oldidx = 9999;
		std::string nname;
		unsigned int idx;
		bool stop;

		loc.y += 5;

		canMove=false;
		left = (player->loc.x < loc.x);
		right = !left;

		if (dialogCount && dialogIndex != 9999) {
			// load the XML file and find the dialog tags
			if (outnabout == 0)
				xml.LoadFile(currentXML.c_str());
			else if (outnabout < 0)
				xml.LoadFile((xmlFolder + currentWorld->getToLeft()).c_str());
			else
				xml.LoadFile((xmlFolder + currentWorld->getToRight()).c_str());
COMMONAIFUNC:
			idx = 0;
			stop = false;
			exml = xml.FirstChildElement("Dialog");

			// search for the dialog block associated with this npc
			while (exml->StrAttribute("name") != name)
				exml = exml->NextSiblingElement();

			// search for the desired text block
			exml = exml->FirstChildElement();
			do {
				if (std::string("text") == exml->Name() && exml->UnsignedAttribute("id") == (unsigned)dialogIndex)
					break;
			} while ((exml = exml->NextSiblingElement()));

			// handle quest tags
			if ((oxml = exml->FirstChildElement("quest"))) {
				std::string qname;

				// iterate through all quest tags
				do {
					// assign quest
					if (!(qname = oxml->StrAttribute("assign")).empty())
						player->qh.assign(qname, "None", std::string(oxml->GetText())); // TODO add descriptions

					// check / finish quest
					else if (!(qname = oxml->StrAttribute("check")).empty()) {
						if (player->qh.hasQuest(qname) && player->qh.finish(qname)) {
							// QuestHandler::finish() did all the work..
							break;
						} else {
							// run error dialog
							oldidx = dialogIndex;
							dialogIndex = oxml->UnsignedAttribute("fail");
							goto COMMONAIFUNC;
						}
					}
				} while((oxml = oxml->NextSiblingElement()));
			}

			// handle give tags
			if ((oxml = exml->FirstChildElement("give"))) {
				do player->inv->addItem(oxml->Attribute("id"), oxml->UnsignedAttribute("count"));
				while ((oxml = oxml->NextSiblingElement()));
			}

			// handle take tags
			if ((oxml = exml->FirstChildElement("take"))) {
				do player->inv->takeItem(oxml->Attribute("id"), oxml->UnsignedAttribute("count"));
				while ((oxml = oxml->NextSiblingElement()));
			}

			// handle movement directs
			if ((oxml = exml->FirstChildElement("gotox")))
				moveTo(std::stoi(oxml->GetText()));

			// asdlfkj
			auto ptr = exml->GetText() - 1;
			while (isspace(*++ptr));

			// handle dialog options
			if ((oxml = exml->FirstChildElement("option"))) {
				std::string optstr;

				// convert option strings to a colon-separated format
				do {
					// append the next option
					optstr.append(std::string(":") + oxml->Attribute("text"));

					// save the associated XMLElement
					dopt.push_back(oxml);
				} while ((oxml = oxml->NextSiblingElement()));

				// run the dialog stuff
				ui::dialogBox(name, optstr, false, ptr);
				ui::waitForDialog();

				if (ui::dialogOptChosen)
					exml = dopt[ui::dialogOptChosen - 1];

				dopt.clear();
			}

			// optionless dialog
			else {
				ui::dialogBox(name, "", false, ptr);
				ui::waitForDialog();
			}

			// trigger other npcs if desired
			if (!(nname = exml->StrAttribute("call")).empty()) {
				NPC *n = *std::find_if(std::begin(currentWorld->npc), std::end(currentWorld->npc), [nname](NPC *npc) {
					return (npc->name == nname);
				});

				if (exml->QueryUnsignedAttribute("callid", &idx) == XML_NO_ERROR) {
					n->dialogIndex = idx;
					n->addAIFunc(false);
				}
			}

			// handle potential following dialogs
			if ((idx = exml->UnsignedAttribute("nextid"))) {
				dialogIndex = idx;

				// stop talking
				if (exml->QueryBoolAttribute("stop", &stop) == XML_NO_ERROR && stop) {
					dialogIndex = 9999;
					dialogCount--;
				}

				// pause, allow player to click npc to continue
				else if (exml->QueryBoolAttribute("pause", &stop) == XML_NO_ERROR && stop) {
					//return 1;
				}

				// instantly continue
				else {
					goto COMMONAIFUNC;
				}
			}

			// stop talking
			else {
				// error text?
				if (oldidx != 9999) {
					dialogIndex = oldidx;
					oldidx = 9999;
				} else {
					dialogIndex = 9999;
					dialogCount--;
				}
			}
		} else {
			ui::dialogBox(name, "", false, randomDialog[randDialog]);
		}

		ui::waitForDialog();
		canMove = true;
	}).detach();
}

void Merchant::wander(int timeRun) {
	static int direction;

	if (forcedMove)
		return;

	if (ticksToUse == 0) {
		ticksToUse = timeRun;

		vel.x = HLINES(0.008);
		direction = (randGet() % 3 - 1);

		if (direction == 0)
			ticksToUse *= 2;

		vel.x *= direction;
	}

	if (vel.x < 0)
		currentWorld->goWorldLeft(this);
	if (inside != nullptr) {
		loc.y = inside->loc.y + HLINES(2);
		vel.y = GRAVITY_CONSTANT * 5;
		if (loc.x <= inside->loc.x + HLINES(5))
			loc.x = inside->loc.x + HLINES(5);
		else if (loc.x + width >= inside->loc.x + inside->width - HLINES(5))
			loc.x = inside->loc.x + inside->width - width - HLINES(5);
	}
	ticksToUse--;
}

void Merchant::interact() {
	std::thread([this]{
		ui::merchantBox(name, trade[currTrade], ":Accept:Good-Bye", false, toSay->c_str());
		ui::waitForDialog();

		// handle normal dialog options
		switch (ui::dialogOptChosen) {
		// Accept
		case 1:
			if (!(player->inv->takeItem(trade[currTrade].item[1], trade[currTrade].quantity[1]))) {
				player->inv->addItem(trade[currTrade].item[0],trade[currTrade].quantity[0]);
				toSay = &text[1];
				interact();
			} else {
				toSay = &text[2];
				interact();
			}
			break;

		// Good-bye
		case 2:
			break;

		default:
			break;
		}

		// handle merchant-specific dialog options
		switch (ui::merchOptChosen) {
		// left arrow
		case 1:
			if (currTrade)
				currTrade--;
			ui::dontTypeOut();
			interact(); // TODO should we nest like this?
			break;

		// right arrow
		case 2:
			if (currTrade < trade.size() - 1)
				currTrade++;
			ui::dontTypeOut();
			interact();
			break;

		default:
			break;
		toSay = &text[0];
		}
	}).detach();
}

void Object::interact(void) {
	std::thread([this]{
		if(questObject && alive){
			ui::dialogBox(player->name, ":Yes:No", false, pickupDialog);
			ui::waitForDialog();
			if (ui::dialogOptChosen == 1) {
				player->inv->addItem(iname, 1);
				alive = false;
			}
		}else{
			alive = false;
			player->inv->addItem(iname, 1);
		}
	}).detach();
}

bool Entity::isInside(vec2 coord) const {
	return coord.x >= loc.x &&
	   	   coord.x <= loc.x + width &&
	   	   coord.y >= loc.y	&&
	       coord.y <= loc.y + height;
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

unsigned int Structures::spawn(BUILD_SUB sub, float x, float y) {
	loc.x = x;
	loc.y = y;
	type = STRUCTURET;

	alive = true;
	canMove = false;

	bsubtype = sub;
	dim2 dim;

	/*
	 *	tempN is the amount of entities that will be spawned in the village. Currently the village
	 *	will spawn bewteen 2 and 7 villagers for the starting hut.
	*/

	//unsigned int tempN = (randGet() % 5 + 2);

	if (textureLoc.empty())
		textureLoc = inWorld->getSTextureLocation(sub);

	switch(sub) {
		case STALL_MARKET:
			tex = TextureIterator({ textureLoc });
			dim = Texture::imageDim(textureLoc);
			width = HLINES(dim.x/2);
			height = HLINES(dim.y/2);
			break;
		default:
			tex = TextureIterator({ textureLoc });
			dim = Texture::imageDim(textureLoc);
			width = HLINES(dim.x/2);
			height = HLINES(dim.y/2);
			inv = NULL;
			break;
	}

	return 0;
}

/*Particles::Particles(const Structures *&s, vec2 vell, Color c, float d, )
{

}*/

Particles::Particles(float x, float y, float w, float h, float vx, float vy, Color c, float d)
{
	loc = vec2 {x, y};
	vel = vec2 {vx, vy};
	width = w;
	height = h;
	color = c;
	duration = d;
	gravity = true;
	fountain = false;
	behind = false;
	bounce = false;
	index = Texture::getIndex(c);
}

void Particles::draw(std::vector<GLfloat> &p) const
{
	vec2 tc = vec2 {0.25f * index.x, 0.125f * (8-index.y)};

    float z = 0.0;
    if (behind)
        z = 2.0;

	// lower left
    p.emplace_back(loc.x);
    p.emplace_back(loc.y);
    p.emplace_back(z);

	p.emplace_back(tc.x);
	p.emplace_back(tc.y);

	// lower right
    p.emplace_back(loc.x + width);
    p.emplace_back(loc.y);
    p.emplace_back(z);

	p.emplace_back(tc.x);
	p.emplace_back(tc.y);

	// upper right
    p.emplace_back(loc.x + width);
    p.emplace_back(loc.y + height);
    p.emplace_back(z);

	p.emplace_back(tc.x);
	p.emplace_back(tc.y);

	// upper right
    p.emplace_back(loc.x + width);
    p.emplace_back(loc.y + height);
    p.emplace_back(z);

	p.emplace_back(tc.x);
	p.emplace_back(tc.y);

	// upper left
    p.emplace_back(loc.x);
    p.emplace_back(loc.y + height);
    p.emplace_back(z);

	p.emplace_back(tc.x);
    p.emplace_back(tc.y);

	// lower left
    p.emplace_back(loc.x);
    p.emplace_back(loc.y);
    p.emplace_back(z);

    p.emplace_back(tc.x);
    p.emplace_back(tc.y);
}

void Particles::update(float _gravity, float ground_y)
{
	// handle ground collision
	if (loc.y < ground_y) {
		loc.y = ground_y;

		// handle bounce
		if (bounce) {
			vel.y *= -0.2f;
			vel.x /= 4.0f;
		} else {
			vel = 0.0f;
			canMove = false;
		}
	}

	// handle gravity
	else if (gravity && vel.y > -1.0f) {
		vel.y -= _gravity * game::time::getDeltaTime();
	}
}

bool Particles::kill(float delta)
{
	return (duration -= delta) <= 0;
}

void Player::save(void) {
	std::string data;
	std::ofstream out ("xml/main.dat",std::ios::out | std::ios::binary);
	std::cout<<"Saving player data..."<<std::endl;
	data.append(std::to_string((int)loc.x) + "\n");
	data.append(std::to_string((int)loc.y) + "\n");
	data.append(std::to_string((int)health) + "\n");
	data.append(std::to_string((int)maxHealth) + "\n");
	data.append(std::to_string((int)game::time::getTickCount()) + "\n");

	data.append("qwer\n");
	data.append(std::to_string((int)inv->Items.size()) + "\n");
	for(auto &i : inv->Items) {
		if(i.second)
			data.append(std::to_string(uint(i.second)) + "\n" + i.first->name + "\n");
	}
	data.append("qwer\n");

	data.append(std::string(currentXML.data() + 4) + "\n");

	data.append("dOnE\0");
	out.write(data.data(),data.size());
	out.close();
}

void Player::sspawn(float x,float y) {
	unsigned int i;
	int count;
	std::ifstream in (std::string(xmlFolder + "main.dat"),std::ios::in | std::ios::binary);
	spawn(x,y);

	if (in.good()) {
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
		game::time::tick(std::stoi(ddata));

		std::getline(data,ddata);
		std::getline(data,ddata);

		for (i = std::stoi(ddata);i;i--) {
			std::getline(data,ddata);
			if (ddata == "qwer")
				break;
			count = std::stoi(ddata);

			std::getline(data,ddata);
			if (ddata == "qwer")
				break;
			inv->addItem(ddata, (uint)count);
		}

		std::getline(data,ddata);
		currentWorld = loadWorldFromXMLNoSave(ddata);

		in.close();
	}
}
