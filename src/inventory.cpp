#include <inventory.hpp>
#include <entities.hpp>
#include <ui.hpp>

#include <tinyxml2.h>
using namespace tinyxml2;

extern Player *player;
extern GLuint invUI;
static float hangle = 0.0f;
static bool swing = false;
static vec2 itemLoc;
static const unsigned char numSlot = 7;
Mix_Chunk* swordSwing;

static std::vector<Item *> itemMap;
static GLuint *itemtex;
void itemDraw(Player *p,uint id);

void items(void){
	XMLDocument xml;
	XMLElement *exml;
	xml.LoadFile("config/items.xml");
	exml = xml.FirstChildElement("item");
	while(exml){

		itemMap.push_back(new Item());

		itemMap.back()->width  = exml->FloatAttribute("width") * HLINE;
		itemMap.back()->height = exml->FloatAttribute("height") * HLINE;
		itemMap.back()->maxStackSize = exml->UnsignedAttribute("maxstack");
		itemMap.back()->attribValue = exml->FloatAttribute("value");


		itemMap.back()->name = exml->Attribute("name");
		itemMap.back()->type = exml->Attribute("type");
		itemMap.back()->texloc = exml->Attribute("sprite");

		exml = exml->NextSiblingElement();
	}
}

int Inventory::addItem(std::string name,uint count){
	for(unsigned int i=0;i<itemMap.size();i++){
		if(itemMap[i]->name == name){
			for(auto &in : items){
				if(in.id == i){
					in.count += count;
					return 0;
				}
			}
			items.push_back( item_t { count, i });
			return 0;
		}
	}
	return -1;
}

int Inventory::takeItem(std::string name,uint count){
	unsigned int id = 999999;

	/*
	 * Name to ID lookup
	 */

	for(unsigned int i=0;i<itemMap.size();i++){
		if(itemMap[i]->name == name){
			id = i;
			break;
		}
	}

	if(id == 999999)
		return -1; //if no such item exists

	/*
	 * Inventory lookup
	 */

	for(unsigned int i=0;i<items.size();i++){
		if(items[i].id == id){
			if(count > items[i].count)
				return -(items[i].count - count);
			else{
				items[i].count -= count;
				if(!items[i].count)
					items.erase(items.begin()+i);
			}
			return 0;
		}
	}
	return -2;
}

int Inventory::hasItem(std::string name){
	unsigned int id = 999999;

	for(unsigned int i=0;i<itemMap.size();i++){
		if(itemMap[i]->name == name){
			id = i;
			break;
		}
	}

	if(id == 999999)
		return 0;

	for(auto &i : items){
		if(i.id == id)
			return i.count;
	}

	return 0;
}

void initInventorySprites(void){

	items();
	itemtex = new GLuint[itemMap.size()];

	for(unsigned int i = 0;i<itemMap.size();i++){
		itemtex[i] = Texture::loadTexture(getItemTexturePath(itemMap[i]->name));
	}

	swordSwing = Mix_LoadWAV("assets/sounds/shortSwing.wav");
	Mix_Volume(2,100);
}

void destroyInventory(void){

	while(!itemMap.empty()){
		delete itemMap.front();
		itemMap.erase(itemMap.begin());
	}

	Mix_FreeChunk(swordSwing);
}

const char *getItemTexturePath(std::string name){
	for(auto &i : itemMap){
		if(i->name == name)
			return i->texloc.c_str();
	}
	return NULL;
}

GLuint getItemTexture(std::string name){
	for ( int i = itemMap.size(); i--; ) {
		if ( itemMap[i]->name == name )
			return itemtex[i];
	}
	DEBUG_printf("Failed to find texture for item %s!", name.c_str() );
	return 0;
}

float getItemWidth(std::string name){
	for(auto &i : itemMap){
		if(i->name == name)
			return i->width;
	}
	return 0;
}

float getItemHeight(std::string name){
	for(auto &i : itemMap){
		if(i->name == name)
			return i->height;
	}
	return 0;
}

Inventory::Inventory(unsigned int s){
	sel=0;
	size=s;
}

Inventory::~Inventory(void){
}

void Inventory::setSelection(unsigned int s){
	sel=s;
}

