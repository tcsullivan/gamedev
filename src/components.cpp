#include <components.hpp>

#include <entityx/entityx.h>
#include <events.hpp>

#include <render.hpp>
#include <ui.hpp>
#include <engine.hpp>
#include <world.hpp>
#include <brice.hpp>

static std::vector<std::string> randomDialog (readFileA("assets/dialog_en-us"));

void MovementSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)ev;
	en.each<Position, Direction>([dt](entityx::Entity entity, Position &position, Direction &direction) {
		position.x += direction.x * dt;
		position.y += direction.y * dt;

		if (entity.has_component<Dialog>() && entity.component<Dialog>()->talking) {
			direction.x = 0;
		} else {
			if (entity.has_component<Sprite>()) {
				auto& fl = entity.component<Sprite>()->faceLeft;
				if (direction.x != 0)
					fl = (direction.x < 0);
			}

			if (entity.has_component<Wander>()) {
				auto& countdown = entity.component<Wander>()->countdown;

				if (countdown > 0) {
					countdown--;
				} else {
					countdown = 5000 + randGet() % 10 * 100;
					direction.x = (randGet() % 3 - 1) * 0.02f;
				}
			}
		}
	});
}

void PhysicsSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)ev;
	en.each<Direction, Physics>([dt](entityx::Entity entity, Direction &direction, Physics &physics) {
		(void)entity;
		// TODO GET GRAVITY FROM WORLD
		direction.y += physics.g * dt;
	});
}

GLuint RenderSystem::loadTexture(const std::string& file)
{
	loadTexString = file;
	loadTexResult = 0xFFFF;
	while (loadTexResult == 0xFFFF)
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	return loadTexResult;
}

void RenderSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)ev;

	if (!loadTexString.empty()) {
		loadTexResult = Texture::loadTexture(loadTexString);
		loadTexString.clear();
	}

	Render::worldShader.use();

	en.each<Visible, Sprite, Position>([dt](entityx::Entity entity, Visible &visible, Sprite &sprite, Position &pos) {
		(void)entity;
		// Verticies and shit
		GLfloat tex_coord[] = {0.0, 0.0,
							   1.0, 0.0,
							   1.0, 1.0,

							   1.0, 1.0,
							   0.0, 1.0,
							   0.0, 0.0};

		GLfloat tex_coordL[] = {1.0, 0.0,
								0.0, 0.0,
								0.0, 1.0,

								0.0, 1.0,
								1.0, 1.0,
								1.0, 0.0};

		for (auto &S : sprite.sprite) {
			float width = HLINES(S.first.size.x);
			float height = HLINES(S.first.size.y);

			vec2 loc = vec2(pos.x + S.first.offset.x, pos.y + S.first.offset.y);

			GLfloat coords[] = {loc.x, 			loc.y, 			visible.z,
								loc.x + width, 	loc.y, 			visible.z,
								loc.x + width, 	loc.y + height, visible.z,

								loc.x + width, 	loc.y + height, visible.z,
								loc.x, 			loc.y + height, visible.z,
								loc.x, 			loc.y, 			visible.z};


			// make the entity hit flash red
			// TODO
			/*if (maxHitDuration-hitDuration) {
				float flashAmt = 1-(hitDuration/maxHitDuration);
				glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, flashAmt, flashAmt, 1.0);
			}*/
			glBindTexture(GL_TEXTURE_2D, S.first.pic);
			glUniform1i(Render::worldShader.uniform[WU_texture], 0);
			Render::worldShader.enable();

			glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 0, coords);
			if (sprite.faceLeft)
				glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 0 ,tex_coordL);
			else
				glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 0 ,tex_coord);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.0);
		}
	});

	Render::worldShader.disable();
	Render::worldShader.unuse();

	en.each<Visible, Position, Solid, Name>([](entityx::Entity e, Visible &v, Position &pos, Solid& dim, Name &name) {
		(void)e;
		(void)v;
		ui::setFontZ(-5.0);
		ui::putStringCentered(pos.x + dim.width / 2, pos.y - ui::fontSize - HLINES(0.5), name.name);
	});
}

void DialogSystem::configure(entityx::EventManager &ev)
{
	ev.subscribe<MouseClickEvent>(*this);
}

void DialogSystem::receive(const MouseClickEvent &mce)
{
	game::entities.each<Position, Solid, Dialog, Name>(
		[&](entityx::Entity e, Position &pos, Solid &dim, Dialog &d, Name &name) {
			(void)e;
			(void)d;

			if (((mce.position.x > pos.x) & (mce.position.x < pos.x + dim.width)) &&
			    ((mce.position.y > pos.y) & (mce.position.y < pos.y + dim.height))) {

			std::thread([&] {
				auto exml = game::engine.getSystem<WorldSystem>()->getXML()->FirstChildElement("Dialog");
				int newIndex;

				if (e.has_component<Direction>())
					d.talking = true;

				if (d.index == 9999) {
					ui::dialogBox(name.name, "", false, randomDialog[d.rindex % randomDialog.size()]);
					ui::waitForDialog();
				} else if (exml != nullptr) {
					while (exml->StrAttribute("name") != name.name)
						exml = exml->NextSiblingElement();

					exml = exml->FirstChildElement("text");
					while (exml->IntAttribute("id") != d.index)
						exml = exml->NextSiblingElement();

					auto oxml = exml->FirstChildElement("set");
					if (oxml != nullptr) {
						do game::setValue(oxml->StrAttribute("id"), oxml->StrAttribute("value"));
						while ((oxml = oxml->NextSiblingElement()));
						game::briceUpdate();
					}

					auto cxml = exml->FirstChildElement("content");
					const char *content;
					if (cxml == nullptr) {
						content = randomDialog[d.rindex % randomDialog.size()].c_str();
					} else {
						content = cxml->GetText() - 1;
						while (*++content && isspace(*content));
					}

					ui::dialogBox(name.name, "", false, content);
					ui::waitForDialog();

					if (exml->QueryIntAttribute("nextid", &newIndex) == XML_NO_ERROR)
						d.index = newIndex;
				}

				d.talking = false;
			}).detach();

			}
		}
	);
}

void DialogSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)en;
	(void)ev;
	(void)dt;
}
