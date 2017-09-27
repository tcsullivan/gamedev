#include <inventory.hpp>

#include <common.hpp>
#include <components.hpp>
#include <engine.hpp>
#include <error.hpp>
#include <fileio.hpp>
#include <font.hpp>
#include <player.hpp>
#include <render.hpp>
#include <ui.hpp>

#include <forward_list>
#include <iostream>
#include <fstream>
#include <unordered_map>

#include <tinyxml2.h>
using namespace tinyxml2;

extern vec2 offset;

std::unordered_map<std::string, Item>   InventorySystem::itemList;
std::unordered_map<std::string, Attack> InventorySystem::attackList;
std::vector<InventoryEntry>             InventorySystem::items;
vec2 InventorySystem::hotStart;
vec2 InventorySystem::hotEnd;
vec2 InventorySystem::fullStart;
vec2 InventorySystem::fullEnd;
int  InventorySystem::movingItem = -1;
bool InventorySystem::fullInventory = false;

inline bool isGoodEntry(const InventoryEntry& ie) {
	return (ie.item != nullptr) && (ie.count > 0);
}

InventorySystem::InventorySystem(int size)
{
	items.resize(size);
	loadItems();
}

void InventorySystem::configure(entityx::EventManager &ev)
{
	ev.subscribe<KeyDownEvent>(*this);
	ev.subscribe<MouseClickEvent>(*this);
	ev.subscribe<MouseReleaseEvent>(*this);
}

