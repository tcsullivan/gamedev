#include <systems/dialog.hpp>

#include <components/position.hpp>
#include <components/solid.hpp>
#include <components/dialog.hpp>
#include <components/flash.hpp>
#include <components/name.hpp>
#include <components/direction.hpp>

#include <brice.hpp>
#include <engine.hpp>
#include <fileio.hpp>
#include <quest.hpp>
#include <thread.hpp>
#include <ui.hpp>
#include <world.hpp>

#include <string>
#include <vector>

static std::vector<std::string> randomDialog (readFileA("assets/dialog_en-us"));

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

			e.replace<Flash>(Color(0, 255, 255));

			if (!dialogRun.load()) {
				// copy entity, windows destroys the original after thread detach
				std::thread([e, &pos, &dim, &d, &name] {
					std::string questAssignedText;
					int newIndex;

					auto exml = WorldSystem::getXML()->FirstChildElement("Dialog");
					dialogRun.store(true);

					if (e.has_component<Direction>())
						d.talking = true;

					if (d.index == 9999) {
						UISystem::dialogBox(name.name, randomDialog[d.rindex % randomDialog.size()]);
						UISystem::waitForDialog();
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

						auto ixml = exml->FirstChildElement("give");
						if (ixml != nullptr) {
							do {
								InventorySystem::add(ixml->StrAttribute("name"), ixml->IntAttribute("count"));
								ixml = ixml->NextSiblingElement();
							} while (ixml != nullptr);
						}

						auto qxml = exml->FirstChildElement("quest");
						if (qxml != nullptr) {
							const char *qname;

							do {
								// assign quest
								qname = qxml->Attribute("assign");
								if (qname != nullptr) {
									questAssignedText = qname;
									auto req = qxml->GetText();
									QuestSystem::assign(qname, qxml->StrAttribute("desc"), req ? req : "");
								}

								// check / finish quest
								else {
									qname = qxml->Attribute("check");
									if (qname != nullptr) {
										if (qname != nullptr && QuestSystem::finish(qname) == 0) {
											d.index = 9999;
										} else {
											UISystem::dialogBox(name.name, "Finish my quest u nug");
											UISystem::waitForDialog();
											return;
										}
									//	oldidx = d.index;
									//	d.index = qxml->UnsignedAttribute("fail");
									//	goto COMMONAIFUNC;
									}
								}
							} while((qxml = qxml->NextSiblingElement()));
						}

						auto xxml = exml->FirstChildElement("option");
						if (xxml != nullptr) {
							do {
								UISystem::dialogAddOption(xxml->StrAttribute("name"), xxml->StrAttribute("value"));
								xxml = xxml->NextSiblingElement();
							} while (xxml != nullptr);
						}

						auto cxml = exml->FirstChildElement("content");
						const char *content;
						if (cxml == nullptr) {
							content = randomDialog[d.rindex % randomDialog.size()].c_str();
						} else {
							content = cxml->GetText() - 1;
							while (*++content && isspace(*content));
						}

						UISystem::dialogBox(name.name, content);
						UISystem::waitForDialog();

						if (!questAssignedText.empty())
							UISystem::dialogImportant("Quest assigned:\n\"" + questAssignedText + "\"");
							//passiveImportantText(5000, ("Quest assigned:\n\"" + questAssignedText + "\"").c_str());

						if (!UISystem::getDialogResult().empty())
							d.index = std::stoi(UISystem::getDialogResult());
						else if (exml->QueryIntAttribute("nextid", &newIndex) == XML_NO_ERROR)
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

