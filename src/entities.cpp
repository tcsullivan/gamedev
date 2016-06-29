#include <entities.hpp>

#include <istream>
#include <sstream>
#include <fstream>

#include <ui.hpp>
#include <world.hpp>
#include <gametime.hpp>
#include <brice.hpp>

extern std::istream *names;

extern Player *player;			// main.cpp
extern World *currentWorld;		// main.cpp
extern unsigned int loops;		// main.cpp

extern std::string xmlFolder;
extern XMLDocument currentXMLDoc;

// a dynamic array of pointers to the NPC's that are being assigned the preloads
std::vector<NPC *> aipreload;

// the size of the player's inventory
const unsigned int PLAYER_INV_SIZE = 43;
// the size of an NPC's inventory
const unsigned int NPC_INV_SIZE = 3;

static std::vector<std::string> randomDialog (readFileA("assets/dialog_en-us"));

void getRandomName(Entity *e)
{
	auto names = readFileA("assets/names_en-us");
	auto name = names[randGet() % names.size()];

	// gender is a binary construct
	e->gender = (name[0] == 'm') ? MALE : FEMALE;

	e->name = &name[1];
}

Entity::Entity(void)
{
	vel = 0;
	width = 0;
	height = 0;
	health = 0;
	maxHealth = 0;
	outnabout = 0;
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
	z = 0.0f;

	// clear counters
	ticksToUse = 0;
	hitCooldown = 0;

	hitDuration = maxHitDuration = 0;

	inv = nullptr;
}

// spawns the entity you pass to it based off of coords and global entity settings
void Entity::spawn(float x, float y)
{
	loc.x = x;
	loc.y = y;

	if (health == 0 && maxHealth == 0)
		health = maxHealth = 1;

	// generate a name
	if (type == MOBT)
		name = "mob";
	else
		getRandomName(this);

	setCooldown(0);
}

void Entity::takeHit(unsigned int _health, unsigned int cooldown)
{
	if (hitCooldown <= 1) {
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

	/*if (xmle) {
		xmle->SetAttribute("alive", false);
		currentXMLDoc.SaveFile(currentXML.c_str(), false);
	}*/
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

	z = -2.0;
}

Player::~Player()
{
	delete inv;
	delete &tex;
}

void Player::createFromXML(XMLElement *e, World *w=nullptr)
{
	(void)e;
	(void)w;
}

void Player::saveToXML(void)
{}

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

	randDialog = randGet() % randomDialog.size();
	dialogIndex = 0;
	dialogCount = 0;

	dim2 tmpDim = Texture::imageDim(tex.getTexturePath(0));
	width = HLINES(tmpDim.x/2);
	height = HLINES(tmpDim.y/2);
}

NPC::~NPC()
{
	delete inv;
}

void NPC::createFromXML(XMLElement *e, World *w=nullptr)
{
	std::string npcname;
	bool dialog;
	unsigned int flooor;

	xmle = e;

    // spawn at coordinates if desired
	E_LOAD_COORDS(100);

    // name override
	if (!(npcname = e->StrAttribute("name")).empty())
		name = npcname;

    // dialog enabling
	dialog = false;
	if (e->QueryBoolAttribute("hasDialog", &dialog) == XML_NO_ERROR && dialog)
		addAIFunc(false);
	else
        dialogIndex = 9999;


    if (/*Indoor && */e->QueryUnsignedAttribute("floor", &flooor) == XML_NO_ERROR)
        Indoorp(w)->moveToFloor(this, flooor);

    // custom health value
	E_LOAD_HEALTH;

	// movemenet
	if (e->QueryBoolAttribute("canMove", &dialog) == XML_NO_ERROR)
		canMove = dialog;

	// dialog index
	if (e->QueryUnsignedAttribute("dindex", &flooor) == XML_NO_ERROR)
		dialogIndex = flooor;
}

