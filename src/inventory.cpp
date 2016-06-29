#include <inventory.hpp>

#include <numeric>

#include <entities.hpp>
#include <ui.hpp>
#include <gametime.hpp>

#include <tinyxml2.h>
using namespace tinyxml2;

extern Player *player;
extern GLuint invUI;
static vec2 itemLoc;
static const unsigned char numSlot = 7;
Mix_Chunk* swordSwing;

static std::vector<NewCurrency *> currencyMap;
static std::vector<Item *> ItemMap;


void itemDraw(Player *p, Item* d);

bool strCaseCmp(std::string one, std::string two)
{
	for (auto &s : one) {
		s = std::tolower(s);
	}
	for (auto &t : two) {
		t = std::tolower(t);
	}

	if (one == two) return true;
	return false;
}

void items(void)
{
	XMLDocument xml;
	xml.LoadFile("config/items.xml");
	XMLElement *exml = xml.FirstChildElement("item");
	XMLElement *cxml = xml.FirstChildElement("currency");

	while (cxml) {

		// NEWEWEWEWEWEWEWEW
		// TODO


		cxml = cxml->NextSiblingElement();
	}
	while (exml) {

		std::string name = exml->Attribute("type");
		// if the type is blank
		if (strCaseCmp(name, "blank")) {

			ItemMap.push_back(new BaseItem());

		// if the type is a sword
		} else if (strCaseCmp(name, "sword")) {

			Sword *tmpSword = new Sword();
			tmpSword->setDamage(exml->FloatAttribute("damage"));
			ItemMap.push_back(tmpSword->clone());
			delete tmpSword;

		// if the type is a bow
		} else if (strCaseCmp(name, "bow")) {

			Bow *tmpBow = new Bow();
			tmpBow->setDamage(exml->FloatAttribute("damage"));
			ItemMap.push_back(tmpBow->clone());
			delete tmpBow;

		// arrow
		} else if (strCaseCmp(name, "arrow")) {

			Arrow *tmpArrow = new Arrow();
			tmpArrow->setDamage(exml->FloatAttribute("damage"));
			ItemMap.push_back(tmpArrow->clone());
			delete tmpArrow;

		// uncooked / raw food
		}else if (strCaseCmp(name, "raw food")) {

			ItemMap.push_back(new RawFood());

		// cooked food or natural food
		} else if (strCaseCmp(name, "food") || strCaseCmp(name, "cooked food")) {

			ItemMap.push_back(new Food());

		// light
		} else if (strCaseCmp(name, "light")) {
				
			ItemMap.push_back(new ItemLight());

		// if type was not specified make it a base item
		} else {

			ItemMap.push_back(new BaseItem());
		}

		// set how much of the item we can stack
		if(exml->QueryUnsignedAttribute("maxStackSize", &ItemMap.back()->maxStackSize) != XML_NO_ERROR) {
			ItemMap.back()->maxStackSize = 1;
		}

		// set all of the texture frames we can use
		ItemMap.back()->tex = new Texturec(1, exml->Attribute("sprite"));

		// get the width and height of the object based off of its sprite
		dim2 tmpDim = Texture::imageDim(exml->Attribute("sprite"));
		ItemMap.back()->dim.x = HLINES(tmpDim.x/2);
		ItemMap.back()->dim.y = HLINES(tmpDim.y/2);

		ItemMap.back()->name = exml->Attribute("name");

		exml = exml->NextSiblingElement();
	}
}

int Inventory::addItem(std::string name, uint count)
{
	for (uint i = 0; i < ItemMap.size(); i++) {
		if (strCaseCmp(ItemMap[i]->name, name)) {
			for (auto &it : Items) {
				if (it.second && strCaseCmp(it.first->name, name)) {
					if ((it.second + count) < it.first->maxStackSize) {
						it.second += count;
						return 0;
					} else {
						count -= (it.second + count) - it.first->maxStackSize;
						it.second = it.first->maxStackSize;
					}
				}
			}
			uint tmpCount = count;
			do {
				if ((count) > ItemMap[i]->maxStackSize) {
					count -=  ItemMap[i]->maxStackSize;
					tmpCount = ItemMap[i]->maxStackSize;
				} else {
					tmpCount = count;
					count = 0;
				}
				Items[os] = std::make_pair(ItemMap[i]->clone(), tmpCount);
				if (!Items[os+1].second) {
					os++;
				} else {
					for (uint z = 0; z < Items.size(); z++) {
						if (!Items[z].second) {
							os = z;
						}
					}
				}
			} while (count > 0);
			return 0;
		}
	}
	return -1;
}

