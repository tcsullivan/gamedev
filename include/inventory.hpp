/**
 * @file inventory.hpp
 * @brief Provides an inventory for the player.
 */
#ifndef INVENTORY_HPP_
#define INVENTORY_HPP_

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_mixer.h>

#include <string>
#include <vector>

#include <entityx/entityx.h>
#include <tinyxml2.h>
using namespace tinyxml2;

#include <events.hpp>
#include <texture.hpp>

/**
 * @struct Item
 * Contains the information neccessary for an item.
 */
struct Item {
	std::string name; /**< The name of the item */
	std::string type; /**< The type of the item */
	int value;        /**< The value/worth of the item */
	int stackSize;    /**< The stack size of the item */
	Texture sprite;   /**< The texture for the item (in inventory) */
	Mix_Chunk* sound; /**< The sound to play on item use */
	int cooldown;

	Item(void)
		: value(0), stackSize(1) {}

	/**
	 * Constructs an item from XML.
	 * @param the xml element (<item />)
	 */
	Item(XMLElement* e) {
		name = e->StrAttribute("name");
		type = e->StrAttribute("type");
		
		value = 0;
		e->QueryIntAttribute("value", &value);
		stackSize = 1;
		e->QueryIntAttribute("maxStackSize", &stackSize);
		
		sprite = Texture(e->StrAttribute("sprite"));

		if (e->Attribute("sound") != nullptr)
			sound = Mix_LoadWAV(e->Attribute("sound"));
		else
			sound = nullptr;

		cooldown = 250;
		e->QueryIntAttribute("cooldown", &cooldown);
	}

	~Item(void) {
		if (sound != nullptr)
			Mix_FreeChunk(sound);
	}
};

/**
 * @struct InventoryEntry
 * Describes a slot in the player's inventory.
 */
struct InventoryEntry {
	Item* item; /**< Pointer to info on what item this slot contains */
	int count;  /**< The quantity of this item stored here */

	vec2 loc;   /**< Used by render, to determine slot location on screen */

	InventoryEntry(void)
		: item(nullptr), count(0) {}
};

struct UseItemEvent {
	Item* item;
	vec2 curs;

	UseItemEvent(Item* i, vec2 c)
		: item(i), curs(c) {}
};

/**
 * @class InventorySystem
 * Handles the player's inventory system.
 */
class InventorySystem : public entityx::System<InventorySystem>, public entityx::Receiver<InventorySystem> {
private:
	constexpr static const char* itemsPath = "config/items.xml";
	constexpr static int entrySize = 70;
	constexpr static int hotbarSize = 4;
	constexpr static float inventoryZ = -5.0f;
	constexpr static unsigned int rowSize = 8;

	static std::unordered_map<std::string, Item> itemList;
	static std::vector<InventoryEntry> items;

	static vec2 hotStart;
	static vec2 hotEnd;
	static vec2 fullStart;
	static vec2 fullEnd;

	static int movingItem;
	static bool fullInventory;

	static void loadItems(void);
public:
	InventorySystem(int size = 20);

	void configure(entityx::EventManager &ev);
	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;
	void receive(const KeyDownEvent &kde);
	void receive(const MouseClickEvent &mce);
	void receive(const MouseReleaseEvent &mce);

	static void render(void);

	/**
	 * Adds 'count' 'name's to the inventory.
	 * @param name the name of the item
	 * @param count the quantity of the item to give
	 */
	static void add(const std::string& name, int count);

	/**
	 * Takes 'count' 'name's from the inventory.
	 * If the inventory does not contain enough of the item, no items are taken
	 * and false is returned.
	 * @param name the name of the item
	 * @param count the quantity of the item to take
	 * @return true if the operation could be completed
	 */
	static bool take(const std::string& name, int count);

	static inline Item getItem(const std::string& s)
	{ return itemList[s]; }
};

#endif // INVENTORY_HPP_
