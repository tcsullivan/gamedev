#include <inventory.h>
#include <entities.h>
#include <ui.h>

#define ITEM_COUNT 5	// Total number of items that actually exist

extern Player *player;
extern GLuint invUI;

static const Item item[ITEM_COUNT]= {
	#include "../config/items.h"
};

static GLuint itemtex[ITEM_COUNT];

void itemDraw(Player *p,ITEM_ID id);

void initInventorySprites(void){
	unsigned int i;
	for(i = 0;i < ITEM_COUNT;i++){
		itemtex[i] = Texture::loadTexture(getItemTexturePath((ITEM_ID)i));
	}
}

char *getItemTexturePath(ITEM_ID id){
	return item[id].textureLoc;
}

int getItemWidth(ITEM_ID id){
	return item[id].width;
}

int getItemHeight(ITEM_ID id){
	return item[id].height;
}

Item::Item(ITEM_ID i, const char *n, ITEM_TYPE t, float w, float h, int m, const char *tl){
	id = i;
	type = t;
	width = w;
	height = h;
	maxStackSize = m;

	name 		= new char[strlen(n)+1];
	textureLoc 	= new char[strlen(tl)+1];

	strcpy(name,n);
	strcpy(textureLoc,tl);

	//tex= new Texturec(1,textureLoc);
}

Inventory::Inventory(unsigned int s){
	sel=0;
	size=s;
	inv = new struct item_t[size];
	memset(inv,0,size*sizeof(struct item_t));
}

Inventory::~Inventory(void){
	delete[] inv;
}

void Inventory::setSelection(unsigned int s){
	sel=s;
}

int Inventory::addItem(ITEM_ID id,unsigned char count){
	//std::cout << id << "," << inv[os].id << std::endl;
	
	for(unsigned int i = 0; i < size; i++){
		if(id == inv[i].id){
			inv[i].count += count;
			break;
		}else{
			inv[os].id = id;
			inv[os].count = count;
			os++;
			break;
		}
	}

#ifdef DEBUG
	DEBUG_printf("Gave player %u more %s(s)(ID: %d).\n",count,item[id].name,item[id].id);
#endif // DEBUG

	return 0;
}

int Inventory::takeItem(ITEM_ID id,unsigned char count){
	for(unsigned int i = 0;i < size;i++){
		if(inv[i].id == id){
#ifdef DEBUG
			DEBUG_printf("Took %u of player's %s(s).\n",count,item[i].name);
#endif // DEBUG
			inv[i].count-=count;
			return 0;
		}
	}
	return -1;
}

