#include <inventory.h>
#include <ui.h>

const char *itemName[]={
	"\0",
	"Dank Maymay"
};

Inventory::Inventory(unsigned int s){
	size=s;
	item=(struct item_t *)calloc(size,sizeof(struct item_t));
}

Inventory::~Inventory(void){
	free(item);
}
	
int Inventory::addItem(ITEM_ID id,unsigned char count){
	unsigned int i;
	for(i=0;i<size;i++){
		if(item[i].id==id){
			item[i].count+=count;
			return 0;
		}else if(!item[i].count){
			item[i].id=id;
			item[i].count=count;
			return 0;
		}
	}
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
	float y=SCREEN_HEIGHT/2;
	ui::putText(player->loc.x-SCREEN_WIDTH/2,y,"Inventory:");
	while(item[i].count){
		y-=ui::fontSize*1.15;
		ui::putText(player->loc.x-SCREEN_WIDTH/2,y,"%d x %s",item[i].count,itemName[(unsigned)item[i].id]);
		i++;
	}
}
