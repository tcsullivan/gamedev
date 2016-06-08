#include <inventory.hpp>
#include <entities.hpp>
#include <world.hpp>

extern Player *player;
extern World *currentWorld;

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
						if (player->left)
							hitbox.end.x -= dist * cos(rotation*PI/180);
						else 
							hitbox.end.x += dist * cos(rotation*PI/180);

						hitbox.end.y += dist * sin(rotation*PI/180);

						if (hitbox.end.x > e->loc.x && hitbox.end.x < e->loc.x + e->width) {
							if (hitbox.end.y > e->loc.y && hitbox.end.y < e->loc.y + e->height) {
								if (e->type == MOBT)
									Mobp(e)->onHit(damage);
								else
									e->takeHit(damage, 600);

								static GLuint sColor = Texture::genColor(Color(255,0,0));
								GLfloat t[] = {0.0, 0.0,
											   1.0, 1.0};
								GLfloat v[] = {hitbox.start.x, 	hitbox.start.y, 1.0,
											   hitbox.end.x,	hitbox.end.y,	1.0};

					
								glBindTexture(GL_TEXTURE_2D, sColor);
								glUseProgram(worldShader);
								glEnableVertexAttribArray(worldShader_attribute_coord);
								glEnableVertexAttribArray(worldShader_attribute_tex);
						
								glVertexAttribPointer(worldShader_attribute_coord, 3, GL_FLOAT, GL_FALSE, 0, v);
								glVertexAttribPointer(worldShader_attribute_tex, 2, GL_FLOAT, GL_FALSE, 0, t);
								glDrawArrays(GL_LINES, 0, 2);

								glDisableVertexAttribArray(worldShader_attribute_coord);
								glDisableVertexAttribArray(worldShader_attribute_tex);
								glUseProgram(0);
								// add some blood
								// for(int r = 0; r < (rand()%5);r++)
								// 	currentWorld->addParticle(rand()%game::HLINE*3 + e->loc.x - .05f,e->loc.y + e->height*.5, game::HLINE,game::HLINE, -(rand()%10)*.01,((rand()%4)*.001-.002), {(rand()%75+10)/100.0f,0,0}, 10000);
							}
						}

						dist += HLINES(0.5f);
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

int Arrow::useItem()
{

	return 0;
}

int Bow::useItem()
{
	float rot = atan(sqrt(pow(ui::mouse.y-(player->loc.y + player->height),2)/pow(ui::mouse.x-player->loc.x,2)));
	float speed = 1.0;
	float vx = speed * cos(rot);
	float vy = speed * sin(rot);

	ui::mouse.x < player->loc.x ? vx *= -1 : vx *= 1;
	ui::mouse.y < player->loc.y + player->height ? vy *= -1 : vy *= 1;

	currentWorld->addParticle(	player->loc.x,					// x
								player->loc.y + player->height,	// y
								HLINES(3),						// width
								HLINES(3),						// height
								vx,								// vel.x
								vy,								// vel.y
								{ 139, 69, 19 },				// RGB color
								2500							// duration (ms)
							);



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

int ItemLight::useItem()
{
	std::cout << "fsdfsdf" << std::endl;
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

Arrow* Arrow::clone()
{
	return new Arrow(*this);
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

ItemLight* ItemLight::clone()
{
	return new ItemLight(*this);
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
*                      ARROW                      *
**************************************************/

float Arrow::getDamage()
{
	return damage;
}

void Arrow::setDamage(float d)
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

void Bow::setDamage(float d)
{
	damage = d;
}

/**************************************************
*                      FOODS                      *
**************************************************/

float RawFood::getHealth()
{
	return health;
}