void Inventory::draw(void){
	static unsigned int lop = 0;
	static unsigned int numSlot = 7;
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
		curCoord[a] = r.start;
		//dfp[a] = 0;
		a++;
	}a=0;
	if(invOpening){
		end = 0;
		for(auto &d : dfp){
			if(a != 0){
				if(dfp[a-1]>50)d+=1.65*deltaTime;
			}else{
				d += 1.65*deltaTime;
			}
			if(d >= range)
				d = range;
			a++;
		}a=0;
		if(end < numSlot)invOpen=true;
	}else if(!invOpening){
		for(auto &d : dfp){
			if(d > 0){
				d-=1.65*deltaTime;
			}else end++;
		}
		if(end >= numSlot)invOpen=false;
	}
	if(invOpen){
		for(auto &r : iray){
			angle=180-(angleB*a) - angleB/2.0f;
			curCoord[a].x += float((dfp[a]) * cos(angle*PI/180));
			curCoord[a].y += float((dfp[a]) * sin(angle*PI/180));
			r.end = curCoord[a];

			glColor4f(0.0f, 0.0f, 0.0f, ((float)dfp[a]/(float)range)*0.4f);
 			glBegin(GL_QUADS);
				glVertex2i(r.end.x-(itemWide/2),			r.end.y-(itemWide/2));
				glVertex2i(r.end.x-(itemWide/2)+itemWide,	r.end.y-(itemWide/2));
				glVertex2i(r.end.x-(itemWide/2)+itemWide,	r.end.y-(itemWide/2)+itemWide);
				glVertex2i(r.end.x-(itemWide/2),			r.end.y-(itemWide/2)+itemWide);
			glEnd();

			if(inv[a].count > 0){
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, itemtex[inv[a].id]);			
				glColor4f(1.0f, 1.0f, 1.0f, ((float)dfp[a]/(float)range)*0.8f);
	 			glBegin(GL_QUADS);
					glTexCoord2i(0,1);glVertex2i(r.end.x-(itemWide/2),			r.end.y-(itemWide/2));
					glTexCoord2i(1,1);glVertex2i(r.end.x-(itemWide/2)+itemWide,	r.end.y-(itemWide/2));
					glTexCoord2i(1,0);glVertex2i(r.end.x-(itemWide/2)+itemWide,	r.end.y-(itemWide/2)+itemWide);
					glTexCoord2i(0,0);glVertex2i(r.end.x-(itemWide/2),			r.end.y-(itemWide/2)+itemWide);
				glEnd();
				glDisable(GL_TEXTURE_2D);
				ui::putText(r.end.x-(itemWide/2),r.end.y-(itemWide*.9),"%s",item[inv[a].id].name);
				ui::putText(r.end.x-(itemWide/2)+(itemWide*.85),r.end.y-(itemWide/2),"%d",inv[a].count);
			}
			a++;
		}
	}else if(invHover){
		static unsigned int highlight = 0;
		static unsigned int thing = 0;

		if(!mouseSel){
			mouseStart.x = ui::mouse.x - offset.x;
			highlight = sel;
			thing = sel;
			mouseSel=true;
		}else{
			if((ui::mouse.x - offset.x) >= mouseStart.x){
				thing = ((ui::mouse.x - offset.x) - mouseStart.x)/80;
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

			if(inv[a].count > 0){
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, itemtex[inv[a].id]);			
				glColor4f(1.0f, 1.0f, 1.0f, a == highlight ? 0.8f : 0.2f);
	 			glBegin(GL_QUADS);
	 				if(item[inv[a].id].height > item[inv[a].id].width){
						glTexCoord2i(0,1);glVertex2i(r.end.x-((itemWide/2)*((float)item[inv[a].id].width/(float)item[inv[a].id].height)),	r.end.y-(itemWide/2));
						glTexCoord2i(1,1);glVertex2i(r.end.x+((itemWide/2)*((float)item[inv[a].id].width/(float)item[inv[a].id].height)),	r.end.y-(itemWide/2));
						glTexCoord2i(1,0);glVertex2i(r.end.x+((itemWide/2)*((float)item[inv[a].id].width/(float)item[inv[a].id].height)),	r.end.y+(itemWide/2));
						glTexCoord2i(0,0);glVertex2i(r.end.x-((itemWide/2)*((float)item[inv[a].id].width/(float)item[inv[a].id].height)),	r.end.y+(itemWide/2));
					}else{
						glTexCoord2i(0,1);glVertex2i(r.end.x-(itemWide/2),	r.end.y-(itemWide/2)*((float)item[inv[a].id].height/(float)item[inv[a].id].width));
						glTexCoord2i(1,1);glVertex2i(r.end.x+(itemWide/2),	r.end.y-(itemWide/2)*((float)item[inv[a].id].height/(float)item[inv[a].id].width));
						glTexCoord2i(1,0);glVertex2i(r.end.x+(itemWide/2),	r.end.y+(itemWide/2)*((float)item[inv[a].id].height/(float)item[inv[a].id].width));
						glTexCoord2i(0,0);glVertex2i(r.end.x-(itemWide/2),	r.end.y+(itemWide/2)*((float)item[inv[a].id].height/(float)item[inv[a].id].width));
					}
				glEnd();
				glDisable(GL_TEXTURE_2D);
				//ui::putText(r.end.x-(itemWide/2)+(itemWide*.85),r.end.y-(itemWide/1.75),"%d",inv[a].count);
			}
			a++;
		}
		if(inv[highlight].count > 0)ui::putStringCentered(player->loc.x+player->width/2, player->loc.y + range*.75,item[inv[highlight].id].name);
	}
	if(inv[sel].count)itemDraw(player,inv[sel].id);
	lop++;
}

void itemDraw(Player *p,ITEM_ID id){
	if(!id)return;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,itemtex[id]);
	glColor4ub(255,255,255,255);
	glBegin(GL_QUADS);
		glTexCoord2i(0,1);glVertex2f(p->loc.x,				 p->loc.y);
		glTexCoord2i(1,1);glVertex2f(p->loc.x+item[id].width,p->loc.y);
		glTexCoord2i(1,0);glVertex2f(p->loc.x+item[id].width,p->loc.y+item[id].height);
		glTexCoord2i(0,0);glVertex2f(p->loc.x,				 p->loc.y+item[id].height);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

int Inventory::useItem(void){
	ITEM_ID id = item[inv[sel].id].id;
	if(!invHover){
		switch(id){
		case FLASHLIGHT:
			player->light ^= true;
			break;
		default:
			//ui::dialogBox(item[id].name,NULL,"You cannot use this item.");
			break;
		}
	}
	return 0;
}