int Inventory::takeItem(std::string name, uint count)
{

	// returns
	// 0 = good
	// -1 = no such item exists
	// -2 = if item doesnt exist in inventory
	// postive items = number returned is how many more the item needs

	std::string iden;

	for (uint i = 0; i < ItemMap.size(); i++) {
		if (ItemMap[i]->name == name) {
			iden = name;
			break;
		}
	}

	if (iden.empty()) {
		return -1;
	}

	for (auto &i : Items) {
		if (i.second && i.first->name == iden) {
			if (count > i.second) {
				return (count - i.second);
			} else {
				i.second -= count;

			}
			return 0;
		}
	}

	return -2;
}

int Inventory::hasItem(std::string name) {

	for (auto &i : Items) {
		if (i.first->name == name) {
			return i.second;
		}
	}

	return 0;
}
void initInventorySprites(void) {

	items();

	// keep
	swordSwing = Mix_LoadWAV("assets/sounds/shortSwing.wav");
	Mix_Volume(2,100);
}

void destroyInventory(void) {

	// NEWEWEWEWEWEWEWEW
	while (!ItemMap.empty()) {
		delete ItemMap.back();
		ItemMap.pop_back();
	}

	Mix_FreeChunk(swordSwing);
}


const char *getItemTexturePath(std::string name){
	for (auto &i : ItemMap) {
		if (i->name == name)
			return i->tex->texLoc[0].c_str();
	}
	return NULL;
}

GLuint getItemTexture(std::string name) {
	for (auto &i : ItemMap) {
		if (i->name == name) {
			return i->tex->image[0];
		}
	}
	return 0;
}

float getItemWidth(std::string name) {
	for (auto &i : ItemMap) {
		if (i->name == name)
			return i->dim.x;
	}
	return 0;
}

float getItemHeight(std::string name) {
	for (auto &i : ItemMap) {
		if (i->name == name)
			return i->dim.y;
	}
	return 0;
}

Inventory::Inventory(unsigned int s) {
	sel=0;
	size=s;

	Items.resize(size);
	for (auto &i : Items) {
		i = std::make_pair(nullptr, 0);
	}
}

Inventory::~Inventory(void) {
}

void Inventory::setSelection(unsigned int s) {
	sel=s;
}

void Inventory::setSelectionUp() {
	if (!sel--)
		sel++;
}

void Inventory::setSelectionDown() {
	if (++sel >= numSlot)
		sel = numSlot - 1;
}

