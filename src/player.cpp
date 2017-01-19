#include <player.hpp>

#include <brice.hpp>
#include <ui.hpp>
#include <gametime.hpp>
#include <world.hpp>
#include <particle.hpp>

static const char *spriteXML =
	"<Sprite> \
		<frame> \
			<src limb='0' offset='0,0' size='15,23' drawOffset='0,9'>assets/player/player.png</src> \
			<src limb='1' offset='15,0' size='12,11' drawOffset='2,0'>assets/player/player.png</src>\
		</frame> \
	</Sprite>";

void PlayerSystem::create(void)
{
	player = game::entities.create();
	player.assign<Position>(0.0f, 100.0f);
	player.assign<Direction>(0.0f, 0.0f);
	// The new g value is a multiplier for the gravity constant. This allows for air resistance simulation.
	//player.assign<Physics>(-0.001f);
	player.assign<Physics>(1);
	player.assign<Visible>(-0.2f);
	auto sprite = player.assign<Sprite>();
	XMLDocument xmld;
	xmld.Parse(spriteXML);
	auto frame = developFrame(xmld.FirstChildElement("Sprite"));
	if (frame.size() > 0)
		sprite->sprite = frame.at(0);
	vec2 dim = player.component<Sprite>().get()->getSpriteSize();
	float cdat[2] = {dim.x, dim.y};
	player.assign<Solid>(cdat[0], cdat[1]);
}

void PlayerSystem::configure(entityx::EventManager &ev)
{
    ev.subscribe<KeyUpEvent>(*this);
    ev.subscribe<KeyDownEvent>(*this);
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
