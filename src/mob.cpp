#include <mob.hpp>
#include <ui.hpp>

Page::Page(void)
{
    type = MOBT;
    aggressive = false;
    maxHealth = health = 50;
    canMove = true;
    width = HLINES(6);
    height = HLINES(4);
    tex = new Texturec({"assets/items/ITEM_PAGE.png"});
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
    tex->bind(0);
    return true;
}

void Page::createFromXML(const XMLElement *e)
{
    float Xlocx;
    if (e->QueryFloatAttribute("x", &Xlocx) == XML_NO_ERROR)
        loc.x = Xlocx;
    pageTexPath = e->StrAttribute("id");
}

Door::Door(void)
{
    type = MOBT;
    aggressive = false;
    maxHealth = health = 50;
    canMove = true;
    width = HLINES(12);
    height = HLINES(20);
    tex = new Texturec({"assets/door.png"});
}

void Door::act(void)
{
}

bool Door::bindTex(void)
{
    glActiveTexture(GL_TEXTURE0);
    tex->bind(0);
    return true;
}

void Door::createFromXML(const XMLElement *e)
{
    float Xlocx;
    if (e->QueryFloatAttribute("x", &Xlocx) == XML_NO_ERROR)
        loc.x = Xlocx;
}

Rabbit::Rabbit(void)
{
    type = MOBT;
    canMove = true;
    aggressive = false;
    maxHealth = health = 50;
    width  = HLINES(10);
    height = HLINES(8);
    tex = new Texturec({"assets/rabbit.png", "assets/rabbit1.png"});
    actCounterInitial = getRand() % 240 + 15;
    actCounter = 1;
}

void Rabbit::act(void)
{
    static int direction = 0;
    if (!--actCounter) {
        actCounter = actCounterInitial;
        direction = (getRand() % 3 - 1); 	//sets the direction to either -1, 0, 1
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

bool Rabbit::bindTex(void)
{
    glActiveTexture(GL_TEXTURE0);
    tex->bind(!ground);
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

Bird::Bird(void)
{
    type = MOBT;
    aggressive = false;
    maxHealth = health = 50;
    canMove = true;
    width = HLINES(8);
    height = HLINES(8);
    tex = new Texturec({"assets/robin.png"});
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
    tex->bind(0);
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

Trigger::Trigger(void)
{
    type = MOBT;
    aggressive = false;
    maxHealth = health = 50;
    canMove = true;
    width = HLINES(20);
    height = 2000;
    tex = new Texturec(0);
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

Mob::~Mob() {
	delete inv;
	delete tex;
	delete[] name;
}

void Mob::wander(void) {
	//static bool YAYA = false;
	if (forcedMove)
		return;
	/*if (aggressive && !YAYA && isInside(vec2 {player->loc.x + width / 2, player->loc.y + height / 4})) {
		if (!ui::dialogBoxExists) {
			Arena *a = new Arena(currentWorld, player, this);
			a->setStyle("");
			a->setBackground(WorldBGType::Forest);
			a->setBGM("assets/music/embark.wav");

			ui::toggleWhiteFast();
			YAYA = true;
			ui::waitForCover();
			YAYA = false;
			currentWorld = a;
			ui::toggleWhiteFast();
		}
	}*/
    act();
}
