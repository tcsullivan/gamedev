#include <inventory.hpp>

#include <common.hpp>
#include <events.hpp>
#include <texture.hpp>
#include <render.hpp>
#include <ui.hpp>

#include <unordered_map>

#include <tinyxml2.h>
using namespace tinyxml2;

constexpr const char* itemsPath = "config/items.xml";

static std::unordered_map<std::string, Item> itemList;

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
	// calculate positions
	items.front().loc = vec2(offset.x - 35 * items.size(), offset.y - game::SCREEN_HEIGHT / 2);
	for (unsigned int i = 1; i < items.size(); i++)
		items[i].loc = vec2(items[i - 1].loc.x + 70, items[i - 1].loc.y);

	// draw items
	for (const auto& i : items) {
		// draw the slot
		Render::textShader.use();
		Render::textShader.enable();

		Colors::black.use();
		glUniform4f(Render::textShader.uniform[WU_tex_color], 1, 1, 1, .8);
		glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 0, Colors::texCoord);
		vec2 end = i.loc + 64;
		GLfloat coords[18] = {
			i.loc.x, i.loc.y, -7, end.x, i.loc.y, -7, end.x, end.y, -7,
			end.x, end.y, -7, i.loc.x, end.y, -7, i.loc.x, i.loc.y, -7
		};
		glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 0, coords);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glUniform4f(Render::textShader.uniform[WU_tex_color], 1, 1, 1, 1);

		// draw the item
		if (i.item != nullptr) {
			i.item->sprite.use();
			static const GLfloat tex[12] = {0,1,1,1,1,0,1,0,0,0,0,1};
			glVertexAttribPointer(Render::textShader.tex, 2, GL_FLOAT, GL_FALSE, 0, tex);
			vec2 end = i.loc + i.item->sprite.getDim();
			GLfloat coords[18] = {
				i.loc.x, i.loc.y, -7.1, end.x, i.loc.y, -7.1, end.x, end.y, -7.1,
				end.x, end.y, -7.1, i.loc.x, end.y, -7.1, i.loc.x, i.loc.y, -7.1
			};
			glVertexAttribPointer(Render::textShader.coord, 3, GL_FLOAT, GL_FALSE, 0, coords);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			ui::setFontZ(-7.2);
			ui::putText(i.loc.x, i.loc.y, std::to_string(i.count).c_str());
			ui::setFontZ(-6);
		}
	}

	Render::textShader.disable();
	Render::textShader.unuse();
}

void InventorySystem::receive(const KeyDownEvent &kde)
{
    (void)kde;
}

void InventorySystem::add(const std::string& name, int count)
{
	for (auto& i : items) {
		if (i.count == 0) {
			i.item = &itemList[name];
			i.count = count;
			break;
		}
	}
}
