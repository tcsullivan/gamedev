#include <mob.hpp>
#include <ui.hpp>
#include <world.hpp>

Mob::Mob(void)
{
    type = MOBT;
    rider = nullptr;
    canMove = true;
}

Page::Page(void) : Mob()
{

    ridable = false;
    aggressive = false;
    maxHealth = health = 50;
    width = HLINES(6);
    height = HLINES(4);
    tex = TextureIterator({"assets/items/ITEM_PAGE.png"});
}

void Page::act(void)
{
    if (player->loc.x > loc.x - 100 && player->loc.x < loc.x + 100 && isInside(ui::mouse) &&
        (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT))) {
        std::thread([this](void){
            ui::drawPage(pageTexPath);
            ui::waitForDialog();
            die();
        }).detach();
    }
}

bool Page::bindTex(void)
{
    glActiveTexture(GL_TEXTURE0);
    tex(0);
    return true;
}

void Page::createFromXML(const XMLElement *e)
{
    float Xlocx;
    if (e->QueryFloatAttribute("x", &Xlocx) == XML_NO_ERROR)
        loc.x = Xlocx;
    pageTexPath = e->StrAttribute("id");
}

Door::Door(void) : Mob()
{
    ridable = false;
    aggressive = false;
    maxHealth = health = 50;
    width = HLINES(12);
    height = HLINES(20);
    tex = TextureIterator({"assets/door.png"});
}

void Door::act(void)
{
}

bool Door::bindTex(void)
{
    glActiveTexture(GL_TEXTURE0);
    tex(0);
    return true;
}

void Door::createFromXML(const XMLElement *e)
{
    float Xlocx;
    if (e->QueryFloatAttribute("x", &Xlocx) == XML_NO_ERROR)
        loc.x = Xlocx;
}

Cat::Cat(void) : Mob()
{
    ridable = true;
    aggressive = false;
    maxHealth = health = 1000;
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

bool Cat::bindTex(void)
{
    glActiveTexture(GL_TEXTURE0);
    tex(0);
    return true;
}

void Cat::createFromXML(const XMLElement *e)
{
    float Xlocx;
    if (e->QueryFloatAttribute("x", &Xlocx) == XML_NO_ERROR)
        loc.x = Xlocx;
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
}

extern bool inBattle;
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

    if (inBattle)
        die();

    if (ground && direction) {
        ground = false;
        vel.y = .15;
        loc.y += HLINES(0.25f);
        vel.x = 0.05f * direction;
    }
}

bool Rabbit::bindTex(void)
{
    glActiveTexture(GL_TEXTURE0);
    tex(!ground);
    return true;
}

void Rabbit::createFromXML(const XMLElement *e)
{
    float Xlocx, Xhealth;
    if (e->QueryFloatAttribute("x", &Xlocx) == XML_NO_ERROR)
        loc.x = Xlocx;
    if (e->QueryFloatAttribute("health", &Xhealth) == XML_NO_ERROR)
        maxHealth = health = Xhealth;
    if (e->QueryBoolAttribute("aggressive", &aggressive) != XML_NO_ERROR)
        aggressive = false;
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
    auto deltaTime = game::time::getDeltaTime();
    if (!--actCounter) {
        actCounter = actCounterInitial;
        direction ^= 1;
    }

    if (loc.y <= initialY)
        vel.y = 0.02f * deltaTime;
    vel.x = (direction ? -0.02f : 0.02f) * deltaTime;
}

bool Bird::bindTex(void)
{
    glActiveTexture(GL_TEXTURE0);
    tex(0);
    return true;
}

void Bird::createFromXML(const XMLElement *e)
{
    float Xlocx, Xhealth;
    if (e->QueryFloatAttribute("x", &Xlocx) == XML_NO_ERROR)
        loc.x = Xlocx;
    if (e->QueryFloatAttribute("health", &Xhealth) == XML_NO_ERROR)
        maxHealth = health = Xhealth;
    if (e->QueryFloatAttribute("y", &initialY) != XML_NO_ERROR)
        initialY = 300;
    if (e->QueryBoolAttribute("aggressive", &aggressive) != XML_NO_ERROR)
        aggressive = false;
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

bool Trigger::bindTex(void)
{
    return false;
}

void Trigger::createFromXML(const XMLElement *e)
{
    float Xlocx;
    if (e->QueryFloatAttribute("x", &Xlocx) == XML_NO_ERROR)
        loc.x = Xlocx;
    id = e->StrAttribute("id");
}

Mob::~Mob()
{
	delete inv;
	delete[] name;
}

extern World *currentWorld;
void Mob::wander(void)
{
	static bool YAYA = false;

    if (forcedMove)
		return;

    if (aggressive && !YAYA && isInside(vec2 {player->loc.x + width / 2, player->loc.y + height / 4})) {
		if (!ui::dialogBoxExists) {
            std::thread([&](void){
			    auto *a = new Arena(currentWorld, player, this);
			    a->setStyle("");
			    a->setBackground(WorldBGType::Forest);
			    a->setBGM("assets/music/embark.wav");

			    ui::toggleWhiteFast();
			    YAYA = true;
			    ui::waitForCover();
			    YAYA = false;
			    currentWorld = a;
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
