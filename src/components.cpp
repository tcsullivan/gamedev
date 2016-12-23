#include <components.hpp>

#include <entityx/entityx.h>
#include <events.hpp>

#include <render.hpp>
#include <ui.hpp>
#include <engine.hpp>
#include <world.hpp>
#include <brice.hpp>
#include <quest.hpp>

#include <atomic>

static std::vector<std::string> randomDialog (readFileA("assets/dialog_en-us"));

void MovementSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)ev;
	en.each<Position, Direction>([dt](entityx::Entity entity, Position &position, Direction &direction) {
		position.x += direction.x * dt;
		position.y += direction.y * dt;

		/*if (entity.has_component<Animate>() && entity.has_component<Sprite>()) {
			auto animate = entity.component<Animate>();
			entity.component<Sprite>()->sprite = 
				(direction.x != 0) ? animate->nextFrame() : animate->firstFrame();
		}*/
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

Texture RenderSystem::loadTexture(const std::string& file)
{
	loadTexString = file;
	loadTexResult = Texture();
	while (loadTexResult.isEmpty())
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	return loadTexResult;
}

void RenderSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)ev;

	if (!loadTexString.empty()) {
		loadTexResult = Texture(loadTexString);
		loadTexString.clear();
	}

	Render::worldShader.use();

	en.each<Visible, Sprite, Position>([dt](entityx::Entity entity, Visible &visible, Sprite &sprite, Position &pos) {
		(void)entity;
		// Verticies and shit
		float its = 0;
		for (auto &S : sprite.sprite) {
			auto sp = S.first;
			auto size = sp.size * game::HLINE;
			vec2 drawOffset(HLINES(S.second.x), HLINES(S.second.y));
			vec2 loc(pos.x + drawOffset.x, pos.y + drawOffset.y);

			GLfloat tex_coord[] = {sp.offset_tex.x, 				sp.offset_tex.y,
								   sp.offset_tex.x + sp.size_tex.x, sp.offset_tex.y,
								   sp.offset_tex.x + sp.size_tex.x, sp.offset_tex.y + sp.size_tex.y,

								   sp.offset_tex.x + sp.size_tex.x, sp.offset_tex.y + sp.size_tex.y,
								   sp.offset_tex.x, 				sp.offset_tex.y + sp.size_tex.y,
								   sp.offset_tex.x, 				sp.offset_tex.y};

			if(sprite.faceLeft) {
				glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(-1.0f,1.0f,1.0f));
				glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f - size.x - pos.x * 2.0f, 0.0f, 0.0f));

				glm::mat4 mov = scale * translate;
				glUniformMatrix4fv(Render::worldShader.uniform[WU_transform], 1, GL_FALSE, glm::value_ptr(mov));
			}

			GLfloat coords[] = {loc.x, 			loc.y, 			visible.z + its,
								loc.x + size.x,	loc.y, 			visible.z + its,
								loc.x + size.x, loc.y + size.y, visible.z + its,

								loc.x + size.x,	loc.y + size.y, visible.z + its,
								loc.x, 			loc.y + size.y, visible.z + its,
								loc.x, 			loc.y, 			visible.z + its};


			// make the entity hit flash red
			// TODO
			/*if (maxHitDuration-hitDuration) {
				float flashAmt = 1-(hitDuration/maxHitDuration);
				glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, flashAmt, flashAmt, 1.0);
			}*/

			sp.tex.use();

			glUniform1i(Render::worldShader.uniform[WU_texture], 0);
			Render::worldShader.enable();

			glVertexAttribPointer(Render::worldShader.coord, 3, GL_FLOAT, GL_FALSE, 0, coords);
			glVertexAttribPointer(Render::worldShader.tex, 2, GL_FLOAT, GL_FALSE, 0 ,tex_coord);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glUniformMatrix4fv(Render::worldShader.uniform[WU_transform], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
			glUniform4f(Render::worldShader.uniform[WU_tex_color], 1.0, 1.0, 1.0, 1.0);
			
			its-=.01;
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
			static std::atomic_bool dialogRun;
			(void)e;
			(void)d;

			if (((mce.position.x > pos.x) & (mce.position.x < pos.x + dim.width)) &&
			    ((mce.position.y > pos.y) & (mce.position.y < pos.y + dim.height))) {

			if (!dialogRun.load()) {
				std::thread([&] {
					std::string questAssignedText;
					int newIndex;

					auto exml = game::engine.getSystem<WorldSystem>()->getXML()->FirstChildElement("Dialog");
					dialogRun.store(true);

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

						auto qxml = exml->FirstChildElement("quest");
						if (qxml != nullptr) {
							const char *qname;
							auto qsys = game::engine.getSystem<QuestSystem>();

							do {
								// assign quest
								qname = qxml->Attribute("assign");
								if (qname != nullptr) {
									questAssignedText = qname;
									auto req = qxml->GetText();
									qsys->assign(qname, qxml->StrAttribute("desc"), req ? req : "");
								}

								// check / finish quest
								else {
									qname = qxml->Attribute("check");
									if (qname != nullptr) {
										if (qname != nullptr && qsys->hasQuest(qname) && qsys->finish(qname) == 0) {
											d.index = 9999;
										} else {
											ui::dialogBox(name.name, "", false, "Finish my quest u nug");
											ui::waitForDialog();
											return;
										}
									//	oldidx = d.index;
									//	d.index = qxml->UnsignedAttribute("fail");
									//	goto COMMONAIFUNC;
									}
								}
							} while((qxml = qxml->NextSiblingElement()));
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

						if (!questAssignedText.empty())
							ui::passiveImportantText(5000, ("Quest assigned:\n\"" + questAssignedText + "\"").c_str());

						if (exml->QueryIntAttribute("nextid", &newIndex) == XML_NO_ERROR)
							d.index = newIndex;
					}

					d.talking = false;
					dialogRun.store(false);
				}).detach();
			}
		}
	});
}

void DialogSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)en;
	(void)ev;
	(void)dt;
}

std::vector<Frame> developFrame(XMLElement* xml)
{
	Frame tmpf;
	std::vector<Frame> tmp;

	vec2 foffset;
	vec2 fsize;
	vec2 fdraw;

	// this is the xml elements first child. It will only be the <frame> tag
	auto framexml = xml->FirstChildElement();
	while (framexml) {
		// this will always be frame. but if it isn't we don't wanna crash the game
		std::string defframe = framexml->Name();
		if (defframe == "frame") {
			tmpf.clear();
			tmp.clear();
			// the xml element to parse each src of the frames
			auto sxml = framexml->FirstChildElement();
			while (sxml) {
				std::string sname = sxml->Name();
				if (sname == "src") {
					foffset = (sxml->Attribute("offset") != nullptr) ? 
						str2coord(sxml->Attribute("offset")) : vec2(0,0);
					fdraw = (sxml->Attribute("drawOffset") != nullptr) ?
						str2coord(sxml->Attribute("drawOffset")) : vec2(0,0);

					if (sxml->Attribute("size") != nullptr) {
						fsize = str2coord(sxml->Attribute("size"));
						tmpf.push_back(std::make_pair(SpriteData(sxml->GetText(), foffset, fsize), fdraw));
					} else {
						tmpf.push_back(std::make_pair(SpriteData(sxml->GetText(), foffset), fdraw));
					}
				}
				sxml = sxml->NextSiblingElement();
			}
			tmp.push_back(tmpf);
		}
		// if it's not a frame we don't care

		// parse next frame
		framexml = framexml->NextSiblingElement();
	}

	return tmp;
}
