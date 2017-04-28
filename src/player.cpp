#include <player.hpp>

#include <brice.hpp>
#include <ui.hpp>
#include <gametime.hpp>
#include <world.hpp>
#include <particle.hpp>
#include <attack.hpp>

static const char *spriteXML =
	"<Sprite> \
		<frame> \
			<src limb='0' offset='0,0' size='15,16' drawOffset='0,9'>assets/player/player.png</src> \
			<src limb='1' offset='0,16' size='13,12' drawOffset='0,20'>assets/player/player.png</src>\
			<src limb='2' offset='15,0' size='14,11' drawOffset='1,0'>assets/player/player.png</src>\
			<src limb='3' offset='15,14' size='15,13' drawOffset='0,9'>assets/player/player.png</src>\
		</frame> \
	</Sprite>";

static const char *animationXML = 
	"<Animation>\
		<movement>\
			<limb update='60.0' id='2'>\
				<frame>\
					<src offset='15,0' size='14,11' drawOffset='1,0'>assets/player/player.png</src>\
				</frame>\
				<frame>\
					<src offset='29,0' size='14,11' drawOffset='1,0'>assets/player/player.png</src>\
				</frame>\
				<frame>\
					<src offset='43,0' size='14,11' drawOffset='1,0'>assets/player/player.png</src>\
				</frame>\
				<frame>\
					<src offset='57,0' size='14,11' drawOffset='1,0'>assets/player/player.png</src>\
				</frame>\
				<frame>\
					<src offset='71,0' size='14,11' drawOffset='1,0'>assets/player/player.png</src>\
				</frame>\
				<frame>\
					<src offset='85,0' size='14,11' drawOffset='1,0'>assets/player/player.png</src>\
				</frame>\
				<frame>\
					<src offset='99,0' size='14,11' drawOffset='1,0'>assets/player/player.png</src>\
				</frame>\
				<frame>\
					<src offset='113,0' size='14,11' drawOffset='1,0'>assets/player/player.png</src>\
				</frame>\
				<frame>\
					<src offset='127,0' size='14,11' drawOffset='1,0'>assets/player/player.png</src>\
				</frame>\
			</limb>\
		</movement>\
		<movement>\
			<limb update='250.0' id='3'>\
				<frame>\
					<src offset='15,14' size='15,13' drawOffset='0,9'>assets/player/player.png</src>\
				</frame>\
				<frame>\
					<src offset='32,14' size='14,13' drawOffset='0,9'>assets/player/player.png</src>\
				</frame>\
				<frame>\
					<src offset='50,14' size='14,13' drawOffset='0,9'>assets/player/player.png</src>\
				</frame>\
				<frame>\
					<src offset='68,14' size='17,13' drawOffset='-1,9'>assets/player/player.png</src>\
				</frame>\
			</limb>\
		</movement>\
	</Animation>";

void PlayerSystem::create(void)
{
	player = game::entities.create();
	player.assign<Player>();
	player.assign<Position>(0.0f, 100.0f);
	player.assign<Direction>(0.0f, 0.0f);
	//player.assign<Physics>(-0.001f);
	player.assign<Physics>(.25);
	player.assign<Visible>(-0.2f);
	player.assign<Health>(100);
	auto sprite = player.assign<Sprite>();
	XMLDocument xmld;
	xmld.Parse(spriteXML);
	auto frame = developFrame(xmld.FirstChildElement("Sprite"));
	if (frame.size() > 0)
		sprite->sprite = frame.at(0);
	vec2 dim = player.component<Sprite>().get()->getSpriteSize();
	float cdat[2] = {dim.x, dim.y};
	player.assign<Solid>(cdat[0], cdat[1]);

	// handle player animation
	xmld.Parse(animationXML);
	player.assign<Animate>(nullptr, xmld.FirstChildElement());
}

void PlayerSystem::configure(entityx::EventManager &ev)
{
    ev.subscribe<KeyUpEvent>(*this);
    ev.subscribe<KeyDownEvent>(*this);
	ev.subscribe<UseItemEvent>(*this);
}

void PlayerSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) {
	(void)en;
    (void)ev;
    (void)dt;

    auto& vel = *player.component<Direction>().get();

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
		}*/
	}
}

void PlayerSystem::receive(const KeyDownEvent &kde)
{
	static auto& worldSystem = *game::engine.getSystem<WorldSystem>();

	auto kc = kde.keycode;
	auto& loc = *player.component<Position>().get();
	auto& vel = *player.component<Direction>().get();

	if ((kc == SDLK_SPACE) && game::canJump && ((vel.y > -0.01) & (vel.y < 0.01))) {
		loc.y += HLINES(2);
		vel.y = .4;
		vel.grounded = false;
	} else if (!ui::dialogBoxExists || ui::dialogPassive) {
		if (kc == getControl(0)) {
			if (!ui::fadeIntensity)
				worldSystem.goWorldPortal(loc);

		} else if (kc == getControl(1)) {
			if (!ui::fadeEnable) {
                moveLeft = true;
				moveRight = false;

				worldSystem.goWorldLeft(loc);
			}
		} else if (kc == getControl(2)) {
			if (!ui::fadeEnable) {
				moveLeft = false;
                moveRight = true;

				worldSystem.goWorldRight(loc, *player.component<Solid>().get());
   			}
		} else if (kc == getControl(3)) {
			if (game::canSprint)
				speed = 2.0f;

			game::engine.getSystem<ParticleSystem>()->addMultiple(10, ParticleType::SmallBlast,
				[&](){ return vec2(loc.x, loc.y); }, 500, 7);
		} else if (kc == getControl(4)) {
			speed = .5;
		} else if (kc == getControl(5)) {
			//static int heyOhLetsGo = 0;
			//edown = true;
			// start hover counter?
			//if (!heyOhLetsGo) {
			//	heyOhLetsGo = game::time::getTickCount();
			//	p->inv->mouseSel = false;
			//}

			// run hover thing
			//if (game::time::getTickCount() - heyOhLetsGo >= 2 && !(p->inv->invOpen) && !(p->inv->selected))
			//	p->inv->invHover = true;
		}
	} else if (kc == SDLK_DELETE) {
		game::endGame();
	} else if (kc == SDLK_t) {
		game::time::tick(50);
	}
}

vec2 PlayerSystem::getPosition(void) const
{
	auto& loc = *game::entities.component<Position>(player.id()).get();
    return vec2(loc.x, loc.y);
}

void PlayerSystem::receive(const UseItemEvent& uie)
{
	static std::atomic_bool cool (true);

	if (cool.load()) {
		if (uie.item->sound != nullptr)
			Mix_PlayChannel(0, uie.item->sound, 0);

		if (uie.item->type == "Sword") {
			auto loc = getPosition();
			auto &solid = *player.component<Solid>().get();
			loc.x += solid.width / 2, loc.y += solid.height / 2;
			game::events.emit<AttackEvent>(loc, AttackType::ShortSlash);
		} else if (uie.item->type == "Bow") {
			if (game::engine.getSystem<InventorySystem>()->take("Arrow", 1)) {
				auto e = game::entities.create();
				auto pos = getPosition();
				e.assign<Position>(pos.x, pos.y + 10);

				auto angle = std::atan2(uie.curs.y - pos.y, uie.curs.x - pos.x);
				e.assign<Direction>(1 * std::cos(angle), 1 * std::sin(angle));

				e.assign<Visible>(-5);
				e.assign<Physics>();
				auto sprite = e.assign<Sprite>();
				auto tex = game::engine.getSystem<InventorySystem>()->getItem("Arrow");
				sprite->addSpriteSegment(SpriteData(tex.sprite), 0);
				auto dim = HLINES(sprite->getSpriteSize());
				e.assign<Solid>(dim.x, dim.y);
				e.assign<Hit>(10);
			}
		}

		/*cool.store(false);
		std::thread([&](void) {
			std::this_thread::sleep_for(std::chrono::milliseconds(uie.item->cooldown));
			cool.store(true);
		}).detach();*/
	}
}
