#ifndef INVENTORY_H
#define INVENTORY_H

#include <common.h>
#include <string.h>

#define DEBUG

/*#define ID 			Item(
#define NAME 		,
#define TYPE 		,
#define WIDTH 		,
#define HEIGHT 		,
#define STACKSIZE 	,
#define TEX 		,
#define ENI 		),
#define STOP		)*/

/*
 * A list of all item IDs.
*/

/*#define ITEM_COUNT 5

enum ITEM_ID {
	DEBUG_ITEM = 0,
	TEST_ITEM,
	PLAYER_BAG,
	FLASHLIGHT,
	SWORD_WOOD
};

enum ITEM_TYPE {
	TOOL = 1,
	SWORD,
	RANGED,
	EQUIP,
	FOOD
};*/

class Item{
protected:
public:
	//ITEM_ID id;				// ID of the item
	//ITEM_TYPE type;			// What category the item falls under
	
	//char *name;
	//char *type;
	
	std::string name,type;
	
	float width;
	float height;
	int   maxStackSize;
	
	std::string texloc;
	Texturec *tex;

	//Item(ITEM_ID i, const char *n, ITEM_TYPE t, float w, float h, int m, const char *tl);
	GLuint rtex(){
		return tex->image[0];
	}
};

struct item_t{
	uint count;
	uint/*ITEM_ID*/ id;
} __attribute__((packed));

class Inventory {
private:

	std::vector<item_t> items;

	unsigned int size;		// Size of 'item' array
	//item_t *inv;
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
	
	int addItem(std::string name,uint count);
	int takeItem(std::string name,uint count);
	
	int useItem(void);
	bool detectCollision(vec2,vec2);
	
	void setSelection(unsigned int s);
	
	void draw(void);	// Draws a text list of items in this inventory (should only be called for the player for now)
};

void initInventorySprites(void);
void destroyInventory(void);

const char *getItemTexturePath(std::string name);
float getItemWidth(std::string name);
float getItemHeight(std::string name);

#endif // INVENTORY_H
