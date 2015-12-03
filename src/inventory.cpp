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
	for(int i = 0;i<ITEM_COUNT;i++){
		itemtex[i]=Texture::loadTexture(getItemTexturePath((ITEM_ID)i));
	}
}

char *getItemTexturePath(ITEM_ID id){
	return item[id].textureLoc;
}

Item::Item(ITEM_ID i, const char *n, ITEM_TYPE t, float w, float h, int m, const char *tl){
	id = i;
	type = t;
	width = w;
	height = h;
	maxStackSize = m;
	count = 0;

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
	std::cout << id << "," << inv[os].id << std::endl;
	for(int i = 0; i < size; i++){
		if(id == inv[i].id){
			inv[i].count += count;
			break;
		}else if(id ){
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
	unsigned int i;
	for(i=0;i<size;i++){
		if(inv[i].id==id){
#ifdef DEBUG
			DEBUG_printf("Took %u of player's %s(s).\n",count,item[i].name);
#endif // DEBUG
			inv[i].count-=count;
			if(item[i].count<0)
				return item[i].count*-1;
			return 0;
		}
	}
	return -1;
}

void Inventory::draw(void){
	unsigned int i=0;
	static unsigned int lop = 0;
	float y,xoff;
	static int numSlot = 7;
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
				ui::putText(r.end.x-(itemWide/2)+(itemWide*.85),r.end.y-(itemWide/1.75),"%d",inv[a].count);
			}
			a++;
		}
	}else if(invHover){
		static int highlight = 0;
		std::cout << ui::mouse.x << "," << mouseStart.x  << "," << mouseSel << std::endl;
		if(!mouseSel){
			mouseStart.x = ui::mouse.x;
			highlight = sel;
			mouseSel=true;
		}else{
			if(ui::mouse.x >= mouseStart.x){
				highlight = (ui::mouse.x - mouseStart.x)/45;
				if(highlight>numSlot)highlight=numSlot;
				if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)){
					sel = highlight;
					mouseSel=false;
					invHover=false;
					selected = true;
				}
			}
			if(ui::mouse.x < mouseStart.x){
				highlight = (mouseStart.x - ui::mouse.x)/45;
				if(highlight<0)highlight=0;
				if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)){
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

			glColor4f(0.0f, 0.0f, 0.0f, 0.1f);
 			glBegin(GL_QUADS);
				glVertex2i(r.end.x-(itemWide/2),			r.end.y-(itemWide/2));
				glVertex2i(r.end.x-(itemWide/2)+itemWide,	r.end.y-(itemWide/2));
				glVertex2i(r.end.x-(itemWide/2)+itemWide,	r.end.y-(itemWide/2)+itemWide);
				glVertex2i(r.end.x-(itemWide/2),			r.end.y-(itemWide/2)+itemWide);
			glEnd();

			if(inv[a].count > 0){
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, itemtex[inv[a].id]);			
				glColor4f(1.0f, 1.0f, 1.0f, a == highlight ? 0.4f : 0.2f);
	 			glBegin(GL_QUADS);
					glTexCoord2i(0,1);glVertex2i(r.end.x-(itemWide/2),			r.end.y-(itemWide/2));
					glTexCoord2i(1,1);glVertex2i(r.end.x-(itemWide/2)+itemWide,	r.end.y-(itemWide/2));
					glTexCoord2i(1,0);glVertex2i(r.end.x-(itemWide/2)+itemWide,	r.end.y-(itemWide/2)+itemWide);
					glTexCoord2i(0,0);glVertex2i(r.end.x-(itemWide/2),			r.end.y-(itemWide/2)+itemWide);
				glEnd();
				glDisable(GL_TEXTURE_2D);
				ui::putText(r.end.x-(itemWide/2)+(itemWide*.85),r.end.y-(itemWide/1.75),"%d",inv[a].count);
			}
			a++;
		}
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
	switch(id){
	case FLASHLIGHT:
		player->light ^= true;
		break;
	default:
		//ui::dialogBox(item[id].name,NULL,"You cannot use this item.");
		break;
	}
	return 0;
}