void NPC::saveToXML(void)
{
	E_SAVE_HEALTH;
	E_SAVE_COORDS;
	xmle->SetAttribute("dindex", dialogIndex);
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
}

void Merchant::saveToXML(void){}

Structures::Structures() : Entity()
{
	type = STRUCTURET;
	canMove = false;
	health = maxHealth = 1;
}

Structures::~Structures()
{
}

extern std::string currentXMLRaw;

void Structures::createFromXML(XMLElement *e, World *w)
{
	float spawnx;

	if (e->QueryBoolAttribute("alive", &alive) == XML_NO_ERROR && !alive) {
		//die();
		return;
	}

	inWorld = w;
	inside = e->StrAttribute("inside");

	// edge
	if (!inside.empty()) {
		insideWorld = dynamic_cast<IndoorWorld *>(loadWorldFromXMLNoTakeover(inside));
		insideWorld->outside = inWorld;
	}

	textureLoc = e->StrAttribute("texture");

	spawn(static_cast<BUILD_SUB>(e->UnsignedAttribute("type")),
	      e->QueryFloatAttribute("spawnx", &spawnx) == XML_NO_ERROR ? spawnx : (randGet() % w->getTheWidth() / 2.0f),
	      100);

	xmle = e;
}

void Structures::saveToXML(void)
{
	xmle->SetAttribute("alive", alive);
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
}

void Object::createFromXML(XMLElement *e, World *w=nullptr)
{
	(void)e;
	(void)w;
}

void Object::saveToXML(void)
{}

void Object::reloadTexture(void)
{
	tex = TextureIterator({getItemTexturePath(iname)});
	width  = getItemWidth(iname);
	height = getItemHeight(iname);
}