void Inventory::draw(void) {
	static std::vector<int>dfp(numSlot);
	static std::vector<Ray>iray(numSlot);
	static std::vector<vec2>curCoord(numSlot);
	static int range = 200;

	static std::vector<int>curdfp(4);
	static std::vector<Ray>curRay(4);
	static std::vector<vec2>curCurCoord(4);
	static int curRange = 100;

	static std::vector<int>massDfp(32);
	static std::vector<vec2>massRay(32);
	static std::vector<int>massOrder = {9,10,11,12,13,14,22,21,20,19,18,17,16,8,0,1,2,3,4,5,6,7,15,23,31,30,29,28,27,26,25,24};
	static std::vector<int>massOrderClosing = {31,30,23,29,22,15,28,21,14,7,27,20,13,6,26,19,12,5,25,18,11,4,24,17,10,3,16,9,2,8,1,0};
	static int massRange = 200;

	static int itemWide = 45;
	float angleB = (float)180/(float)numSlot;
	float angle = float(angleB/2.0f);
	unsigned int a = 0;
	//static vec2 mouseStart = {0,0};
	C("End define");

	auto deltaTime = game::time::getDeltaTime();
	auto SCREEN_WIDTH = game::SCREEN_WIDTH;
	auto SCREEN_HEIGHT = game::SCREEN_HEIGHT;

	for (auto &r : iray) {
		r.start.x = player->loc.x + (player->width  / 2);
		r.start.y = player->loc.y + (player->height / 2);
		curCoord[a++] = r.start;
	} a = 0;

	for (auto &cr : curRay) {
		cr.start.x = (offset.x + SCREEN_WIDTH / 2);
		cr.start.y =  offset.y - (a * itemWide * 1.5f);
		curCurCoord[a++] = cr.start;
	} a = 0;

	for (int r = 0; r < 4; r++) {
		for (int c = 0; c < 8; c++) {
			massRay[a  ].x = ((offset.x - SCREEN_WIDTH  / 2) + itemWide) + c * itemWide * 1.5f;
			massRay[a++].y = ((offset.y + SCREEN_HEIGHT / 2) - itemWide * 1.5f) - r * itemWide * 1.5f;
		}
	} a = 0;

	auto averagef = [](const std::vector<int> &v) {
		auto sum = std::accumulate(std::begin(v), std::end(v), 0);
		return sum / v.size();
	};

	ui::fontTransInv = 255 * (averagef(dfp) / range);
	if (ui::fontTransInv > 255)
		ui::fontTransInv = 255;
	else if (ui::fontTransInv < 0)
		ui::fontTransInv = 0;

	if (invOpening) {
		for (auto &d : dfp) {
			if (!a || dfp[a - 1] > 50)
				d += 4.0f * deltaTime;
			if (d > range)
				d = range;
			a++;
		} a = 0;

		for (auto &cd : curdfp) {
			if (!a || curdfp[a - 1] > 90)
				cd += 3.0f * deltaTime;
			if (cd > curRange)
				cd = curRange;
			a++;
		} a = 0;

		while (a < massOrder.size()) {
			if (!a || massDfp[massOrder[a - 1]] > massRange * 0.75f)
				massDfp[massOrder[a]] += 20.0f * deltaTime;
			if (massDfp[massOrder[a]] > massRange)
				massDfp[massOrder[a]] = massRange;
			a++;
		} a = 0;

		if (numSlot > 0)
			invOpen = true;
	} else {
		for (auto &d : dfp) {
			if (d > 0)
				d -= 4.5f * deltaTime;
		}
		for (auto &cd : curdfp) {
			if (cd > 0)
				cd -= 3.0f * deltaTime;
		}

		while (a < massRay.size()) {
			if (!a || massDfp[massOrderClosing[a - 1]] <= 0)
				massDfp[massOrderClosing[a]] -= 30.0f * deltaTime;
			else if (massDfp[massOrderClosing[a - 1]] < 0)
				massDfp[massOrderClosing[a - 1]] = 0;
			a++;
		} a = 0;

		if (std::all_of(std::begin(massDfp), std::end(massDfp), [](auto d) { return d <= 0; })) {
			invOpen = false;
			for (auto &md : massDfp) {
				if (md < 0)
					md = 0;
			}
		}

	}

	/*
	 * 	a = 0
	 */

	 C("Start drawing inventory");
	if (invOpen) {
        useShader(&textShader,
                  &textShader_uniform_texture,
                  &textShader_attribute_coord,
                  &textShader_attribute_tex);
		for(auto &mr : massRay) {
            float t = (((float)massDfp[a]/(float)massRange)*.5f);
            glActiveTexture(GL_TEXTURE0);
            glUseProgram(textShader);

			glBindTexture(GL_TEXTURE_2D, Texture::genColor(Color(0.0f,0.0f,0.0f, t >= 0? 255*t : 0)));
            glUniform1i(textShader_uniform_texture, 0);

            drawRect(vec2(mr.x-(itemWide/2), mr.y-(itemWide/2)), vec2(mr.x-(itemWide/2)+itemWide, mr.y-(itemWide/2)+itemWide), -6.0);

            glUseProgram(0);
			if (!Items.empty() && a+numSlot < Items.size() && Items[a+numSlot].second) {
				glUseProgram(textShader);
                glBindTexture(GL_TEXTURE_2D, Items[a+numSlot].first->tex->image[0]);//itemtex[items[a+numSlot].id]);
				glUniform4f(textShader_uniform_color, 1.0f, 1.0f, 1.0f, ((float)massDfp[a]/(float)(massRange?massRange:1))*0.8f);
                if (Items[a+numSlot].first->dim.y > Items[a+numSlot].first->dim.x) {
                    drawRect(vec2(mr.x-((itemWide/2)*((float)Items[a+numSlot].first->dim.x/(float)Items[a+numSlot].first->dim.y)),	mr.y-(itemWide/2)),
                             vec2(mr.x+((itemWide/2)*((float)Items[a+numSlot].first->dim.x/(float)Items[a+numSlot].first->dim.y)),	mr.y+(itemWide/2)), -6.1);
                }else{
                    drawRect(vec2(mr.x-(itemWide/2),mr.y-(itemWide/2)*((float)Items[a+numSlot].first->dim.y/(float)Items[a+numSlot].first->dim.x)),
                             vec2(mr.x-(itemWide/2),mr.y+(itemWide/2)*((float)Items[a+numSlot].first->dim.y/(float)Items[a+numSlot].first->dim.x)), -6.1);
                }
				ui::setFontColor(255,255,255,((float)massDfp[a]/(float)(massRange?massRange:1))*255);
				ui::putText(mr.x-(itemWide/2)+(itemWide*.85),mr.y-(itemWide/2),"%d",Items[a+numSlot].second);
				ui::setFontColor(255,255,255,255);
                glUseProgram(0);
			}
			a++;
		}a=0;

		for(auto &cr : curRay) {
			curCurCoord[a].x -= float((curdfp[a]) * cos(-1));
			curCurCoord[a].y += float((curdfp[a]) * sin(0));
			cr.end = curCurCoord[a];

            float curTrans = (((float)curdfp[a]/(float)(curRange?curRange:1))*0.5f);

            glUseProgram(textShader);
			glBindTexture(GL_TEXTURE_2D, Texture::genColor(Color(0.0f, 0.0f, 0.0f, curTrans >= 0 ? 255 * curTrans : 0)));
            drawRect(vec2(cr.end.x-(itemWide/2),		 cr.end.y-(itemWide/2)),
                     vec2(cr.end.x-(itemWide/2)+itemWide,cr.end.y-(itemWide/2)+itemWide), -6.0);
            glUseProgram(0);
			a++;
		}a=0;

		for(auto &r : iray) {
			angle = 180 - (angleB * a) - angleB / 2.0f;
			curCoord[a].x += float((dfp[a]) * cos(angle*PI/180));
			curCoord[a].y += float((dfp[a]) * sin(angle*PI/180));
			r.end = curCoord[a];

            float t = ((float)dfp[a]/(float)(range?range:1))*0.5f;

            glUseProgram(textShader);
 			glBindTexture(GL_TEXTURE_2D, Texture::genColor(Color(0.0f, 0.0f, 0.0f, t >= 0 ? 255 * t : 0)));
            drawRect(vec2(r.end.x-(itemWide/2),		 r.end.y-(itemWide/2)),
                     vec2(r.end.x-(itemWide/2)+itemWide,r.end.y-(itemWide/2)+itemWide), -6.0);

			if (!Items.empty() && a < numSlot && Items[a].second) {
				glBindTexture(GL_TEXTURE_2D, Items[a].first->tex->image[0]);//itemtex[items[a].id]);
				glUniform4f(textShader_uniform_color, 1.0f, 1.0f, 1.0f, ((float)dfp[a]/(float)(range?range:1))*0.8f);
				if (Items[a].first->dim.y > Items[a].first->dim.x) {
				    drawRect(vec2(r.end.x-((itemWide/2)*((float)Items[a].first->dim.x/(float)Items[a].first->dim.y)),	r.end.y-(itemWide/2)),
                             vec2(r.end.x+((itemWide/2)*((float)Items[a].first->dim.x/(float)Items[a].first->dim.y)),	r.end.y+(itemWide/2)), -6.1);
                }else{
                    drawRect(vec2(r.end.x-(itemWide/2),r.end.y-(itemWide/2)*((float)Items[a].first->dim.y/(float)Items[a].first->dim.x)),
                             vec2(r.end.x+(itemWide/2),r.end.y+(itemWide/2)*((float)Items[a].first->dim.y/(float)Items[a].first->dim.x)), -6.1);
				}
				ui::setFontColor(255,255,255,((float)dfp[a]/(float)(range?range:1))*255);
				ui::putStringCentered(r.end.x,r.end.y-(itemWide*.9),Items[a].first->name);//itemMap[items[a].id]->name);
				ui::putText(r.end.x-(itemWide/2)+(itemWide*.85),r.end.y-(itemWide/2),"%d",Items[a].second);
				ui::setFontColor(255,255,255,255);
			}
            glUseProgram(0);
			if (sel == a) {
				static float sc = 1;
				static bool up;
				up ? sc += .0025*deltaTime : sc -= .0025*deltaTime;
				if (sc > 1.2) {
					up = false;
					sc = 1.2;
				}
				if (sc < 1.0) {
					up = true;
					sc = 1.0;
				}
                float t = ((float)dfp[a]/(float)(range?range:1));
                useShader(&textShader,
                          &textShader_uniform_texture,
                          &textShader_attribute_coord,
                          &textShader_attribute_tex);

                glUseProgram(textShader);
                glUniform4f(textShader_uniform_color, 1.0, 1.0, 1.0, 1.0);

                // bottom
                glBindTexture(GL_TEXTURE_2D, Texture::genColor(Color(255, 255, 255, t >= 0 ? 255 * t : 0)));
                drawRect(vec2(r.end.x - (itemWide*sc)/2 - (itemWide*sc)*.09,r.end.y - (itemWide*sc)/2 - (itemWide*sc)*.09),
                         vec2(r.end.x + (itemWide*sc)/2 + (itemWide*sc)*.09,r.end.y - (itemWide*sc)/2), -6.2);

                // top
                glBindTexture(GL_TEXTURE_2D, Texture::genColor(Color(255, 255, 255, t  >= 0 ? 255 * t : 0)));
                drawRect(vec2(r.end.x - (itemWide*sc)/2 - (itemWide*sc)*.09,r.end.y + (itemWide*sc)/2 + (itemWide*sc)*.09),
                         vec2(r.end.x + (itemWide*sc)/2 + (itemWide*sc)*.09,r.end.y + (itemWide*sc)/2), -6.2);

                // left
                glBindTexture(GL_TEXTURE_2D, Texture::genColor(Color(255, 255, 255, t >= 0 ? 255 * t : 0)));
                drawRect(vec2(r.end.x - (itemWide*sc)/2 - (itemWide*sc)*.09,r.end.y - (itemWide*sc)/2 - (itemWide*sc)*.09),
                         vec2(r.end.x - (itemWide*sc)/2				   ,r.end.y + (itemWide*sc)/2 + (itemWide*sc)*.09), -6.2);

                // right
                glBindTexture(GL_TEXTURE_2D, Texture::genColor(Color(255, 255, 255, t >= 0 ? 255 * t : 0)));
                drawRect(vec2(r.end.x + (itemWide*sc)/2					,r.end.y - (itemWide*sc)/2 - (itemWide*sc)*.09),
                         vec2(r.end.x + (itemWide*sc)/2 + (itemWide*sc)*.09,r.end.y + (itemWide*sc)/2 + (itemWide*sc)*.09), -6.2);

                //glUseProgram(0);
			}
			a++;
		}
		C("Done drawing standard inv");
	} /*else if (invHover) {
		static unsigned int highlight = 0;
		static unsigned int thing = 0;

		if (!mouseSel) {
			// setup?
			mouseStart.x = ui::mouse.x - offset.x;
			highlight = sel;
			thing = sel;
			mouseSel = true;
		} else {
			if((ui::mouse.x - offset.x) >= mouseStart.x){
				thing = (ui::mouse.x - offset.x - mouseStart.x)/80;
				highlight=sel+thing;
				if(highlight>numSlot-1)highlight=numSlot-1;
				if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)){
					sel = highlight;
					mouseSel=false;
					invHover=false;
					selected = true;
				}
			}
			if ((ui::mouse.x - offset.x) < mouseStart.x) {
				thing = (mouseStart.x - (ui::mouse.x - offset.x))/80;
				if ((int)sel - (int)thing < 0)
					highlight = 0;
				else
					highlight = sel - thing;
				if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)){
					sel = highlight;
					mouseSel=false;
					invHover=false;
					selected = true;
				}
			}
		}

		a = 0;
		for (auto &r : iray) {
			angle = 180 - (angleB * a) - angleB / 2.0f;
			curCoord[a].x += float(range) * cos(angle*PI/180);
			curCoord[a].y += float(range) * sin(angle*PI/180);
			r.end = curCoord[a];

			// square drawing
			glColor4f(0.0f, 0.0f, 0.0f, a == highlight ? 0.5f : 0.1f);
 			glBegin(GL_QUADS);
				glVertex2i(r.end.x-(itemWide/2),	r.end.y-(itemWide/2));
				glVertex2i(r.end.x+(itemWide/2),	r.end.y-(itemWide/2));
				glVertex2i(r.end.x+(itemWide/2),	r.end.y+(itemWide/2));
				glVertex2i(r.end.x-(itemWide/2),	r.end.y+(itemWide/2));
			glEnd();

			if (a < items.size() && items[a].count) {
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, itemtex[items[a].id]);
				glColor4f(1.0f, 1.0f, 1.0f, a == highlight ? 0.8f : 0.2f);
				glBegin(GL_QUADS);
					if(itemMap[items[a].id]->height > itemMap[items[a].id]->width){
						glTexCoord2i(0,1);glVertex2i(r.end.x-((itemWide/2)*((float)itemMap[items[a].id]->width/(float)itemMap[items[a].id]->height)),r.end.y-(itemWide/2));
						glTexCoord2i(1,1);glVertex2i(r.end.x+((itemWide/2)*((float)itemMap[items[a].id]->width/(float)itemMap[items[a].id]->height)),r.end.y-(itemWide/2));
						glTexCoord2i(1,0);glVertex2i(r.end.x+((itemWide/2)*((float)itemMap[items[a].id]->width/(float)itemMap[items[a].id]->height)),r.end.y+(itemWide/2));
						glTexCoord2i(0,0);glVertex2i(r.end.x-((itemWide/2)*((float)itemMap[items[a].id]->width/(float)itemMap[items[a].id]->height)),r.end.y+(itemWide/2));
					}else{
						glTexCoord2i(0,1);glVertex2i(r.end.x-(itemWide/2),r.end.y-(itemWide/2)*((float)itemMap[items[a].id]->height/(float)itemMap[items[a].id]->width));
						glTexCoord2i(1,1);glVertex2i(r.end.x+(itemWide/2),r.end.y-(itemWide/2)*((float)itemMap[items[a].id]->height/(float)itemMap[items[a].id]->width));
						glTexCoord2i(1,0);glVertex2i(r.end.x+(itemWide/2),r.end.y+(itemWide/2)*((float)itemMap[items[a].id]->height/(float)itemMap[items[a].id]->width));
						glTexCoord2i(0,0);glVertex2i(r.end.x-(itemWide/2),r.end.y+(itemWide/2)*((float)itemMap[items[a].id]->height/(float)itemMap[items[a].id]->width));
					}
				glEnd();
				glDisable(GL_TEXTURE_2D);
			}
			a++;
		}

		if (highlight < items.size()) {
			ui::putStringCentered(player->loc.x + player->width / 2,
			                      player->loc.y + range * 0.75f,
							      itemMap[items[highlight].id]->name
						          );
		}
	}*/
	/*if (!items.empty() && items.size() > sel && items[sel].count)
		itemDraw(player,items[sel].id);*/
	if (!Items.empty() && Items.size() > sel && Items[sel].second)
		itemDraw(player, Items[sel].first);
}

