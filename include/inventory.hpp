#ifndef INVENTORY_HPP_
#define INVENTORY_HPP_

#include <entityx/entityx.h>

#include <components.hpp>
#include <events.hpp>

struct Item {
	std::string name;
	std::string type;
	int value;
	int stackSize;
	Texture sprite;

	Item(void)
		: value(0), stackSize(1) {}

	Item(XMLElement *e) {
		name = e->StrAttribute("name");
		type = e->StrAttribute("type");
		
		value = 0;
		e->QueryIntAttribute("value", &value);
		stackSize = 1;
		e->QueryIntAttribute("maxStackSize", &stackSize);
		
		sprite = Texture(e->StrAttribute("sprite"));
	}
};

struct InventoryEntry {
	Item* item;
	int count;
	vec2 loc;

	InventoryEntry(void)
		: item(nullptr), count(0) {}
};

class InventorySystem : public entityx::System<InventorySystem>, public entityx::Receiver<InventorySystem> {
private:
    std::vector<InventoryEntry> items;

public:
    InventorySystem(int size = 4) {
		items.resize(size);
		loadItems();
	}

    void configure(entityx::EventManager &ev);

    void loadItems(void);

    void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;

    void receive(const KeyDownEvent &kde);
	void render(void);

	void add(const std::string& name, int count);
};

#endif // INVENTORY_HPP_