bool Entity::isNear(const Entity *e) {
	return (e != nullptr) ? (pow(e->loc.x - loc.x, 2) + pow(e->loc.y - loc.y, 2) <= pow(HLINES(40), 2)) : false;
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

		// TODO use texture made for this
		static GLuint thingyColor = Texture::genColor(Color(236, 238, 15));

		glUseProgram(worldShader);

		glEnableVertexAttribArray(worldShader_attribute_coord);
		glEnableVertexAttribArray(worldShader_attribute_tex);

		glBindTexture(GL_TEXTURE_2D, thingyColor);

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
	        Mobp(this)->rider->loc.y = loc.y + height - HLINES(5);
	        Mobp(this)->rider->vel.y = .12;
			Mobp(this)->rider->z     = z + 0.01;
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
		loc.x, 			loc.y + height, 			      z + 0.1f,
		loc.x + width,	loc.y + height, 			      z + 0.1f,
		loc.x + width, 	loc.y + height + game::HLINE * 2, z + 0.1f,

		loc.x + width, 	loc.y + height + game::HLINE * 2, z + 0.1f,
		loc.x, 			loc.y + height + game::HLINE * 2, z + 0.1f,
		loc.x, 			loc.y + height, 			      z + 0.1f,
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
		else {
			targetx = 0.9112001f;
			vel.x = 0;
		}
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
		XMLDocument *xml;
		XMLElement *exml,*oxml;

		static unsigned int oldidx = 9999;
		const char *ptr;
		std::string nname;
		unsigned int idx;
		bool stop;
		float tgt = 0.12345678f;
		bool pmove = true, advance = false;

		loc.y += 5;

		// freeze the npc, face the player
		canMove = false;
		left    = (player->loc.x < loc.x);
		right   = !left;

		// if there's actual scripted stuff to do, do it
		if (dialogCount && dialogIndex != 9999) {
			// load the XML file and find the dialog tags
			if (outnabout == 0) {
				xml = &currentXMLDoc;
			} else if (outnabout < 0) {
				xml = new XMLDocument();
				xml->LoadFile((xmlFolder + currentWorld->getToLeft()).c_str());
			} else {
				xml = new XMLDocument();
				xml->LoadFile((xmlFolder + currentWorld->getToRight()).c_str());
			}

COMMONAIFUNC:
			idx = 0;
			stop = false;
			exml = xml->FirstChildElement("Dialog");

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
				while ((oxml = oxml->NextSiblingElement("give")));
			}

			// handle take tags
			if ((oxml = exml->FirstChildElement("take"))) {
				do player->inv->takeItem(oxml->Attribute("id"), oxml->UnsignedAttribute("count"));
				while ((oxml = oxml->NextSiblingElement()));
			}

			// handle movement directs
			if ((oxml = exml->FirstChildElement("gotox"))) {
				moveTo((tgt = std::stoi(oxml->GetText())));
				if (oxml->QueryBoolAttribute("playerMove", &pmove) != XML_NO_ERROR)
					pmove = true;
				if (oxml->QueryBoolAttribute("advance", &advance) != XML_NO_ERROR)
					advance = false;
			}

			// handle attribute setting
			if ((oxml = exml->FirstChildElement("set"))) {
				do game::setValue(oxml->StrAttribute("id"), oxml->StrAttribute("value"));
				while ((oxml = oxml->NextSiblingElement()));
				game::briceUpdate();
			}

			// asdlfkj
			auto txml = exml->FirstChildElement("content");
			if (txml == nullptr)
				goto OTHERSTUFF;

			ptr = txml->GetText() - 1;
			while (*++ptr && isspace(*ptr));

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

OTHERSTUFF:

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

			if (tgt != 0.12345678f) {
				stop = canMove;
				canMove = true;
				while (targetx != 0.9112001f) {
					if (!pmove)
						player->speed = 0;
				}
				if (!pmove) {
					pmove = true;
					player->speed = 1;
				}
				canMove = stop;
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

			// advance if desired
			else if (advance) {
				goto COMMONAIFUNC;
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
		ui::merchantBox(name.c_str(), trade[currTrade], ":Accept:Good-Bye", false, toSay->c_str());
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

	z = 1.0;
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
	zOffset = ((rand()%20)-10)/1000.0f;
	stu = nullptr;
}

void Particles::draw(GLfloat*& p) const
{
	vec2 tc = vec2(0.25f * index.x, 0.125f * (8.0f - index.y));

    float z = 0.9;
	if (behind)
		z = 2.0;

	z += zOffset;

	// lower left
    *(p++) = loc.x;
    *(p++) = loc.y;
    *(p++) = z;

	*(p++) = tc.x;
	*(p++) = tc.y;

	// lower right
    *(p++) = loc.x + width;
    *(p++) = loc.y;
    *(p++) = z;

	*(p++) = tc.x;
	*(p++) = tc.y;

	// upper right
    *(p++) = loc.x + width;
    *(p++) = loc.y + height;
    *(p++) = z;

	*(p++) = tc.x;
	*(p++) = tc.y;

	// upper right
    *(p++) = loc.x + width;
    *(p++) = loc.y + height;
    *(p++) = z;

	*(p++) = tc.x;
	*(p++) = tc.y;

	// upper left
    *(p++) = loc.x;
    *(p++) = loc.y + height;
    *(p++) = z;

	*(p++) = tc.x;
    *(p++) = tc.y;

	// lower left
    *(p++) = loc.x;
    *(p++) = loc.y;
    *(p++) = z;

    *(p++) = tc.x;
    *(p++) = tc.y;
}

void Particles::update(float _gravity, float ground_y)
{
	auto delta = game::time::getDeltaTime();

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
		vel.y -= _gravity * delta;
	}

	// handle lifetime
	duration -= delta;
}

bool Particles::timeUp(void)
{
	return !(duration > 0);
}

void Player::save(void) {
	std::string data;
	std::ofstream out (xmlFolder + ".main.dat", std::ios::out | std::ios::binary);
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
	std::ifstream in (xmlFolder + ".main.dat", std::ios::in | std::ios::binary);
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
