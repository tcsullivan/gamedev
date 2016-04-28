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
    std::cout << "Swing!" << std::endl;
	std::thread([this]{
		player->inv->usingi = true;
		bool swing = true;
		float coef = 0.0f;

		while (swing) {
			coef += .01f;
			if (player->left)
				rotation = coef;
			else
				rotation = -coef;
		}
		player->inv->usingi = false;
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

Item::~Item()
{
	delete tex;
}

GLuint Item::rtex()
{
	return tex->image[0];
}

GLuint Item::rtex(int n)
{
	return tex->image[n];
}

/**************************************************
*                      SWORD                      *
**************************************************/

float Sword::getDamage()
{
	return damage;
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
