#ifndef INVENTORY_H
#define INVENTORY_H

#include <common.h>
#include <string.h>

#define DEBUG

#define ID 			Item(
#define NAME 		,
#define TYPE 		,
#define WIDTH 		,
#define HEIGHT 		,
#define STACKSIZE 	,
#define TEX 		,
#define ENI 		),
#define STOP		)

/*
 * A list of all item IDs.
*/

enum ITEM_ID {
	DEBUG_ITEM = 0,
	TEST_ITEM = 1,
	PLAYER_BAG = 2,
	FLASHLIGHT = 3,
	SWORD_WOOD = 4
};

enum ITEM_TYPE{
	TOOL = 1,
	SWORD = 2,
	RANGED = 3,
	EQUIP = 4,
	FOOD = 5
};

class Item{
protected:
public:
	ITEM_ID id;				// ID of the item
	char *name;
	ITEM_TYPE type;			// What category the item falls under
	float width;
	float height;
	int maxStackSize;
	char* textureLoc;
	Texturec *tex;
	GLuint text;

	Item(ITEM_ID i, const char *n, ITEM_TYPE t, float w, float h, int m, const char *tl);
	GLuint rtex(){
		return tex->image[0];
	}
};

struct item_t{
	int count;
	ITEM_ID id;
} __attribute__((packed));

typedef struct {
	unsigned int size;
	int os;
	unsigned int sel;
} __attribute__ ((packed)) InventorySavePacket;

class Inventory {
private:
	unsigned int size;		// Size of 'item' array
	item_t *inv;
	int os = 0;
public:
	unsigned int sel;
	bool invOpen = false;
	bool invOpening = false;
	bool invHover = false;
	bool selected = false;
	bool mouseSel = false;
	bool usingi = false;

	Inventory(unsigned int s);	// Creates an inventory of size 's'
	~Inventory(void);			// Free's allocated memory
	
	int addItem(ITEM_ID id,unsigned char count);	// Add 'count' items with an id of 'id' to the inventory
	int takeItem(ITEM_ID id,unsigned char count);	// Take 'count' items with an id of 'id' from the inventory
	int useItem(void);
	bool detectCollision(vec2,vec2);
	
	void setSelection(unsigned int s);
	
	void draw(void);	// Draws a text list of items in this inventory (should only be called for the player for now)
	
	char *save(void){
		static InventorySavePacket *isp = new InventorySavePacket();
		isp->size = size;
		isp->os = os;
		isp->sel = sel;
		return (char *)isp;
	}
	void load(InventorySavePacket *isp){
		size = isp->size;
		os = isp->os;
		sel = isp->sel;
	}
};

void itemUse(void *p);
void initInventorySprites(void);
char *getItemTexturePath(ITEM_ID id);
int getItemWidth(ITEM_ID);
int getItemHeight(ITEM_ID);

#endif // INVENTORY_H
