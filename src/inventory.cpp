#include <inventory.h>
#include <entities.h>
#include <ui.h>

#define ITEM_COUNT 5	// Total number of items that actually exist

extern Player *player;
extern GLuint invUI;
static float hangle = 0.0f;
static bool up = true;
static float xc,yc;
static vec2 itemLoc;

static const Item item[ITEM_COUNT]= {
	#include "../config/items.h"
};

static GLuint itemtex[ITEM_COUNT];
void itemDraw(Player *p,ITEM_ID id, ITEM_TYPE type);

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
		if(inv[i].id == id){
			inv[i].count += count;
			return 0;
		}
	}
	inv[os].id = id;
	inv[os].count += count;
	os++;

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

			glColor4f(0.0f, 0.0f, 0.0f, ((float)dfp[a]/(float)range)*0.5f);
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
				ui::putText(r.end.x-(itemWide/2),r.end.y-(itemWide*.9),"%s",item[inv[a].id].name);
				ui::putText(r.end.x-(itemWide/2)+(itemWide*.85),r.end.y-(itemWide/2),"%d",inv[a].count);
			}
			a++;
			if(sel==a){
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
	if(inv[sel].count)itemDraw(player,inv[sel].id,item[inv[sel].id].type);
	lop++;
}

void itemDraw(Player *p,ITEM_ID id,ITEM_TYPE type){
	itemLoc.y = p->loc.y+(p->height/3);
	itemLoc.x = p->left?p->loc.x:p->loc.x+p->width;
	glPushMatrix();
	if(!id)return;
	switch(type){
	case SWORD:
		if(p->left){
			if(hangle < 15){
				hangle=15.0f;
				p->inv->usingi = false;
				up = false;
			}
		}else{
			if(hangle > -15){
				hangle=-15.0f;
				p->inv->usingi = false;
				up = false;
			}
		}
		break;
	default:
		hangle = 0.0f;
	}

	glTranslatef(itemLoc.x,itemLoc.y,0);
	glRotatef(hangle, 0.0f, 0.0f, 1.0f);
	glTranslatef(-itemLoc.x,-itemLoc.y,0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,itemtex[id]);
	glColor4ub(255,255,255,255);
	glBegin(GL_QUADS);
		glTexCoord2i(0,1);glVertex2f(itemLoc.x,				 itemLoc.y);
		glTexCoord2i(1,1);glVertex2f(itemLoc.x+item[id].width,itemLoc.y);
		glTexCoord2i(1,0);glVertex2f(itemLoc.x+item[id].width,itemLoc.y+item[id].height);
		glTexCoord2i(0,0);glVertex2f(itemLoc.x,				 itemLoc.y+item[id].height);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glTranslatef(player->loc.x*2,0,0);
	glPopMatrix();
}

int Inventory::useItem(void){
	ITEM_TYPE type = item[inv[sel].id].type;
	if(!invHover){
		switch(type){
		case SWORD:
		if(!player->left){
			if(hangle==-15)up=true;
			if(up)hangle-=15;
			if(hangle<=-90)hangle=-14;
		}else{
			if(hangle==15)up=true;
			if(up)hangle+=15;
			if(hangle>=90)hangle=14;
		}
			break;
		default:
			break;
		}
	}
	return 0;
}

bool Inventory::detectCollision(vec2 one, vec2 two){
	float i = 0.0f;
	if(item[inv[sel].id].type == SWORD){
		while(i<item[inv[sel].id].height){
			xc = itemLoc.x; yc = itemLoc.y;
			xc += float(i) * cos((hangle+90)*PI/180);
			yc += float(i) * sin((hangle+90)*PI/180);

			/*glColor4f(1.0f,1.0f,1.0f,1.0f);
			glBegin(GL_LINES);
				glVertex2f(player->loc.x,player->loc.y+player->height/3);
				glVertex2f(xc,yc);
			glEnd();*/

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

