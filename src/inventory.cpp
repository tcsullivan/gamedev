#include <inventory.hpp>

#include <common.hpp>
#include <components.hpp>
#include <engine.hpp>
#include <error.hpp>
#include <render.hpp>
#include <ui.hpp>

#include <forward_list>
#include <unordered_map>

#include <tinyxml2.h>
using namespace tinyxml2;

extern vec2 offset;

static std::unordered_map<std::string, Item> itemList;
constexpr const char* itemsPath = "config/items.xml";

static bool fullInventory = false;

constexpr int          entrySize = 70;
constexpr int          hotbarSize = 4;
constexpr float        inventoryZ = -6.0f;
constexpr unsigned int rowSize = 8;

static int movingItem = -1;

void InventorySystem::configure(entityx::EventManager &ev)
{
    ev.subscribe<KeyDownEvent>(*this);
	ev.subscribe<MouseClickEvent>(*this);
	ev.subscribe<MouseReleaseEvent>(*this);
}

void InventorySystem::loadItems(void) {
	XMLDocument doc;
	doc.LoadFile(itemsPath);

	auto item = doc.FirstChildElement("item");
	if (item == nullptr)
		UserError("No items found");

	do {
		itemList.emplace(item->StrAttribute("name"), item);
		item = item->NextSiblingElement("item");
	} while (item != nullptr);
}

void InventorySystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
    (void)en;
    (void)ev;
    (void)dt;
}

void InventorySystem::render(void)
{
	int size = items.size();

	// calculate positions
	items.front().loc = vec2(offset.x - entrySize / 2 * hotbarSize, offset.y + game::SCREEN_HEIGHT / 2 - entrySize);
	for (unsigned int i = 1; i < hotbarSize; i++)
		items[i].loc = vec2(items[i - 1].loc.x + entrySize, items[i - 1].loc.y);

	ui::drawNiceBox(items[0].loc - 10, items[hotbarSize - 1].loc + vec2(entrySize + 4, entrySize + 10), inventoryZ);

	if (fullInventory) {
		vec2 start (offset.x - entrySize * rowSize / 2, offset.y + game::SCREEN_HEIGHT / 2 - 180);
		for (unsigned int i = hotbarSize; i < items.size(); i++) {
			items[i].loc = vec2(start.x + entrySize * ((i - hotbarSize) % rowSize), start.y);
			if ((i - hotbarSize) % rowSize == rowSize - 1)
				start.y -= entrySize;
		}

		ui::drawNiceBox(items[items.size() - rowSize].loc - 10, items[hotbarSize + rowSize - 1].loc + (entrySize + 4), inventoryZ);
	} else {
		size = hotbarSize;
	}

	// draw items
	for (int n = 0; n < size; n++) {
		auto &i = items[n];

		// draw the slot
		Render::textShader.use();
		Render::textShader.enable();

		Colors::black.use();
		glUniform4f(Render::textShader.uniform[WU_tex_color], 1, 1, 1, .6);
		glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 0, Colors::texCoord);
		vec2 end = i.loc + entrySize - 6;
		GLfloat coords[18] = {
			i.loc.x, i.loc.y, inventoryZ - 0.1, end.x, i.loc.y, inventoryZ - 0.1, end.x, end.y, inventoryZ - 0.1,
			end.x, end.y, inventoryZ - 0.1, i.loc.x, end.y, inventoryZ - 0.1, i.loc.x, i.loc.y, inventoryZ - 0.1
		};
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 0, coords);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glUniform4f(Render::textShader.uniform[WU_tex_color], 1, 1, 1, 1);

		// draw the item
		if (i.item != nullptr) {
			i.item->sprite.use();
			static const GLfloat tex[12] = {0,1,1,1,1,0,1,0,0,0,0,1};
			glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 0, tex);

			vec2 sta ((n == movingItem) ? ui::mouse - i.item->sprite.getDim() / 2 : i.loc);
			vec2 end = (n == movingItem) ? ui::mouse + i.item->sprite.getDim() / 2 : i.loc + i.item->sprite.getDim();
			GLfloat coords[18] = {
				sta.x, sta.y, inventoryZ - 0.2, end.x, sta.y, inventoryZ - 0.2, end.x, end.y, inventoryZ - 0.2,
				end.x, end.y, inventoryZ - 0.2, sta.x, end.y, inventoryZ - 0.2, sta.x, sta.y, inventoryZ - 0.2
			};
			glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 0, coords);
			if (n == movingItem)
				glUniform4f(Render::textShader.uniform[WU_tex_color], .8, .8, 1, .8);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			ui::setFontZ(-7.2); // TODO fix z's
			ui::putText(sta.x, sta.y, std::to_string(i.count).c_str());
			ui::setFontZ(-6);
			glUniform4f(Render::textShader.uniform[WU_tex_color], 1, 1, 1, 1);
		}
	}

	Render::textShader.disable();
	Render::textShader.unuse();
}