void itemDraw(Player *p, Item *d) {

	itemLoc.y = p->loc.y+(p->height/3);
	itemLoc.x = p->left?p->loc.x-d->dim.x/2:p->loc.x+p->width-d->dim.x/2;

	glUseProgram(worldShader);

	if (p->left) {
		// move to center of screen
		glm::mat4 tro = glm::translate(glm::mat4(1.0f),
									   glm::vec3(itemLoc.x+d->dim.x/2, itemLoc.y, 0));
		// rotate off center
		glm::mat4 rot = glm::rotate(glm::mat4(1.0f),
									static_cast<GLfloat>((d->rotation*3.14159)/180.0f),
									glm::vec3(0.0f, 0.0f, 1.0f));
		// move back to player
		glm::mat4 trt = glm::translate(glm::mat4(1.0f),
									   glm::vec3(-itemLoc.x-d->dim.x/2, -itemLoc.y, 0));
		// tell shader to translate the object using steps above
		glUniformMatrix4fv(worldShader_uniform_transform, 1, GL_FALSE, glm::value_ptr(tro * rot * trt));
	} else {
		// move to center of screen
		glm::mat4 tro = glm::translate(glm::mat4(1.0f),
									   glm::vec3(itemLoc.x+d->dim.x/2,itemLoc.y,0));
		// rotate off center
		glm::mat4 rot = glm::rotate(glm::mat4(1.0f),
									static_cast<GLfloat>((d->rotation*3.14159)/180.0f),
									glm::vec3(0.0f, 0.0f, 1.0f));
		// move back to player
		glm::mat4 trt = glm::translate(glm::mat4(1.0f),
									   glm::vec3(-itemLoc.x-d->dim.x/2,-itemLoc.y,0));
		// tell shader to translate the object using steps above
		glUniformMatrix4fv(worldShader_uniform_transform, 1, GL_FALSE, glm::value_ptr(tro * rot * trt));
	}

    GLfloat itemTex[12] = {0.0, 0.0,
                           1.0, 0.0,
                           1.0, 1.0,

                           1.0, 1.0,
                           0.0, 1.0,
                           0.0, 0.0};
    if (!p->left) {
        itemTex[0] = 1.0;
        itemTex[2] = 0.0;
        itemTex[4] = 0.0;
        itemTex[6] = 0.0;
        itemTex[8] = 1.0;
        itemTex[10] = 1.0;
    }

    GLfloat itemCoords[] = {itemLoc.x,          itemLoc.y,          p->z,
                            itemLoc.x+d->dim.x, itemLoc.y,          p->z,
                            itemLoc.x+d->dim.x, itemLoc.y+d->dim.y, p->z,

                            itemLoc.x+d->dim.x, itemLoc.y+d->dim.y, p->z,
                            itemLoc.x,          itemLoc.y+d->dim.y, p->z,
                            itemLoc.x,          itemLoc.y,          p->z};

	glBindTexture(GL_TEXTURE_2D,d->tex->image[0]);

    glEnableVertexAttribArray(worldShader_attribute_coord);
    glEnableVertexAttribArray(worldShader_attribute_tex);

    glVertexAttribPointer(worldShader_attribute_coord, 3, GL_FLOAT, GL_FALSE, 0, itemCoords);
    glVertexAttribPointer(worldShader_attribute_tex, 2, GL_FLOAT, GL_FALSE, 0, itemTex);
    glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(worldShader_attribute_coord);
    glDisableVertexAttribArray(worldShader_attribute_tex);

    glUseProgram(0);
}

/*
 *	This function is used to trigger the player's item's ability.
 */
int Inventory::useItem(void)
{
	return 0;
}

int Inventory::useCurrent()
{
	if (Items[sel].second)
		return Items[sel].first->useItem();
	return -1;
}

void Inventory::currentAddInteract(Entity* e)
{
	if (Items[sel].second)
		Items[sel].first->addInteract(e);
}

void Inventory::currentAddInteract(std::vector<Entity*> e)
{
	if (Items[sel].second)
		Items[sel].first->addInteract(e);
}

bool Inventory::detectCollision(vec2 one, vec2 two) {
	(void)one;
	(void)two;
	return false;
}

const Item* Inventory::getCurrentItem(void)
{
	if (Items.size() > 0)
		return Items[sel].first;
	else
		return nullptr;
}