void Inventory::setSelectionUp(){
	if ( !sel-- )
		sel++;
}

void Inventory::setSelectionDown(){
	if ( ++sel >= numSlot )
		sel = numSlot - 1;
}

void Inventory::draw(void){
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
	static vec2 mouseStart = {0,0};
	C("End define");

	for ( auto &r : iray ) {
		r.start.x = player->loc.x + (player->width  / 2);
		r.start.y = player->loc.y + (player->height / 2);
		curCoord[a++] = r.start;
	} a = 0;

	for ( auto &cr : curRay ) {
		cr.start.x = (offset.x + SCREEN_WIDTH / 2);
		cr.start.y =  offset.y - (a * itemWide * 1.5f);
		curCurCoord[a++] = cr.start;
	} a = 0;

	for ( int r = 0; r < 4; r++ ) {
		for ( int c = 0; c < 8; c++ ) {
			massRay[a  ].x = ((offset.x - SCREEN_WIDTH  / 2) + itemWide       ) + c * itemWide * 1.5f;
			massRay[a++].y = ((offset.y + SCREEN_HEIGHT / 2) - itemWide * 1.5f) - r * itemWide * 1.5f;
		}
	} a = 0;

	ui::fontTransInv = 255 * (averagef(dfp) / range);
	if ( ui::fontTransInv > 255 )
		ui::fontTransInv = 255;
	else if ( ui::fontTransInv < 0 )
		ui::fontTransInv = 0;

	if ( invOpening ) {
		for ( auto &d : dfp ) {
			if ( !a || dfp[a - 1] > 50 )
				d += 1.65f * deltaTime;
			if ( d > range )
				d = range;
			a++;
		} a = 0;

		for ( auto &cd : curdfp ) {
			if ( !a || curdfp[a - 1] > 90 )
				cd += 1.5f * deltaTime;
			if ( cd > curRange )
				cd = curRange;
			a++;
		} a = 0;

		for ( unsigned int i = 0; i < massOrder.size() ; i++, a++ ) {
			if ( !a || massDfp[ massOrder[a - 1] ] > massRange * 0.75f )
				massDfp[ massOrder[a] ] += 5.0f * deltaTime;
			if ( massDfp[ massOrder[a] ] > massRange )
				massDfp[ massOrder[a] ] = massRange;
		} a = 0;

		if ( numSlot > 0 )
			invOpen = true;
	} else {
		for ( auto &d : dfp ) {
			if ( d > 0 )
				d -= 1.65f * deltaTime;
		}
		for ( auto &cd : curdfp ) {
			if ( cd > 0 )
				cd -= 1.0f * deltaTime;
		}

		for ( unsigned int i = 0; i < massRay.size(); i++, a++ ) {
			if ( !a || massDfp[ massOrderClosing[a - 1] ] <= 0 )
				massDfp[ massOrderClosing[a] ] -= 10.0f * deltaTime;
			if ( massDfp[ massOrderClosing[a - 1] ] < 0 )
				massDfp[ massOrderClosing[a - 1] ] = 0;
		} a = 0;

		if ( std::all_of( std::begin(massDfp), std::end(massDfp), [](auto d){ return d <= 0; } ) ) {
			invOpen = false;
			for ( auto &md : massDfp ) {
				if ( md < 0 )
					md = 0;
			}
		}

	}

	/*
	 * 	a = 0
	 */

	 C("Start drawing inventory");
	if(invOpen){

		for(auto &mr : massRay){
			glColor4f(0.0f,0.0f,0.0f, ((float)massDfp[a]/(float)massRange)*.5f);
			glBegin(GL_QUADS);
				glVertex2i(mr.x-(itemWide/2),		 mr.y-(itemWide/2));
				glVertex2i(mr.x-(itemWide/2)+itemWide,mr.y-(itemWide/2));
				glVertex2i(mr.x-(itemWide/2)+itemWide,mr.y-(itemWide/2)+itemWide);
				glVertex2i(mr.x-(itemWide/2),		 mr.y-(itemWide/2)+itemWide);
			glEnd();
			if(!items.empty() && a < items.size() && items[a+numSlot].count){
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, itemtex[items[a+numSlot].id]);
				glColor4f(1.0f, 1.0f, 1.0f, ((float)massDfp[a]/(float)(massRange?massRange:1))*0.8f);
				glBegin(GL_QUADS);
					if(itemMap[items[a].id]->height > itemMap[items[a+numSlot].id]->width){
						glTexCoord2i(0,1);glVertex2i(mr.x-((itemWide/2)*((float)itemMap[items[a+numSlot].id]->width/(float)itemMap[items[a+numSlot].id]->height)),	mr.y-(itemWide/2));
						glTexCoord2i(1,1);glVertex2i(mr.x+((itemWide/2)*((float)itemMap[items[a+numSlot].id]->width/(float)itemMap[items[a+numSlot].id]->height)),	mr.y-(itemWide/2));
						glTexCoord2i(1,0);glVertex2i(mr.x+((itemWide/2)*((float)itemMap[items[a+numSlot].id]->width/(float)itemMap[items[a+numSlot].id]->height)),	mr.y+(itemWide/2));
						glTexCoord2i(0,0);glVertex2i(mr.x-((itemWide/2)*((float)itemMap[items[a+numSlot].id]->width/(float)itemMap[items[a+numSlot].id]->height)),	mr.y+(itemWide/2));
					}else{
						glTexCoord2i(0,1);glVertex2i(mr.x-(itemWide/2),mr.y-(itemWide/2)*((float)itemMap[items[a+numSlot].id]->height/(float)itemMap[items[a+numSlot].id]->width));
						glTexCoord2i(1,1);glVertex2i(mr.x+(itemWide/2),mr.y-(itemWide/2)*((float)itemMap[items[a+numSlot].id]->height/(float)itemMap[items[a+numSlot].id]->width));
						glTexCoord2i(1,0);glVertex2i(mr.x+(itemWide/2),mr.y+(itemWide/2)*((float)itemMap[items[a+numSlot].id]->height/(float)itemMap[items[a+numSlot].id]->width));
						glTexCoord2i(0,0);glVertex2i(mr.x-(itemWide/2),mr.y+(itemWide/2)*((float)itemMap[items[a+numSlot].id]->height/(float)itemMap[items[a+numSlot].id]->width));
					}
				glEnd();
				glDisable(GL_TEXTURE_2D);
				ui::setFontColor(255,255,255,((float)massDfp[a]/(float)(massRange?massRange:1))*255);
				ui::putText(mr.x-(itemWide/2)+(itemWide*.85),mr.y-(itemWide/2),"%d",items[a+numSlot].count);
				ui::setFontColor(255,255,255,255);
			}
			a++;
		}a=0;

		for(auto &cr : curRay){
			curCurCoord[a].x -= float((curdfp[a]) * cos(-1));
			curCurCoord[a].y += float((curdfp[a]) * sin(0));
			cr.end = curCurCoord[a];

			glColor4f(0.0f, 0.0f, 0.0f, ((float)curdfp[a]/(float)(curRange?curRange:1))*0.5f);
 			glBegin(GL_QUADS);
				glVertex2i(cr.end.x-(itemWide/2),		 cr.end.y-(itemWide/2));
				glVertex2i(cr.end.x-(itemWide/2)+itemWide,cr.end.y-(itemWide/2));
				glVertex2i(cr.end.x-(itemWide/2)+itemWide,cr.end.y-(itemWide/2)+itemWide);
				glVertex2i(cr.end.x-(itemWide/2),		 cr.end.y-(itemWide/2)+itemWide);
			glEnd();
			a++;
		}a=0;

		for(auto &r : iray){
			angle = 180 - (angleB * a) - angleB / 2.0f;
			curCoord[a].x += float((dfp[a]) * cos(angle*PI/180));
			curCoord[a].y += float((dfp[a]) * sin(angle*PI/180));
			r.end = curCoord[a];

			glColor4f(0.0f, 0.0f, 0.0f, ((float)dfp[a]/(float)(range?range:1))*0.5f);
 			glBegin(GL_QUADS);
				glVertex2i(r.end.x-(itemWide/2),		 r.end.y-(itemWide/2));
				glVertex2i(r.end.x-(itemWide/2)+itemWide,r.end.y-(itemWide/2));
				glVertex2i(r.end.x-(itemWide/2)+itemWide,r.end.y-(itemWide/2)+itemWide);
				glVertex2i(r.end.x-(itemWide/2),		 r.end.y-(itemWide/2)+itemWide);
			glEnd();

			if(!items.empty() && a < items.size() && items[a].count){
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, itemtex[items[a].id]);
				glColor4f(1.0f, 1.0f, 1.0f, ((float)dfp[a]/(float)(range?range:1))*0.8f);
				glBegin(GL_QUADS);
					if(itemMap[items[a].id]->height > itemMap[items[a].id]->width){
						glTexCoord2i(0,1);glVertex2i(r.end.x-((itemWide/2)*((float)itemMap[items[a].id]->width/(float)itemMap[items[a].id]->height)),	r.end.y-(itemWide/2));
						glTexCoord2i(1,1);glVertex2i(r.end.x+((itemWide/2)*((float)itemMap[items[a].id]->width/(float)itemMap[items[a].id]->height)),	r.end.y-(itemWide/2));
						glTexCoord2i(1,0);glVertex2i(r.end.x+((itemWide/2)*((float)itemMap[items[a].id]->width/(float)itemMap[items[a].id]->height)),	r.end.y+(itemWide/2));
						glTexCoord2i(0,0);glVertex2i(r.end.x-((itemWide/2)*((float)itemMap[items[a].id]->width/(float)itemMap[items[a].id]->height)),	r.end.y+(itemWide/2));
					}else{
						glTexCoord2i(0,1);glVertex2i(r.end.x-(itemWide/2),r.end.y-(itemWide/2)*((float)itemMap[items[a].id]->height/(float)itemMap[items[a].id]->width));
						glTexCoord2i(1,1);glVertex2i(r.end.x+(itemWide/2),r.end.y-(itemWide/2)*((float)itemMap[items[a].id]->height/(float)itemMap[items[a].id]->width));
						glTexCoord2i(1,0);glVertex2i(r.end.x+(itemWide/2),r.end.y+(itemWide/2)*((float)itemMap[items[a].id]->height/(float)itemMap[items[a].id]->width));
						glTexCoord2i(0,0);glVertex2i(r.end.x-(itemWide/2),r.end.y+(itemWide/2)*((float)itemMap[items[a].id]->height/(float)itemMap[items[a].id]->width));
					}
				glEnd();
				glDisable(GL_TEXTURE_2D);
				ui::setFontColor(255,255,255,((float)dfp[a]/(float)(range?range:1))*255);
				ui::putStringCentered(r.end.x,r.end.y-(itemWide*.9),itemMap[items[a].id]->name.c_str());
				ui::putText(r.end.x-(itemWide/2)+(itemWide*.85),r.end.y-(itemWide/2),"%d",items[a].count);
				ui::setFontColor(255,255,255,255);
			}

			if(sel == a){
				static float sc = 1;
				static bool up;
				up ? sc += .0005*deltaTime : sc -= .0005*deltaTime;
				if(sc > 1.2){
					up = false;
					sc = 1.2;
				}
				if(sc < 1.0){
					up = true;
					sc = 1.0;
				}
	 			glBegin(GL_QUADS);
	 				glColor4f(1.0f, 1.0f, 1.0f, ((float)dfp[a]/(float)(range?range:1)));
					glVertex2f(r.end.x - (itemWide*sc)/2 - (itemWide*sc)*.09,r.end.y - (itemWide*sc)/2 - (itemWide*sc)*.09);
					glVertex2f(r.end.x + (itemWide*sc)/2 + (itemWide*sc)*.09,r.end.y - (itemWide*sc)/2 - (itemWide*sc)*.09);
					glVertex2f(r.end.x + (itemWide*sc)/2 + (itemWide*sc)*.09,r.end.y - (itemWide*sc)/2);
					glVertex2f(r.end.x - (itemWide*sc)/2 - (itemWide*sc)*.09,r.end.y - (itemWide*sc)/2);

					glVertex2f(r.end.x - (itemWide*sc)/2 - (itemWide*sc)*.09,r.end.y + (itemWide*sc)/2 + (itemWide*sc)*.09);
					glVertex2f(r.end.x + (itemWide*sc)/2 + (itemWide*sc)*.09,r.end.y + (itemWide*sc)/2 + (itemWide*sc)*.09);
					glVertex2f(r.end.x + (itemWide*sc)/2 + (itemWide*sc)*.09,r.end.y + (itemWide*sc)/2);
					glVertex2f(r.end.x - (itemWide*sc)/2 - (itemWide*sc)*.09,r.end.y + (itemWide*sc)/2);

					glVertex2f(r.end.x - (itemWide*sc)/2 - (itemWide*sc)*.09,r.end.y - (itemWide*sc)/2 - (itemWide*sc)*.09);
					glVertex2f(r.end.x - (itemWide*sc)/2				   ,r.end.y - (itemWide*sc)/2 - (itemWide*sc)*.09);
					glVertex2f(r.end.x - (itemWide*sc)/2				   ,r.end.y + (itemWide*sc)/2 + (itemWide*sc)*.09);
					glVertex2f(r.end.x - (itemWide*sc)/2 - (itemWide*sc)*.09,r.end.y + (itemWide*sc)/2 + (itemWide*sc)*.09);

					glVertex2f(r.end.x + (itemWide*sc)/2					,r.end.y - (itemWide*sc)/2 - (itemWide*sc)*.09);
					glVertex2f(r.end.x + (itemWide*sc)/2 + (itemWide*sc)*.09,r.end.y - (itemWide*sc)/2 - (itemWide*sc)*.09);
					glVertex2f(r.end.x + (itemWide*sc)/2 + (itemWide*sc)*.09,r.end.y + (itemWide*sc)/2 + (itemWide*sc)*.09);
					glVertex2f(r.end.x + (itemWide*sc)/2					,r.end.y + (itemWide*sc)/2 + (itemWide*sc)*.09);
				glEnd();
			}
			a++;
		}
		C("Done drawing standard inv");
	}else if(invHover){
		static unsigned int highlight = 0;
		static unsigned int thing = 0;

		std::cout<<"Inventory2???"<<std::endl;

		if(!mouseSel){
			mouseStart.x = ui::mouse.x - offset.x;
			std::cout << "Setting highlight" << std::endl;
			highlight = sel;
			std::cout << "Setting thing" << std::endl;
			thing = sel;
			std::cout << "Setting mouseSel" << std::endl;
			mouseSel=true;
			std::cout << "Done" << std::endl;
		}else{
			std::cout << "Is mousex greater than the start" << std::endl;
			if((ui::mouse.x - offset.x) >= mouseStart.x){
				std::cout << "Thing" << std::endl;
				thing = (ui::mouse.x - offset.x - mouseStart.x)/80;
				std::cout << "Highlight" << std::endl;
				highlight=sel+thing;
				std::cout << "Highlight Check" << std::endl;
				if(highlight>numSlot-1)highlight=numSlot-1;
				std::cout << "Left Click" << std::endl;
				if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)){
					sel = highlight;
					mouseSel=false;
					invHover=false;
					selected = true;
				}
			}
			if((ui::mouse.x - offset.x) < mouseStart.x){
				thing = (mouseStart.x - (ui::mouse.x - offset.x))/80;
				if((int)sel-(int)thing<0)highlight=0;
				else highlight=sel-thing;
				if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)){
					sel = highlight;
					mouseSel=false;
					invHover=false;
					selected = true;
				}
			}
		}
		std::cout << "Rays" << std::endl;
		for(auto &r : iray){
			std::cout << "Setting angle" << std::endl;
			angle=180-(angleB*a) - angleB/2.0f;
			std::cout << "Currcourd" << std::endl;
			curCoord[a].x += float(range) * cos(angle*PI/180);
			curCoord[a].y += float(range) * sin(angle*PI/180);
			std::cout << "Ray.end" << std::endl;
			r.end = curCoord[a];

			std::cout << "Draw" << std::endl;
			glColor4f(0.0f, 0.0f, 0.0f, a == highlight ? 0.5f : 0.1f);
 			glBegin(GL_QUADS);
				glVertex2i(r.end.x-(itemWide/2),	r.end.y-(itemWide/2));
				glVertex2i(r.end.x+(itemWide/2),	r.end.y-(itemWide/2));
				glVertex2i(r.end.x+(itemWide/2),	r.end.y+(itemWide/2));
				glVertex2i(r.end.x-(itemWide/2),	r.end.y+(itemWide/2));
			glEnd();

			std::cout << "Draw items" << std::endl;
			if(!items.empty() && a < items.size() && items[a].count){
				std::cout << "Jamie" << std::endl;
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, itemtex[items[a].id]);
				glColor4f(1.0f, 1.0f, 1.0f, a == highlight ? 0.8f : 0.2f);
				std::cout << "Done Binding" << std::endl;
				glBegin(GL_QUADS);
				std::cout << "jdjdjd" << std::endl;
					if(itemMap[items[a].id]->height > itemMap[items[a].id]->width){
						std::cout << "map" << std::endl;
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
				std::cout << "Adding a" << std::endl;
				a++;
			}
		}
		ui::putStringCentered(player->loc.x+player->width/2, player->loc.y + range*.75,itemMap[items[highlight].id]->name.c_str());
	}

	if(!items.empty() && items.size() > sel && items[sel].count)
		itemDraw(player,items[sel].id);
}

