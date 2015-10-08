#ifndef INVENTORY_H
#define INVENTORY_H

#include <cstdlib>

#define DEBUG

enum ITEM_ID {		// Contains item IDs for every item in the game, this is how items are stored. IDs are also used to lookup item strings
	TEST_ITEM = 1,	// A test item (duh)
	SWORD_ITEM	
};

struct item_t {				// Used to define entries in an entity's inventory
	short count;			// Quantity of the item in this slot
	ITEM_ID id;				// ID of the item
} __attribute__ ((packed));

class Inventory {
private:
	unsigned int size;		// Size of 'item' array
	struct item_t *item;	// An array of the items contained in this inventory.
public:
	Inventory(unsigned int s);	// Creates an inventory of size 's'
	~Inventory(void);			// Free's 'item'
	
	int addItem(ITEM_ID id,unsigned char count);	// Add 'count' items with an id of 'id' to the inventory
	int takeItem(ITEM_ID id,unsigned char count);	// Take 'count' items with an id of 'id' from the inventory
	
	void draw(void);	// Draws a text list of items in this inventory (should only be called for the player for now)
};

unsigned int initInventorySprites(void);	// Loads as many inventory textures as it can find, returns count

#endif // INVENTORY_H
