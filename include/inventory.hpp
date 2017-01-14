#ifndef INVENTORY_HPP_
#define INVENTORY_HPP_

#include <entityx/entityx.h>

#include <components.hpp>
#include <events.hpp>

struct InventoryEntry {
	GLuint icon;
	std::string name;
	std::string type;

	int count;
	int max;
	vec2 loc;
};

class InventorySystem : public entityx::System<InventorySystem>, public entityx::Receiver<InventorySystem> {
private:
    entityx::Entity currentItemEntity;

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
};

#endif // INVENTORY_HPP_