void InventorySystem::receive(const MouseClickEvent &mce)
{
	if (mce.button == SDL_BUTTON_RIGHT) {
		game::entities.each<ItemDrop>([&](entityx::Entity e, ItemDrop& id) {
			auto& posComp = *e.component<Position>();
			auto& dimComp = *e.component<Solid>();
			vec2 sta (posComp.x, posComp.y);
			vec2 end (sta.x + dimComp.width, sta.y + dimComp.height);

			if (mce.position > sta && mce.position < end) {
				add(id.item.item->name, id.item.count);
				e.destroy();
			}
		});
	} else {
		int end = fullInventory ? items.size() : hotbarSize;
		movingItem = -1;
		for (int i = 0;	i < end; i++) {
			if (mce.position > items[i].loc && mce.position < items[i].loc + entrySize) {
				movingItem = i;
				break;
			}
		}
	}
}

void InventorySystem::receive(const MouseReleaseEvent &mre)
{
	if (movingItem != -1) {
		int end = fullInventory ? items.size() : hotbarSize;
		for (int i = 0;	i < end; i++) {
			if (mre.position > items[i].loc && mre.position < items[i].loc + entrySize) {
				if (items[i].count > 0) {
					std::swap(items[movingItem], items[i]);
				} else {
					items[i] = items[movingItem];
					items[movingItem].item = nullptr;
					items[movingItem].count = 0;
				}

				movingItem = -1;
				return;
			}
		}

		auto e = game::entities.create();
		e.assign<Position>(mre.position.x, mre.position.y);
		e.assign<Direction>(0, 1);
		e.assign<ItemDrop>(items[movingItem]);
		e.assign<Sprite>();
		e.component<Sprite>()->addSpriteSegment(
			SpriteData(items[movingItem].item->sprite), vec2(0, 0));
		auto dim = items[movingItem].item->sprite.getDim();
		e.assign<Solid>(HLINES(dim.x), HLINES(dim.y));
		e.assign<Visible>();
		e.assign<Physics>();

		items[movingItem].item = nullptr;
		items[movingItem].count = 0;

		movingItem = -1;
	}
}

void InventorySystem::receive(const KeyDownEvent &kde)
{
    if (kde.keycode == SDLK_e) {
		fullInventory ^= true;
	}
}

void InventorySystem::add(const std::string& name, int count)
{
	auto i = std::find_if(items.begin(), items.end(),
		[&name](const InventoryEntry& ie) {
			// either matching item that isn't filled, or empty slow
			return (ie.item != nullptr && ie.item->name == name && ie.count < ie.item->stackSize) || ie.count == 0;
		});

	if (i != items.end()) {
		auto& ie = *i;

		// update the slot
		if (ie.item == nullptr) {
			ie.item = &itemList[name];
			ie.count = count;
		} else {
			ie.count += count;
		}

		// handle overflow
		if (ie.count > ie.item->stackSize) {
			int diff = ie.count - ie.item->stackSize;
			ie.count = ie.item->stackSize;
			add(name, diff);
		}
	}
}

bool InventorySystem::take(const std::string& name, int count)
{
	using InvIter = std::vector<InventoryEntry>::iterator;
	std::forward_list<InvIter> toDelete;
	InvIter next = items.begin();
	int taken = 0;

	while (taken < count) {
		auto i = std::find_if(next, items.end(),
			[&name](const InventoryEntry& ie) {
				return ie.item != nullptr && ie.item->name == name;
			});

		if (i == items.end())
			break;

		toDelete.push_front(i);
		taken += i->count;
	}	

	if (taken < count)
		return false;

	for (auto& ii : toDelete) {
		if (count > ii->count) {
			ii->item = nullptr;
			count -= ii->count;
			ii->count = 0;
		} else {
			ii->count -= count;
			if (ii->count == 0)
				ii->item = nullptr;
			break;
		}
	}

	return true;
}
