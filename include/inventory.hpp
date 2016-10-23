#ifndef INVENTORY_HPP_
#define INVENTORY_HPP_

#include <entityx/entityx.h>

#include <components.hpp>
#include <events.hpp>

struct Item {
    GLuint icon;
};

using InventoryEntry = std::pair<Item, unsigned int>;

class InventorySystem : public entityx::System<InventorySystem>, public entityx::Receiver<InventorySystem> {
private:
    entityx::Entity currentItemEntity;

    std::vector<InventoryEntry> items;
    unsigned int maxItemCount;

public:
    InventorySystem(unsigned int mic = 1)
        : maxItemCount(mic) {}

    void configure(entityx::EventManager &ev);

    void loadIcons(void);

    void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;

    void receive(const KeyDownEvent &kde);
};

#endif // INVENTORY_HPP_
