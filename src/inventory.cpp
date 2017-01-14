#include <inventory.hpp>

#include <common.hpp>
#include <events.hpp>
#include <texture.hpp>
#include <render.hpp>

#include <deque>

#include <tinyxml2.h>
using namespace tinyxml2;

constexpr const char* itemsPath = "config/items.xml";


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

static std::deque<Item> itemList;

void InventorySystem::configure(entityx::EventManager &ev)
{
    ev.subscribe<KeyDownEvent>(*this);
}

void InventorySystem::loadItems(void) {
	XMLDocument doc;
	doc.LoadFile(itemsPath);

	auto item = doc.FirstChildElement("item");
	if (item == nullptr)
		UserError("No items found");

	do {
		itemList.emplace_back(item);
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
	Render::textShader.use();
	Render::textShader.enable();
	Colors::black.use();
	glUniform4f(Render::textShader.uniform[WU_tex_color], 1, 1, 1, .8);

	// calculate positions
	items.front().loc = vec2(offset.x - 35 * items.size(), offset.y - game::SCREEN_HEIGHT / 2);
	for (unsigned int i = 1; i < items.size(); i++)
		items[i].loc = vec2(items[i - 1].loc.x + 70, items[i - 1].loc.y);

	// draw items
	for (const auto& i : items) {
		glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 0, Colors::texCoord);
		vec2 end = i.loc + 64;
		GLfloat coords[18] = {
			i.loc.x, i.loc.y, -9, end.x, i.loc.y, -9, end.x, end.y, -9,
			end.x, end.y, -9, i.loc.x, end.y, -9, i.loc.x, i.loc.y, -9
		};
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 0, coords);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	glUniform4f(Render::textShader.uniform[WU_tex_color], 1, 1, 1, 1);
	Render::textShader.disable();
	Render::textShader.unuse();
}

void InventorySystem::receive(const KeyDownEvent &kde)
{
    (void)kde;
}
