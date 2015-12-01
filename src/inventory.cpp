#include <inventory.h>
#include <entities.h>
#include <ui.h>

#define ITEM_COUNT 5	// Total number of items that actually exist

extern Player *player;
extern GLuint invUI;

void itemDraw(Player *p,ITEM_ID id);

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

	tex= new Texturec(1,textureLoc);
}

Inventory::Inventory(unsigned int s){
	sel=0;
	size=s;
	inv = new struct item_t[size];
	memset(inv,0,size*sizeof(struct item_t));
	tossd=false;
}

Inventory::~Inventory(void){
	delete[] inv;
}

void Inventory::setSelection(unsigned int s){
	sel=s;
}

int Inventory::addItem(ITEM_ID id,unsigned char count){
	inv[os].id = id;
	inv[os].count = count;
	os++;


	#ifdef DEBUG
	DEBUG_printf("Gave player %u more %s(s)(%d).\n",count,item[id].name,item[id].id);
	#endif // DEBUG

	/*#ifdef DEBUG
	DEBUG_printf("Failed to add non-existant item with id %u.\n",id);
	#endif // DEBUG*/
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
	ui::putText(offset.x-SCREEN_WIDTH/2,480,"%d",sel);
	unsigned int i=0;
	static unsigned int lop = 0;
	float y,xoff;
	static int numSlot = 7;
	static std::vector<int>dfp(numSlot);
	static std::vector<Ray>iray(numSlot);
	static std::vector<vec2>curCoord(numSlot);
	static int range = 200;
	float angleB = (float)180/(float)numSlot;
	float angle = float(angleB/2.0f);
	unsigned int a = 0;
	unsigned int end = 0;
	for(auto &r : iray){
		r.start = player->loc;
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

			glColor4f(0.0f, 0.0f, 0.0f, ((float)dfp[a]/(float)range));
 			glBegin(GL_QUADS);
				glVertex2i(r.end.x,		r.end.y);
				glVertex2i(r.end.x+45,	r.end.y);
				glVertex2i(r.end.x+45,	r.end.y+45);
				glVertex2i(r.end.x,		r.end.y+45);
			glEnd();

			if(inv[a].count > 0){
				glBindTexture(GL_TEXTURE_2D, item[inv[a].id].text);			
				glColor4f(1.0f, 1.0f, 1.0f, (float)dfp[a]/(float)range);
	 			glBegin(GL_QUADS);
					glTexCoord2i(0,1);glVertex2i(r.end.x,		r.end.y);
					glTexCoord2i(1,1);glVertex2i(r.end.x+45,	r.end.y);
					glTexCoord2i(1,0);glVertex2i(r.end.x+45,	r.end.y+45);
					glTexCoord2i(0,0);glVertex2i(r.end.x,		r.end.y+45);
				glEnd();
			}
			a++;
		}
	}


	/*else if(!invOpen){
		for(auto &d : dfp){
			d = 0;
		}
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, invUI);
		glBegin(GL_QUADS);
			glTexCoord2i(0,1);glVertex2i(offset.x-SCREEN_WIDTH/2,			0);
			glTexCoord2i(1,1);glVertex2i(offset.x-SCREEN_WIDTH/2+261,		0);
			glTexCoord2i(1,0);glVertex2i(offset.x-SCREEN_WIDTH/2+261,	   57);
			glTexCoord2i(0,0);glVertex2i(offset.x-SCREEN_WIDTH/2,		   57);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		while(i<size && inv[i].count > 0 && i<5){
			y = 6;
			xoff = (offset.x - (SCREEN_WIDTH /2)) + (51*i) + 6;
			glEnable(GL_TEXTURE_2D);
			item[inv[i].id].tex->bind(0);
			if(sel==i)glColor3ub(255,0,255);
			else      glColor3ub(255,255,255);
			glBegin(GL_QUADS);
				glTexCoord2i(0,1);glVertex2i(xoff,		y);
				glTexCoord2i(1,1);glVertex2i(xoff+45,	y);
				glTexCoord2i(1,0);glVertex2i(xoff+45,	y+45);
				glTexCoord2i(0,0);glVertex2i(xoff,		y+45);
			glEnd();
			glDisable(GL_TEXTURE_2D);
			i++;
		}
	}*/
	
	if(inv[sel].count)itemDraw(player,inv[sel].id);
	lop++;
}

static vec2 item_coord = {0,0};
static vec2 item_velcd = {0,0};
static bool item_tossd = false;
static bool yes=false;

void itemDraw(Player *p,ITEM_ID id){
	static vec2 p1,p2;
	if(!id)return;
	glEnable(GL_TEXTURE_2D);
	item[id].tex->bind(0);
	if(!yes){
		p1 = {p->loc.x+p->width/2,
			  p->loc.y+p->width/2+HLINE*3};
		p2 = {(float)(p1.x+p->width*(p->left?-.5:.5)),
			  p->loc.y+HLINE*3};
	}
	if(p->inv->tossd) yes=true;
	glColor4ub(255,255,255,255);
	glBegin(GL_QUADS);
		glTexCoord2i(0,1);glVertex2f(item_coord.x+p->loc.x,						item_coord.y+p->loc.y);
		glTexCoord2i(1,1);glVertex2f(item_coord.x+item[id].width+p->loc.x,		item_coord.y+p->loc.y);
		glTexCoord2i(1,0);glVertex2f(item_coord.x+item[id].width+p->loc.x,		item_coord.y+item[id].height+p->loc.y);
		glTexCoord2i(0,0);glVertex2f(item_coord.x+p->loc.x,						item_coord.y+item[id].height+p->loc.y);
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

int Inventory::itemToss(void){
	if(!item_tossd && item[sel].count && item[sel].id){
		item_tossd = true;
		item_coord.x = HLINE;
		item_velcd.x = 0;
		item_velcd.y = 3;
		tossd = true;
		return 1;
	}else if(item_tossd){
		if(item_coord.y<0){
			memset(&item_coord,0,sizeof(vec2));
			memset(&item_velcd,0,sizeof(vec2));
			item_tossd = false;
			
			takeItem(item[sel].id,1);
			
			tossd = yes = false;
			
			return 0;
		}else{
			item_coord.x += item_velcd.x;
			item_coord.y += item_velcd.y;
			//item_velcd.x -= .005;
			item_velcd.y -= .1;
			return 1;
		}
	}
}
