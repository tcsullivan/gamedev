#include <mob.hpp>
#include <ui.hpp>
#include <world.hpp>
#include <brice.hpp>

extern World *currentWorld;

Mob::Mob(void)
{
    type = MOBT;
	inv = nullptr;
    rider = nullptr;
	xmle = nullptr;
    canMove = true;
	loc = 0;
}

Page::Page(void) : Mob()
{

    ridable = false;
    aggressive = false;
    maxHealth = health = 50;
    width = HLINES(6);
    height = HLINES(4);
    tex = TextureIterator({"assets/items/ITEM_PAGE.png"});
    pageTexture = 0;
}

void Page::act(void)
{
    if (player->loc.x > loc.x - 100 && player->loc.x < loc.x + 100 && isInside(ui::mouse) &&
        (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT))) {
        std::thread([this](void){
            ui::drawPage(pageTexture);
            ui::waitForDialog();
			game::setValue(cId, cValue);
			game::briceUpdate();
            die();
        }).detach();
    }
}

void Page::onHit(unsigned int _health)
{
	(void)_health;
	act();
}

bool Page::bindTex(void)
{
    glActiveTexture(GL_TEXTURE0);
    tex(0);
    return true;
}

void Page::createFromXML(XMLElement *e, World *w=nullptr)
{
	(void)w;
    float Xlocx;
    if (e->QueryFloatAttribute("x", &Xlocx) == XML_NO_ERROR)
        loc.x = Xlocx;
    pageTexPath = e->StrAttribute("id");
    pageTexture = Texture::loadTexture(pageTexPath);

	cId = e->StrAttribute("cid");
	cValue = e->StrAttribute("cvalue");

	xmle = e;
}

void Page::saveToXML(void)
{}

Door::Door(void) : Mob()
{
    ridable = false;
    aggressive = false;
    maxHealth = health = 50;
    width = HLINES(12);
    height = HLINES(20);
    tex = TextureIterator({"assets/style/classic/door.png"});
}

void Door::act(void)
{
}

void Door::onHit(unsigned int _health)
{
	(void)_health;
}

bool Door::bindTex(void)
{
    glActiveTexture(GL_TEXTURE0);
    tex(0);
    return true;
}

void Door::createFromXML(XMLElement *e, World *w=nullptr)
{
	(void)w;
    float Xlocx;
    if (e->QueryFloatAttribute("x", &Xlocx) == XML_NO_ERROR)
        loc.x = Xlocx;
}

void Door::saveToXML(void)
{}

Cat::Cat(void) : Mob()
{
    ridable = true;
    aggressive = false;
    maxHealth = health = 100000;
    width  = HLINES(19);
    height = HLINES(15);
    tex = TextureIterator({"assets/cat.png"});
    actCounterInitial = 0;
    actCounter = 1;
}

void Cat::act(void)
{
    static float vely = 0;
    if (rider != nullptr) {
        if (!rider->ground) {
            loc.y += HLINES(2);
            vel.y = .4;
        }
        if (rider->speed > 1) {
            vely = .5;
        } else if (rider->speed == .5) {
            vely = -.5;
        } else {
            vely = 0;
        }
        vel.y = vely;
        if (!rider->ground) {
            if ((vel.y -= .015) < -.2)
                rider->ground = true;
        }
        vel.x = .1 * (rider->left ? -1 : 1);
    } else {
        vel = 0;
    }
}

void Cat::onHit(unsigned int _health)
{
	health += _health;
}

bool Cat::bindTex(void)
{
    glActiveTexture(GL_TEXTURE0);
    tex(0);
    return true;
}

void Cat::createFromXML(XMLElement *e, World *w=nullptr)
{
	(void)w;
    float spawnc;

    if (e->QueryFloatAttribute("x", &spawnc) == XML_NO_ERROR)
        loc.x = spawnc;
	else
		loc.x = e->FloatAttribute("spawnx");

	if (e->QueryFloatAttribute("y", &spawnc) == XML_NO_ERROR)
		loc.y = spawnc;

	xmle = e;
}

void Cat::saveToXML(void)
{
	E_SAVE_COORDS;
	xmle->SetAttribute("alive", alive);
}

Rabbit::Rabbit(void) : Mob()
{
    ridable = true;
    aggressive = false;
    maxHealth = health = 50;
    width  = HLINES(10);
    height = HLINES(8);
    tex = TextureIterator({"assets/rabbit.png", "assets/rabbit1.png"});
    actCounterInitial = randGet() % 240 + 15;
    actCounter = 1;

	drop = {
		std::make_tuple("Dank MayMay", 5, 1.00f)
	};
}

void Rabbit::act(void)
{
    static int direction = 0;

    if (!--actCounter) {
        actCounter = actCounterInitial;
        direction = (randGet() % 3 - 1); 	//sets the direction to either -1, 0, 1
        if (direction == 0)
            ticksToUse /= 2;
        vel.x *= direction;
    }

    if (ground && direction) {
        ground = false;
        vel.y = .15;
        loc.y += HLINES(0.25f);
        vel.x = 0.05f * direction;
    }
}

void Rabbit::onHit(unsigned int _health)
{
	takeHit(_health, 600);
}

bool Rabbit::bindTex(void)
{
    glActiveTexture(GL_TEXTURE0);
    tex(!ground);
    return true;
}