void itemDraw(Player *p,uint id){
	itemLoc.y = p->loc.y+(p->height/3);
	itemLoc.x = p->left?p->loc.x:p->loc.x+p->width;
	glPushMatrix();

	if(!id)return;

	if(itemMap[id]->type == "Sword"){
		if(p->left){
			if(hangle < 15){
				hangle=15.0f;
				p->inv->usingi = false;
			}
		}else{
			if(hangle > -15){
				hangle=-15.0f;
				p->inv->usingi = false;
			}
		}
	}else hangle = 0.0f;
	if ( p->inv->usingi )
		p->inv->useItem();

	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "sampler"), 0);
	glTranslatef(itemLoc.x,itemLoc.y,0);
	glRotatef(hangle, 0.0f, 0.0f, 1.0f);
	glTranslatef(-itemLoc.x,-itemLoc.y,0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,itemtex[id]);
	glColor4ub(255,255,255,255);
	glBegin(GL_QUADS);
		glTexCoord2i(0,1);glVertex2f(itemLoc.x,					  itemLoc.y);
		glTexCoord2i(1,1);glVertex2f(itemLoc.x+itemMap[id]->width,itemLoc.y);
		glTexCoord2i(1,0);glVertex2f(itemLoc.x+itemMap[id]->width,itemLoc.y+itemMap[id]->height);
		glTexCoord2i(0,0);glVertex2f(itemLoc.x,					  itemLoc.y+itemMap[id]->height);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glTranslatef(player->loc.x*2,0,0);
	glPopMatrix();
	glUseProgram(0);
}

