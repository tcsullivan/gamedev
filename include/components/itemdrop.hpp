#ifndef COMPONENTS_ITEMDROP_HPP_
#define COMPONENTS_ITEMDROP_HPP_

#include <inventory.hpp>

struct ItemDrop {
	ItemDrop(InventoryEntry& ie)
		: item(ie) {}

	InventoryEntry item;
};

#endif // COMPONENTS_ITEMDROP_HPP_
