#include <player.hpp>

#include <brice.hpp>
#include <engine.hpp>
#include <ui.hpp>
#include <gametime.hpp>
#include <world.hpp>
#include <components.hpp>

void PlayerSystem::configure(entityx::EventManager &ev)
{
    ev.subscribe<KeyUpEvent>(*this);
    ev.subscribe<KeyDownEvent>(*this);
}

void PlayerSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) {
    (void)ev;
    (void)dt;

    auto& vel = *en.get(pid).component<Direction>().get();

    if (moveLeft & !moveRight)
        vel.x = -PLAYER_SPEED_CONSTANT;
    else if (!moveLeft & moveRight)
        vel.x = PLAYER_SPEED_CONSTANT;
    else
        vel.x = 0;

    vel.x *= speed;

    if (std::stoi(game::getValue("Slow")) == 1)
        vel.x /= 2.0f;
}

void PlayerSystem::receive(const KeyUpEvent &kue)
{
	auto kc = kue.keycode;

	if (kc == getControl(1)) {
		moveLeft = false;
	} else if (kc == getControl(2)) {
		moveRight = false;
	} else if (kc == getControl(3) || kc == getControl(4)) {
		speed = 1.0f;
	} else if (kc == getControl(5)) {
		/*if (p->inv->invHover) {
			p->inv->invHover = false;
		} else {
			if (!p->inv->selected)
				p->inv->invOpening ^= true;
			else
				p->inv->selected = false;

			p->inv->mouseSel = false;
		}

		// disable action ui
		ui::action::disable();*/
	}
}

void PlayerSystem::receive(const KeyDownEvent &kde)
{
	auto kc = kde.keycode;
	auto& loc = *game::entities.get(pid).component<Position>().get();
    auto& faceLeft = game::entities.get(pid).component<Sprite>().get()->faceLeft;

	/*auto worldSwitch = [&](const WorldSwitchInfo& wsi){
		player->canMove = false;
		ui::toggleBlackFast();
		ui::waitForCover();
		game::events.emit<BGMToggleEvent>(wsi.first->bgm, wsi.first);
		std::tie(currentWorld, player->loc) = wsi; // using p causes segfault
		game::engine.getSystem<WorldSystem>()->setWorld(currentWorld);
		ui::toggleBlackFast();
		ui::waitForUncover();
		player->canMove = true; // using p causes segfault
	};*/

	/*if ((kc == SDLK_SPACE) && (game::canJump & p->ground)) {
		p->loc.y += HLINES(2);
		p->vel.y = .4;
		p->ground = false;
	}*/

	if (!ui::dialogBoxExists || ui::dialogPassive) {
		if (kc == getControl(0)) {
			/*if (inBattle) {
				std::thread([&](void){
					auto thing = dynamic_cast<Arena *>(currentWorld)->exitArena(p);
					if (thing.first != currentWorld)
						worldSwitch(thing);
				}).detach();
			} else if (!ui::fadeIntensity) {
				std::thread([&](void){
					auto thing = currentWorld->goInsideStructure(p);
					if (thing.first != currentWorld)
						worldSwitch(thing);
				}).detach();
			}*/
		} else if (kc == getControl(1)) {
			if (!ui::fadeEnable) {
                moveLeft = faceLeft = true;
				moveRight = false;

				game::engine.getSystem<WorldSystem>()->goWorldLeft(loc);
			}
		} else if (kc == getControl(2)) {
			if (!ui::fadeEnable) {
				moveLeft = faceLeft = false;
                moveRight = true;

				game::engine.getSystem<WorldSystem>()->goWorldRight(loc);
   			}
		} else if (kc == getControl(3)) {
			if (game::canSprint)
				speed = 2.0f;
		} else if (kc == getControl(4)) {
			speed = .5;
		} else if (kc == getControl(5)) {
			/*static int heyOhLetsGo = 0;

			//edown = true;

			// start hover counter?
			if (!heyOhLetsGo) {
				heyOhLetsGo = game::time::getTickCount();
				p->inv->mouseSel = false;
			}

			// run hover thing
			if (game::time::getTickCount() - heyOhLetsGo >= 2 && !(p->inv->invOpen) && !(p->inv->selected)) {
				p->inv->invHover = true;

				// enable action ui
				ui::action::enable();
			}*/
		}
	} else if (kc == SDLK_DELETE) {
		game::endGame();
	} else if (kc == SDLK_t) {
		game::time::tick(50);
	}
}
