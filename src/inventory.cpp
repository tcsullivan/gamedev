#include <inventory.h>
#include <entities.h>
#include <ui.h>

#define ITEM_COUNT 5	// Total number of items that actually exist

extern Player *player;
extern GLuint invUI;

static Item item[5]= {
	#include "../config/items.h"
};

GLuint *ITEM_TEX;

void itemDraw(Player *p,ITEM_ID id);

unsigned int initInventorySprites(void){
	unsigned int i,loadCount=0;
	ITEM_TEX=(GLuint *)calloc(ITEM_COUNT,sizeof(GLuint));
	for(i=0;i<ITEM_COUNT;i++){
		if((ITEM_TEX[i]=Texture::loadTexture(item[i].textureLoc)))loadCount++;
	}
#ifdef DEBUG
	DEBUG_printf("Loaded %u/%u item texture(s).\n",loadCount,ITEM_COUNT);
#endif // DEBUG
	return loadCount;
}

Inventory::Inventory(unsigned int s){
	sel=0;
	size=s;
	//item=(struct item_t *)calloc(size,sizeof(struct item_t));
	tossd=false;
}

Inventory::~Inventory(void){
	free(item);
}

void Inventory::setSelection(unsigned int s){
	sel=s;
}

int Inventory::addItem(ITEM_ID id,unsigned char count){
	unsigned int i;

	for(i=0;i<size;i++){
		if(item[i].id==id){
			item[i].count+=count;

			#ifdef DEBUG
			DEBUG_printf("Gave player %u more %s(s).\n",count,item[i].name);
			#endif // DEBUG

			return 0;
		}else if(!item[i].count){
			item[i].id=id;
			item[i].count=count;

			#ifdef DEBUG
			DEBUG_printf("Gave player %u %s(s).\n",count,item[i].name);
			#endif // DEBUG

			return 0;
		}
	}

#ifdef DEBUG
	DEBUG_printf("Failed to add non-existant item with id %u.\n",id);
#endif // DEBUG
	return -1;
}

int Inventory::takeItem(ITEM_ID id,unsigned char count){
	unsigned int i;
	for(i=0;i<size;i++){
		if(item[i].id==id){
#ifdef DEBUG
			DEBUG_printf("Took %u of player's %s(s).\n",count,item[i].name);
#endif // DEBUG
			item[i].count-=count;
			if(item[i].count<0)
				return item[i].count*-1;
			return 0;
		}
	}
	return -1;
}

void Inventory::draw(void){
	unsigned int i=0;
	float y=offset.y,xoff;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, invUI);
	glBegin(GL_QUADS);
		glTexCoord2i(0,1);glVertex2i(offset.x-SCREEN_WIDTH/2,			0);
		glTexCoord2i(1,1);glVertex2i(offset.x-SCREEN_WIDTH/2+261,		0);
		glTexCoord2i(1,0);glVertex2i(offset.x-SCREEN_WIDTH/2+261,	   57);
		glTexCoord2i(0,0);glVertex2i(offset.x-SCREEN_WIDTH/2,		   57);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	while(item[i].count){
		y-=HLINE*12;
		xoff=ui::putText(offset.x-SCREEN_WIDTH/2,y,"%d x ",item[i].count);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,ITEM_TEX[item[i].id]);
		if(sel==i)glColor3ub(255,0,255);
		else      glColor3ub(255,255,255);
		glBegin(GL_QUADS);
			glTexCoord2i(0,1);glVertex2i(xoff		  ,y);
			glTexCoord2i(1,1);glVertex2i(xoff+HLINE*10,y);
			glTexCoord2i(1,0);glVertex2i(xoff+HLINE*10,y+HLINE*10);
			glTexCoord2i(0,0);glVertex2i(xoff		  ,y+HLINE*10);
		glEnd();
		y-=ui::fontSize*1.15;
		ui::putText(offset.x-SCREEN_WIDTH/2,y,"%s",item[i].name);
		glDisable(GL_TEXTURE_2D);
		i++;
	}
	if(item[sel].count)itemDraw(player,item[sel].id);
}

static vec2 item_coord = {0,0};
static vec2 item_velcd = {0,0};
static bool item_tossd = false;
static bool yes=false;

void itemDraw(Player *p,ITEM_ID id){
	static vec2 p1,p2;
	if(!id)return;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,ITEM_TEX[id-1]);
	if(!yes){
		p1 = {p->loc.x+p->width/2,
			  p->loc.y+p->width/2+HLINE*3};
		p2 = {(float)(p1.x+p->width*(p->left?-.5:.5)),
			  p->loc.y+HLINE*3};
	}
	if(p->inv->tossd) yes=true;
	glBegin(GL_QUADS);
		glTexCoord2i(0,1);glVertex2f(item_coord.x+p->loc.x,						item_coord.y+p->loc.y);
		glTexCoord2i(1,1);glVertex2f(item_coord.x+item[sel].width+p->loc.x,		item_coord.y+p->loc.y);
		glTexCoord2i(1,0);glVertex2f(item_coord.x+item[sel].width+p->loc.x,		item_coord.y+item[sel].height+p->loc.y);
		glTexCoord2i(0,0);glVertex2f(item_coord.x+p->loc.x,						item_coord.y+item[sel].height+p->loc.y);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

int Inventory::useItem(void){
	ITEM_ID id = item[sel].id;
	switch(id){
	case SWORD_WOOD:

		break;
	default:
		ui::dialogBox(item[id].name,NULL,"You cannot use this item.");
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
