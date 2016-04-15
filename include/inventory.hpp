#ifndef INVENTORY_H
#define INVENTORY_H

#include <common.hpp>
#include <string.h>

#include <texture.hpp>

#define DEBUG

class Item{
public:
	std::string name, type;

	float width;
	float height;
	int   maxStackSize;
	float attribValue;

	std::string texloc;
	Texturec *tex;

	GLuint rtex()
	{
		return tex->image[0];
	}
};

class Currency{
public:
	std::string name;

	float width;
	float height;

	std::string texloc;
	Texturec *tex;

	float value;

	GLuint rtex()
	{
		return tex->image[0];
	}
};

struct item_t{
	uint count;
	uint id;
} __attribute__((packed));

class Inventory {
private:
	unsigned int size; //how many slots our inventory has
	unsigned int sel; //what item is currently selected
	int os = 0;
public:
	std::vector<item_t> items;

	bool invOpen = false; //is the inventory open
	bool invOpening = false; //is the opening animation playing
	bool invHover = false; //are we using the fancy hover inventory
	bool selected = false; //used in hover inventory to show which item has been selected
	bool mouseSel = false; //the location of the temperary selection for the hover inv
	bool usingi = false; //bool used to tell if inventory owner is using selected item

	Inventory(unsigned int s);	// Creates an inventory of size 's'
	~Inventory(void);			// Free's allocated memory

	int addItem(std::string name,uint count);
	int takeItem(std::string name,uint count);
	int hasItem(std::string name);

	int useItem(void);
	bool detectCollision(vec2,vec2);

	void setSelection(unsigned int s);
	void setSelectionUp();
	void setSelectionDown();

	void draw(void);	// Draws a text list of items in this inventory (should only be called for the player for now)
};

void initInventorySprites(void);
void destroyInventory(void);

const char *getItemTexturePath(std::string name);
GLuint getItemTexture(std::string name);
float getItemWidth(std::string name);
float getItemHeight(std::string name);

#endif // INVENTORY_H