int Inventory::useItem(void){
	static bool up = false;
	if(!invHover){

		if(itemMap[items[sel].id]->type == "Sword"){

			if(swing){
				if(!player->left){
					if(hangle==-15){up=true;Mix_PlayChannel(2,swordSwing,0);}
					if(up)hangle-=.75*deltaTime;
					if(hangle<=-90)hangle=-14;
				}else{
					if(hangle==15){up=true;Mix_PlayChannel(2,swordSwing,0);}
					if(up)hangle+=.75*deltaTime;
					if(hangle>=90)hangle=14;
				}
			}else if(!swing){
				swing=true;
				Mix_PlayChannel(2,swordSwing,0);
			}
		}else if(itemMap[items[sel].id]->type == "Cooked Food"){
			player->health += itemMap[items[sel].id]->attribValue;
			usingi = false;
		}
	}
	return 0;
}

bool Inventory::detectCollision(vec2 one, vec2 two){
	(void)one;
	(void)two;
	float xc, yc;
	float i = 0.0f;

	if(items.empty() || !items[sel].count)
		return false;
	if(itemMap[items[sel].id]->type == "Sword"){
		std::cout<<"Collision???"<<std::endl;
		while(i<itemMap[items[sel].id]->height){
			xc = itemLoc.x; yc = itemLoc.y;
			xc += float(i) * cos((hangle+90)*PI/180);
			yc += float(i) * sin((hangle+90)*PI/180);

			if(xc >= one.x && xc <= two.x){
				if(yc >= one.y && yc <= two.y){
					return true;
				}
			}

			i+=HLINE;
		}
	}
	return false;
}