void InventorySystem::loadItems(void) {
	XMLDocument doc;
	doc.LoadFile(itemsPath);

	auto itm = doc.FirstChildElement("item");
	UserAssert(itm != nullptr, "No items found");

	while (itm != nullptr) {
		Item item;
		item.name = itm->StrAttribute("name");
		UserAssert(!item.name.empty(), "Item must have a name");
		item.type = itm->StrAttribute("type");
		UserAssert(!item.type.empty(), "Item must have a type");
		item.value = 0;
		itm->QueryIntAttribute("value", &item.value);
		item.stackSize = 1;
		itm->QueryIntAttribute("maxStackSize", &item.stackSize);
		item.sprite = Texture(itm->StrAttribute("sprite"));
		if (itm->Attribute("sound") != nullptr)
			item.sound = Mix_LoadWAV(itm->Attribute("sound"));
		else
			item.sound = nullptr;
		item.cooldown = 250;
		itm->QueryIntAttribute("cooldown", &item.cooldown);

		auto atk = itm->FirstChildElement("attack");
		if (atk != nullptr) {
			Attack attack;
			attack.power = 0;
			atk->QueryIntAttribute("power", &attack.power);
			attack.offset = atk->StrAttribute("offset");
			attack.range  = atk->StrAttribute("range");
			attack.vel    = atk->StrAttribute("velocity");
			attack.accel  = atk->StrAttribute("accel");
			if (atk->Attribute("effect") != nullptr)
				attack.effect.appendGIF(atk->StrAttribute("effect")); 
			attackList.emplace(item.name, attack);
		}

		itemList.emplace(item.name, item);
		itm = itm->NextSiblingElement("item");
	}
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

	hotStart = items[0].loc - 10;
	hotEnd = items[hotbarSize - 1].loc + vec2(entrySize + 4, entrySize + 10);
	ui::drawNiceBox(hotStart, hotEnd, inventoryZ);

	if (fullInventory) {
		vec2 start (offset.x - entrySize * rowSize / 2, offset.y + game::SCREEN_HEIGHT / 2 - 180);
		for (unsigned int i = hotbarSize; i < items.size(); i++) {
			items[i].loc = vec2(start.x + entrySize * ((i - hotbarSize) % rowSize), start.y);
			if ((i - hotbarSize) % rowSize == rowSize - 1)
				start.y -= entrySize;
		}

		fullStart = items[items.size() - rowSize].loc - 10;
		fullEnd = items[hotbarSize + rowSize - 1].loc + (entrySize + 4);
		ui::drawNiceBox(fullStart, fullEnd, inventoryZ);
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
		vec2 end = i.loc + entrySize - 6;
		GLfloat coords[] = {
			i.loc.x, i.loc.y, inventoryZ - 0.1, 0, 0,
			end.x,   i.loc.y, inventoryZ - 0.1, 0, 0,
			end.x,   end.y,   inventoryZ - 0.1, 0, 0,
			end.x,   end.y,   inventoryZ - 0.1, 0, 0,
			i.loc.x, end.y,   inventoryZ - 0.1, 0, 0,
			i.loc.x, i.loc.y, inventoryZ - 0.1, 0, 0,
		};

		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), coords);
		glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), coords + 3);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glUniform4f(Render::textShader.uniform[WU_tex_color], 1, 1, 1, 1);

		// draw the item
		if (isGoodEntry(i)) {
			i.item->sprite.use();

			auto& dim = i.item->sprite.getDim();
			vec2 truDim;
			if (dim.x >= dim.y)
				truDim.x = entrySize - 6, truDim.y = truDim.x * dim.y / dim.x;
			else
				truDim.y = entrySize - 6, truDim.x = truDim.y * dim.x / dim.y;

			vec2 loc (i.loc.x + ((entrySize - 6) / 2 - truDim.x / 2), i.loc.y);
			vec2 sta ((n == movingItem) ? ui::mouse - truDim / 2 : loc);
			vec2 end = (n == movingItem) ? ui::mouse + truDim / 2 : loc + truDim;

			GLfloat coords[] = {
				sta.x, sta.y, inventoryZ - 0.2, 0, 1,
				end.x, sta.y, inventoryZ - 0.2, 1, 1,
				end.x, end.y, inventoryZ - 0.2, 1, 0,
				end.x, end.y, inventoryZ - 0.2, 1, 0,
				sta.x, end.y, inventoryZ - 0.2, 0, 0,
				sta.x, sta.y, inventoryZ - 0.2, 0, 1,
			};

			glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), coords);
			glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), coords + 3);
			if (n == movingItem)
				glUniform4f(Render::textShader.uniform[WU_tex_color], .8, .8, 1, .8);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			FontSystem::setFontZ(inventoryZ - 0.3); // TODO fix z's
			UISystem::putString(i.loc, std::to_string(i.count));
			FontSystem::setFontZ(-6);
			glUniform4f(Render::textShader.uniform[WU_tex_color], 1, 1, 1, 1);
		}
	}

	if (isGoodEntry(items[0])) {
		Render::textShader.use();
		Render::textShader.enable();
		
		auto& i = items[0];
		i.item->sprite.use();

		auto pos = PlayerSystem::getPosition();
		vec2 sta (pos.x, pos.y);
		vec2 end (sta + (i.item->sprite.getDim() * game::HLINE));
		GLfloat coords[] = {
			sta.x, sta.y, -8, 0, 1,
			end.x, sta.y, -8, 1, 1,
			end.x, end.y, -8, 1, 0,
			end.x, end.y, -8, 1, 0,
			sta.x, end.y, -8, 0, 0,
			sta.x, sta.y, -8, 0, 1,
		};

		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), coords);
		glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), coords + 3);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	Render::textShader.disable();
	Render::textShader.unuse();
}

bool InventorySystem::receive(const MouseClickEvent &mce)
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
	} else if (mce.button == SDL_BUTTON_LEFT) {
		if ((mce.position > hotStart && mce.position < hotEnd) ||
			(fullInventory && mce.position > fullStart && mce.position < fullEnd)) {
			int end = fullInventory ? items.size() : hotbarSize;
			movingItem = -1;
			for (int i = 0;	i < end; i++) {
				if (!isGoodEntry(items[i]))
					continue;

				if (mce.position > items[i].loc && mce.position < items[i].loc + entrySize) {
					movingItem = i;
					break;
				}
			}
		} else if (isGoodEntry(items[0])) {
			auto attack = attackList.find(items[0].item->name); 
			if (attack == attackList.end())
				game::events.emit<UseItemEvent>(mce.position, items[0].item);
			else
				game::events.emit<UseItemEvent>(mce.position, items[0].item, &attack->second);
		}
	}
	return true;
}