void Rabbit::createFromXML(XMLElement *e, World *w=nullptr)
{
	(void)w;
    float spawnc;

	xmle = e;

    if (e->QueryFloatAttribute("x", &spawnc) == XML_NO_ERROR)
        loc.x = spawnc;
	else
		loc.x = e->FloatAttribute("spawnx");

	if (e->QueryFloatAttribute("y", &spawnc) == XML_NO_ERROR)
		loc.y = spawnc;

	E_LOAD_HEALTH;
	
	if (e->QueryBoolAttribute("aggressive", &aggressive) != XML_NO_ERROR)
		aggressive = false;
}

void Rabbit::saveToXML(void)
{
	E_SAVE_HEALTH;
}

Bird::Bird(void) : Mob()
{
    ridable = true;
    aggressive = false;
    maxHealth = health = 50;
    width = HLINES(8);
    height = HLINES(8);
    tex = TextureIterator({"assets/robin.png"});
    actCounterInitial = actCounter = 200;
}

void Bird::act(void)
{
    static bool direction = false;
    static const float wstart = currentWorld->getWorldStart();

    if (!--actCounter) {
        actCounter = actCounterInitial;
        direction ^= 1;
    }

    if (loc.x > -wstart - HLINES(10.0f))
        loc.x = wstart + HLINES(10.0f);
    else if (loc.x < wstart + HLINES(10.0f))
        loc.x = -wstart - HLINES(10.0f);

    if (loc.y <= initialY)
        vel.y = 0.3f;
    vel.x = direction ? -0.3f : 0.3f;
}

void Bird::onHit(unsigned int _health)
{
	takeHit(_health, 1000);
}

bool Bird::bindTex(void)
{
    glActiveTexture(GL_TEXTURE0);
    tex(0);
    return true;
}

void Bird::createFromXML(XMLElement *e, World *w=nullptr)
{
	(void)w;
    float Xlocx, Xhealth;
	
	xmle = e;

	if (e->QueryFloatAttribute("x", &Xlocx) == XML_NO_ERROR)
		loc.x = Xlocx;
	if (e->QueryFloatAttribute("y", &initialY) != XML_NO_ERROR)
		initialY = 300;

	E_LOAD_HEALTH;

    if (e->QueryBoolAttribute("aggressive", &aggressive) != XML_NO_ERROR)
        aggressive = false;
}

void Bird::saveToXML(void)
{
	E_SAVE_COORDS;
	E_SAVE_HEALTH;
}

Trigger::Trigger(void) : Mob()
{
    ridable = false;
    aggressive = false;
    maxHealth = health = 50;
    width = HLINES(20);
    height = 2000;
    //tex = TextureIterator();
    triggered = false;
}

void Trigger::act(void)
{
    auto c = player->loc.x + player->width / 2;
    static bool running = false;

    if (triggered) {
        die();
    } else if (!running && c > loc.x && c < loc.x + width) {
        std::thread([&]{
            running = true;

            XMLDocument xml;
            XMLElement *exml;

            xml.LoadFile(currentXML.c_str());
            exml = xml.FirstChildElement("Trigger");

            while(exml->StrAttribute("id") != id)
                exml = exml->NextSiblingElement();

            player->vel.x = 0;

            ui::toggleBlackFast();
            ui::waitForCover();

            std::string text = exml->GetText();
            char *pch = strtok(&text[0], "\n");

            while (pch) {
                ui::importantText(pch);
                ui::waitForDialog();
                pch = strtok(NULL, "\n");
            }

            ui::toggleBlackFast();

            triggered = true;
            running = false;
        }).detach();
    }
}

void Trigger::onHit(unsigned int _health)
{
	(void)_health;
}

bool Trigger::bindTex(void)
{
    return false;
}

void Trigger::createFromXML(XMLElement *e, World *w=nullptr)
{
	(void)w;
    float Xlocx;
    if (e->QueryFloatAttribute("x", &Xlocx) == XML_NO_ERROR)
        loc.x = Xlocx;
    id = e->StrAttribute("id");
}

void Trigger::saveToXML(void)
{}

Mob::~Mob()
{
	delete inv;
}

extern World *currentWorld;
extern Arena *arena;

void Mob::wander(void)
{
	static bool YAYA = false;

    if (forcedMove)
		return;

    if (aggressive && !YAYA && isInside(vec2 {player->loc.x + width / 2, player->loc.y + height / 4})) {
		if (!ui::dialogBoxExists) {
            std::thread([&](void){
			    arena->fight(currentWorld, player, this);
			    ui::toggleWhiteFast();
			    YAYA = true;
			    ui::waitForCover();
			    YAYA = false;
			    currentWorld = arena;
			    ui::toggleWhiteFast();
            }).detach();
		}
	}
    act();
}

void Mob::ride(Entity *e)
{
    if (!ridable)
        return;

    if (rider == e)
        rider = nullptr;
    else
        rider = e;
}

void Mob::onDeath(void)
{
	vec2 q = vec2 {player->loc.x, game::SCREEN_HEIGHT - 100.0f};

	ui::putTextL(q, "Player got: ");

	for (const auto &d : drop) {
		if ((randGet() % 100) < std::get<float>(d) * 100.0f) {
			q.y -= 20;
			ui::putTextL(q, "%d x %s", std::get<unsigned int>(d), std::get<std::string>(d).c_str());
			player->inv->addItem(std::get<std::string>(d), std::get<unsigned int>(d));
		}
	}
}
