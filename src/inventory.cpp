#include <inventory.h>
#include <entities.h>
#include <ui.h>

#include <tinyxml2.h>
using namespace tinyxml2;

extern Player *player;
extern GLuint invUI;
static float hangle = 0.0f;
static bool swing = false;
//static float xc,yc;
static vec2 itemLoc;
Mix_Chunk* swordSwing;

static std::vector<Item *> itemMap;

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
			items.push_back((item_t){count,i});
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
		return -1;
	
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

static GLuint *itemtex;
void itemDraw(Player *p,uint id);

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

Texturec *getItemTexture(std::string name){
	for(auto &i : itemMap){
		if(i->name == name)
			return i->tex;
	}
	return NULL;
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

void Inventory::draw(void){
	static unsigned int lop = 0;
	const unsigned int numSlot = 7;
	static std::vector<int>dfp(numSlot);
	static std::vector<Ray>iray(numSlot);
	static std::vector<vec2>curCoord(numSlot);
	static int range = 200;
	static int itemWide = 45;
	float angleB = (float)180/(float)numSlot;
	float angle = float(angleB/2.0f);
	unsigned int a = 0;
	unsigned int end = 0;
	static vec2 mouseStart = {0,0};
	
	for(auto &r : iray){
		r.start.x = player->loc.x + (player->width/2);
		r.start.y = player->loc.y + (player->height/2);
		curCoord[a++] = r.start;
	}a=0;
	
	if(invOpening){		
		for(auto &d : dfp){
			if(!a || dfp[a - 1] > 50)
				d += 1.65 * deltaTime;
			if(d >= range)
				d = range;
			a++;
		}a=0;
		
		if(numSlot > 0)invOpen=true;
	}else{
		for(auto &d : dfp){
			if(d > 0){
				d -= 1.65 * deltaTime;
			}else end++;
		}
		if(end >= numSlot)
			invOpen = false;
	}
	
	/*
	 * 	a = 0
	 */
	
	if(invOpen){
		
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
				ui::putText(r.end.x-(itemWide/2),r.end.y-(itemWide*.9),"%s",itemMap[items[a].id]->name.c_str());
				ui::putText(r.end.x-(itemWide/2)+(itemWide*.85),r.end.y-(itemWide/2),"%d",items[a].count);
			}

			a++;
			
			if(sel == a - 1){
	 			glBegin(GL_LINES);
	 				glColor4f(1.0f, 0.0f, 0.0f, 0.0f);
					glVertex2i(r.start.x,r.start.y);
					glColor4f(1.0f, 0.0f, 0.0f, 0.8f);
					glVertex2i(r.end.x+20, r.end.y-20);
				glEnd();
			}
		}
	}else if(invHover){
		static unsigned int highlight = 0;
		static unsigned int thing = 0;

		std::cout<<"Inventory2???"<<std::endl;

		if(!mouseSel){
			mouseStart.x = ui::mouse.x - offset.x;
			highlight = sel;
			thing = sel;
			mouseSel=true;
		}else{
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
		for(auto &r : iray){
			angle=180-(angleB*a) - angleB/2.0f;
			curCoord[a].x += float(range) * cos(angle*PI/180);
			curCoord[a].y += float(range) * sin(angle*PI/180);
			r.end = curCoord[a];

			glColor4f(0.0f, 0.0f, 0.0f, a == highlight ? 0.5f : 0.1f);
 			glBegin(GL_QUADS);
				glVertex2i(r.end.x-(itemWide/2),	r.end.y-(itemWide/2));
				glVertex2i(r.end.x+(itemWide/2),	r.end.y-(itemWide/2));
				glVertex2i(r.end.x+(itemWide/2),	r.end.y+(itemWide/2));
				glVertex2i(r.end.x-(itemWide/2),	r.end.y+(itemWide/2));
			glEnd();

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

			a++;
		}
		ui::putStringCentered(player->loc.x+player->width/2, player->loc.y + range*.75,itemMap[items[highlight].id]->name.c_str());
	}
	
	if(!items.empty() && items.size() > sel && items[sel].count)
		itemDraw(player,items[sel].id);
	lop++;
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
		}
	}
	return 0;
}

bool Inventory::detectCollision(vec2 one, vec2 two){
	//float i = 0.0f;
	
	/*if(items.empty() || !items[sel].count)
		return false;
	if(itemMap[items[sel].id]->type == "Sword"){
		std::cout<<"Collision???"<<std::endl;
		while(i<itemMap[items[sel].id]->height){
			xc = itemLoc.x; yc = itemLoc.y;
			xc += float(i) * cos((hangle+90)*PI/180);
			yc += float(i) * sin((hangle+90)*PI/180);

			*glColor4f(1.0f,1.0f,1.0f,1.0f);
			glBegin(GL_LINES);
				glVertex2f(player->loc.x,player->loc.y+player->height/3);
				glVertex2f(xc,yc);
			glEnd();*

			if(xc >= one.x && xc <= two.x){
				if(yc >= one.y && yc <= two.y){
					return true;
				}
			}

			i+=HLINE;
		}
	}*/
	return !(one.x == two.y);
}

