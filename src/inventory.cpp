#include <inventory.h>
#include <ui.h>

#define ITEM_COUNT 2	// Total number of items that actually exist

const char *itemName[]={
	"\0",
	"Dank Maymay",
	"Sword"
};

const char *ITEM_SPRITE[]={
	"\0",							// Null
	"assets/items/ITEM_TEST.png",	// Dank maymay
	"assets/items/ITEM_SWORD.png"
};

GLuint *ITEM_TEX;

unsigned int initInventorySprites(void){
	unsigned int i,loadCount=0;
	ITEM_TEX=(GLuint *)calloc(ITEM_COUNT,sizeof(GLuint));
	for(i=0;i<ITEM_COUNT;i++){
		if((ITEM_TEX[i]=loadTexture(ITEM_SPRITE[i+1])))loadCount++;
	}
#ifdef DEBUG
	DEBUG_printf("Loaded %u/%u item texture(s).\n",loadCount,ITEM_COUNT);
#endif // DEBUG
	return loadCount;
}

Inventory::Inventory(unsigned int s){
	sel=0;
	size=s;
	item=(struct item_t *)calloc(size,sizeof(struct item_t));
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
			DEBUG_printf("Gave player %u more %s(s).\n",count,itemName[i]);
#endif // DEBUG
			return 0;
		}else if(!item[i].count){
			item[i].id=id;
			item[i].count=count;
#ifdef DEBUG
			DEBUG_printf("Gave player %u %s(s).\n",count,itemName[i]);
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
			item[i].count-=count;
			if(item[i].count<0)
				return item[i].count*-1;
			return 0;
		}
	}
	return -1;
}

#include <entities.h>
extern Player *player;

void Inventory::draw(void){
	unsigned int i=0;
	float y=SCREEN_HEIGHT/2,xoff;
	ui::putText(player->loc.x-SCREEN_WIDTH/2,y,"Inventory:");
	while(item[i].count){
		y-=HLINE*12;
		xoff=ui::putText(player->loc.x-SCREEN_WIDTH/2,y,"%d x ",item[i].count);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,ITEM_TEX[item[i].id-1]);
		if(sel==i)glColor3ub(255,0,255);
		else      glColor3ub(255,255,255);
		glBegin(GL_QUADS);
			glTexCoord2i(0,1);glVertex2i(xoff		  ,y);
			glTexCoord2i(1,1);glVertex2i(xoff+HLINE*10,y);
			glTexCoord2i(1,0);glVertex2i(xoff+HLINE*10,y+HLINE*10);
			glTexCoord2i(0,0);glVertex2i(xoff		  ,y+HLINE*10);
		glEnd();
		y-=ui::fontSize*1.15;
		ui::putText(player->loc.x-SCREEN_WIDTH/2,y,"%s",itemName[(unsigned)item[i].id]);
		glDisable(GL_TEXTURE_2D);
		i++;
	}
}