bool InventorySystem::receive(const MouseReleaseEvent &mre)
{
	if (movingItem != -1) {
		int end = fullInventory ? items.size() : hotbarSize;
		for (int i = 0;	i < end; i++) {
			if (mre.position > items[i].loc && mre.position < items[i].loc + entrySize) {
				if (isGoodEntry(items[i])) {
					std::swap(items[movingItem], items[i]);
				} else {
					items[i] = items[movingItem];
					items[movingItem].item = nullptr;
					items[movingItem].count = 0;
				}

				movingItem = -1;
				return true;
			}
		}

		makeDrop(mre.position, items[movingItem]);
		items[movingItem].item = nullptr;
		items[movingItem].count = 0;
		movingItem = -1;
	}
	return true;
}

void InventorySystem::makeDrop(const vec2& p, InventoryEntry& ie)
{
	auto e = game::entities.create();
	e.assign<Position>(p.x, p.y);
	e.assign<Direction>(0, 0.1f);
	e.assign<ItemDrop>(ie);
	e.assign<Sprite>();
	e.component<Sprite>()->addSpriteSegment(
		SpriteData(ie.item->sprite), vec2(0, 0));
	auto dim = ie.item->sprite.getDim();
	e.assign<Solid>(HLINES(dim.x), HLINES(dim.y));
	e.assign<Visible>();
	e.assign<Physics>();
}

void InventorySystem::makeDrop(const vec2& p, const std::string& s, int c)
{
	auto item = getItem(s);
	auto e = game::entities.create();
	e.assign<Position>(p.x, p.y);
	e.assign<Direction>(0, 0.1f);
	InventoryEntry ie (item, c);
	e.assign<ItemDrop>(ie);
	e.assign<Sprite>();
	e.component<Sprite>()->addSpriteSegment(
		SpriteData(item->sprite), vec2(0, 0));
	auto dim = item->sprite.getDim();
	e.assign<Solid>(HLINES(dim.x), HLINES(dim.y));
	e.assign<Visible>();
	e.assign<Physics>();
}

bool InventorySystem::receive(const KeyDownEvent &kde)
{
    if (kde.keycode == SDLK_e)
		fullInventory ^= true;
	return true;
}

void InventorySystem::add(const std::string& name, int count, int slot)
{
	auto& ie = items[slot];

	if (isGoodEntry(ie)) {
		if (ie.item->name != name)
			return; // TODO behavior?
		ie.count += count;
	} else {
		ie.item = &itemList[name];
		ie.count = count;
	}

	if (ie.count > ie.item->stackSize) {
		int diff = ie.count - ie.item->stackSize;
		ie.count = ie.item->stackSize;
		add(name, diff);
	}
}

void InventorySystem::add(const std::string& name, int count)
{
	for (unsigned int i = 0; i < items.size(); i++) {
		if ((items[i].item != nullptr && items[i].item->name == name
			&& items[i].count < items[i].item->stackSize)
			|| items[i].count == 0) {
			add(name, count, static_cast<int>(i));
			break;
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
				return isGoodEntry(ie) && ie.item->name == name;
			});

		if (i >= items.end())
			break;

		toDelete.push_front(i);
		taken += i->count;
		next = i + 1;
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

bool InventorySystem::save(void)
{	
	std::ofstream s (game::config::xmlFolder + "inventory.dat");

	// signature?
	s << "831998\n";

	int idx = 0;
	for (const auto& i : items) {
		if (i.item != nullptr && i.count > 0)
			s << std::string(i.item->name) << '\n' << i.count << '\n' << idx << '\n';
		idx++;
	}
	
	// end of list?
	s.close();
	return true;
}

void InventorySystem::load(void)
{
	// attempt to load data
	std::ifstream sf (game::config::xmlFolder + "inventory.dat");
	if (sf.good()) {
		sf.close();
		auto lines = readFileA(game::config::xmlFolder + "inventory.dat");

		// check signature
		if (std::stoi(lines[0]) != 831998)
			UserError("Inventory save file signature is invalid... (delete it)");

		for (unsigned int i = 1; i < lines.size(); i += 3) {
			if (lines[i].size() > 0) {
				int count = std::stoi(lines[i + 1]);
				int slot = std::stoi(lines[i + 2]);
				if (count > 0)
					add(lines[i], count, slot);
			}
		}
	}
}
