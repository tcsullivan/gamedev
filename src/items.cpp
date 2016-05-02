#include <inventory.hpp>
#include <entities.hpp>

extern Player *player;

/************************************************************************************
*                                          GLOBAL                                   *
************************************************************************************/

/**************************************************
*                    USE ITEM                     *
**************************************************/

int BaseItem::useItem()
{
	return 0;
}

int Sword::useItem()
{
    if (inUse())
		return -1;

	std::thread([this]{
		setUse(true);
		volatile bool swing = true;
		bool back = false;
		float coef = 0.0f;

		while (swing) {

			// handle swinging
			if (!back)
				coef += .8f;
			else
				coef -= .4f;

			if (player->left)
				rotation = coef;
			else
				rotation = -coef;

			if (coef > 80 && !back)
				back = true;

			if (coef <= 0 && back) {
				swing = false;
				coef = 0.0f;
				rotation = 0.0f;
			}

			if (!back) {
				// handle collision with interaction
				hitbox.start.y = player->loc.y+(player->height/3);
				hitbox.start.x = player->left ? player->loc.x : player->loc.x + player->width;

				for (auto &e : interact) {
					float dist = 0.0f;
					while (dist < dim.y) {
						hitbox.end = hitbox.start;
						hitbox.end.x += dist * cos(rotation*PI/180);
						hitbox.end.y += dist * sin(rotation*PI/180);

						if (hitbox.end.x > e->loc.x && hitbox.end.x < e->loc.x + e->width) {
							if (hitbox.end.y > e->loc.y && hitbox.end.y < e->loc.y + e->height) {
								e->takeHit(damage, 600);
							}
						}

						dist += HLINES(1);
					}
				}
			}

			// add a slight delay
			SDL_Delay(1);
		}
		for (auto &e : interact)
			e->setCooldown(0);
		setUse(false);
	}).detach();

	return 0;
}

int Bow::useItem()
{
	return 0;
}

// TODO chance to hurt
int RawFood::useItem()
{
	return 0;
}

int Food::useItem()
{
    std::cout << "Yum!" << std::endl;
	return 0;
}


/**************************************************
*                       CLONE                     *
**************************************************/

BaseItem* BaseItem::clone()
{
	return new BaseItem(*this);
}

Sword* Sword::clone()
{
	return new Sword(*this);
}

Bow* Bow::clone()
{
	return new Bow(*this);
}

Food* Food::clone()
{
	return new Food(*this);
}

RawFood* RawFood::clone()
{
	return new RawFood(*this);
}

/************************************************************************************
*                                    ITEM SPECIFIC                                  *
************************************************************************************/

/**************************************************
*                      ITEM                       *
**************************************************/

bool Item::inUse()
{
	return beingUsed;
}

void Item::setUse(bool use)
{
	beingUsed = use;
}

void Item::addInteract(Entity* e)
{
	interact.push_back(e);
}

void Item::addInteract(std::vector<Entity*> e)
{
	for (auto &v : e) {
		interact.push_back(v);
	}
}

GLuint Item::rtex()
{
	return tex->image[0];
}

GLuint Item::rtex(int n)
{
	return tex->image[n];
}

Item::~Item()
{
	delete tex;
}

/**************************************************
*                      SWORD                      *
**************************************************/

float Sword::getDamage()
{
	return damage;
}

void Sword::setDamage(float d)
{
	damage = d;
}

/**************************************************
*                      BOW                        *
**************************************************/

float Bow::getDamage()
{
	return damage;
}

/**************************************************
*                      FOODS                      *
**************************************************/

float RawFood::getHealth()
{
	return health;
}
