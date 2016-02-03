#ifndef INVENTORY_H
#define INVENTORY_H

#include <common.h>
#include <string.h>

#include <Texture.h>

#define DEBUG

class Item{
public:
	std::string name,type;
	
	float width;
	float height;
	int   maxStackSize;
	
	std::string texloc;
	Texturec *tex;

	GLuint rtex(){
		return tex->image[0];
	}
};

struct item_t{
	uint count;
	uint id;
} __attribute__((packed));

class Inventory {
private:
	std::vector<item_t> items;
	unsigned int size;
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
