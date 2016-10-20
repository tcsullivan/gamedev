#ifndef INVENTORY_H
#define INVENTORY_H

#include <common.hpp>
#include <string.h>

#include <texture.hpp>
#include <events.hpp>

#define DEBUG

class Entity;

/**
 *	The base item class
 *	This stores the name, width, height, and texture(s)
 */
class Item {
private:
	bool beingUsed;

protected:
	std::vector<Entity*> interact;
	
public:

	// what we want to call each item
	std::string name;

	// how many pixel tall and white each thing is
	dim2 dim;

	// the total amount of this item each slot can have
	uint maxStackSize;

	// the array of textures for each frame of animation
	Texturec *tex;

	// how much the item is rotated in the hand
	float rotation = 0.0f;

	// return if the item is currently in use
	virtual bool inUse();

	// set the state of the item
	virtual void setUse(bool use);

	// add entities to the list of those being interacted
	virtual void addInteract(Entity* e);
	virtual void addInteract(std::vector<Entity*> e);

	/**
	 *	The function we use to call the child classes ability
	 * 	Note: Since this function is abstract, we HAVE to create one for each
	 *	child class/type of item.
	 */
	virtual int useItem()=0;

	virtual Item* clone()=0;

	// destructor
	virtual ~Item();
	// return the first texture for the item
	GLuint rtex();

	// return the nth texture for the item
	GLuint rtex(int n);
};

/**
 *	Class for blank items, we use this for items that do not have an ability
 *	Like Quest or Debug items
 */
class BaseItem : public Item {
public:
	// since the items don't have a use, we don't make one for it
	int useItem();

	BaseItem* clone();

	//~BaseItem(){}
};

/**
 *	Sword class. This is for swords, y'know. Pretty basic stuff
 */
class Sword : public Item {
//	can't touch this
private:
	/**
	 *	How much damage our sword will do
	 *	notice that it's private to avoid a change
	 */
	float damage;

	Ray hitbox;

//can touch this
public:
	/**
	 *	Lets us return the amount of damage the sword has
	 *	TODO takes into account enchants and/or buffs/nerfs
	 */
	//TODO move
	float getDamage();

	// set the damage of the sword
	void setDamage(float d);

	/**
	 *	handles the swinging of the sword
	 */
	//TODO move
	int useItem();

	Sword* clone();
};

class Arrow : public Item {
private:
	// same as sword
	float damage;
public:

	// see sword
	float getDamage();

	// sets amount of damage
	void setDamage(float d);

	// uses item
	int useItem();

	Arrow *clone();
};

/**
 *	Bow class. We use this for shooting bow and arrows
 */
class Bow : public Item {
private:
	// same as sword
	float damage;
public:
	// returns the amount of damage, see sword
	float getDamage();

	// sets the amount of damages
	void setDamage(float d);

	// handles shooting and arrow curving
	int useItem();

	Bow* clone();
};

/**
 *	Raw food class, this will be used for uncooked meats...
 *	TODO Eating this may cause health loss, salmonela, mad cow diese
 */
class RawFood : public Item {
private:
	// the amount of the health the food heals
	float health;

public:
	// since the health is private, we get how much it is here
	float getHealth();

	// TODO chance to hurt
	virtual int useItem();

	RawFood* clone();
};

/**
 *	Cooked/Naturale food class
 *	When this food is consumed, higher stats are gained than Raw Food and
 *	there is no chance of damage/de-buffs
 */
class Food : public RawFood {
private:
public:

	// consume food in hand, no chance for de-buff;
	int useItem();

	Food* clone();
};

class ItemLight : public Item {
private:
	// the color of the light
	Color color;
public:
	// turn on/off the light
	// TODO
	int useItem();

	ItemLight* clone();
};

/**
 *	Currency class. Well, it's used for currency
 */
class NewCurrency : public Item {
private:
	// how much the coin is "worth" so to say
	int value;
public:
	// get the value of the coin
	int getValue();

	// TODO maybe play a jingling noise
	// probably won't have a use
	int useItem();

	NewCurrency(){}
	~NewCurrency(){}
};

/***********************************************************************************
 *						OLD STUFF THAT NEEDS TO GET UPDATED				   		   *
 **********************************************************************************/

using InventorySlot = std::pair<Item *, unsigned int>;

class Inventory {
private:
	unsigned int size; //how many slots our inventory has
	unsigned int sel; //what item is currently selected
	int os = 0;
public:
	std::vector<InventorySlot> Items;

	bool invOpen = false; //is the inventory open
	bool invOpening = false; //is the opening animation playing
	bool invHover = false; //are we using the fancy hover inventory
	bool selected = false; //used in hover inventory to show which item has been selected
	bool mouseSel = false; //the location of the temperary selection for the hover inv
	bool usingi = false; //bool used to tell if inventory owner is using selected item

	Inventory(unsigned int s);	// Creates an inventory of size 's'
	~Inventory(void);			// Free's allocated memory

	int useCurrent();
	void currentAddInteract(Entity* e);
	void currentAddInteract(std::vector<Entity*> e);

	int addItem(std::string name,uint count);
	int takeItem(std::string name,uint count);
	int hasItem(std::string name);

	int useItem(void);
	bool detectCollision(vec2,vec2);

	void setSelection(unsigned int s);
	void setSelectionUp();
	void setSelectionDown();

	void draw(void);	// Draws a text list of items in this inventory (should only be called for the player for now)

	const Item* getCurrentItem(void);
};

void initInventorySprites(void);
void destroyInventory(void);

const char *getItemTexturePath(std::string name);
GLuint getItemTexture(std::string name);
float getItemWidth(std::string name);
float getItemHeight(std::string name);

#include <entityx/entityx.h>

class InventorySystem : public entityx::System<InventorySystem>, public entityx::Receiver<InventorySystem> {
public:
	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;

	void configure(entityx::EventManager &em);
	void receive(const MouseScrollEvent &mse);
};

#endif // INVENTORY_H
