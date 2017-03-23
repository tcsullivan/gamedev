/**
 * @file components.hpp
 * @brief Where all of an enities possible components are stored.
 * Using an ECS (Entity component system) the entities are given components on the fly,
 * this allows the entity to change stats or skills on the go. This also allows every "object"
 * the be an entity, and it gives the game a much better customizability over xml.
 */
#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include <string>
#include <vector>

#include <error.hpp>
#include <events.hpp>
#include <inventory.hpp>
#include <random.hpp>
#include <texture.hpp>
#include <vector2.hpp>

#include <entityx/entityx.h>
#include <tinyxml2.h>
using namespace tinyxml2;

//  TODO heyyy guys
#include <components/all.hpp>

/**
 * @struct Position
 * @brief Stores the position of an entity on the xy plane.
 */
struct Position : public Component {
	/**
	 * Constructor that sets the position of the object, if nothing is passed it will default to 0.
	 * @param x The x position the object will be placed at.
	 * @param y the y position the object will be placed at.
	 */
	Position(float x = 0.0f, float y = 0.0f): x(x), y(y) {}
	Position(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	float x; /**< The x position in the world */
	float y; /**< The y position in the world */

	void fromXML(XMLElement* imp, XMLElement* def) final {
		vec2 c;
		if (imp->Attribute("position") != nullptr)
			c = imp->StrAttribute("position");
		else
			c = def->StrAttribute("value");

		x = c.x, y = c.y;
	}
};

/**
 * @struct Direction
 * @brief Store an entities velocity.
 * This allows the entity to move throughout the world.
 */
struct Direction : public Component {
	/**
	 * Constructor that sets the velocity, if no position is passed, it defaults to (0,0).
	 * @param x The velocity of the object on the x axis.
	 * @param y The velocity of the object on the y axis.
	 */
	Direction(float x = 0.0f, float y = 0.0f): x(x), y(y), grounded(false) {}
	Direction(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	float x; /**< Velocity the object is moving in the x direction, this is added to the position */
	float y; /**< Velocity the object is moving in the y direction, this is added to the position */
	bool grounded;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		vec2 c;
		if (imp->Attribute("direction") != nullptr) {
			c = imp->StrAttribute("direction");
		} else if (def->Attribute("value") != nullptr) {
			c = def->StrAttribute("value");
		} else {
			c = vec2(0, 0);
		}

		x = c.x, y = c.y, grounded = false;
	}
};

/**
 * @struct Physics
 * @brief Allows and entity to react to gravity and frictions.
 * When an entity inherits this component it will react with gravity and move with friction.
 */
struct Physics : public Component {
	/**
	 * Constructor that sets the gravity constant, if not specified it becomes 0.
	 * @param g The non default gravity constant.
	 */
	Physics(float g = 1.0f): g(g) {}
	Physics(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	float g; /**< The gravity constant, how fast the object falls */

	void fromXML(XMLElement* imp, XMLElement* def) final {
		if (imp->QueryFloatAttribute("gravity", &g) != XML_NO_ERROR) {
			if (def->QueryFloatAttribute("value", &g) != XML_NO_ERROR)
				g = 1.0f;
		}
	}
};

/**
 * @struct Grounded
 * @brief Places an entity without physics on the ground.
 * This is used so we don't have to update the physics of a non-moving object every loop.
 */
struct Grounded : public Component {
	Grounded(bool g = false)
		: grounded(g) {}
	Grounded(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	bool grounded;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp;
		(void)def;
		grounded = false;
	}
};

/**
 * @struct Health
 * @brief Gives and entity health and stuff.
 */
struct Health : public Component {
	/**
	 * Constructor that sets the variables, with 1 health as default.
	 */
	Health(int m = 1, int h = 0)
		: health(h != 0 ? h : m), maxHealth(m) {}
	Health(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	int health; /**< The current amount of health */
	int maxHealth; /**< The maximum amount of health */

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp;
		(void)def;
		// TODO
		if (def->QueryIntAttribute("value", &health) != XML_NO_ERROR)
			health = 1;
		maxHealth = health;
	}
};

struct Portal : public Component {
	Portal(std::string tf = "")
		: toFile(tf) {}
	Portal(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	std::string toFile;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)def;
		toFile = imp->StrAttribute("inside");
	}
};

struct Name : public Component {
	Name(std::string n = "")
		: name(n) {}
	Name(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	std::string name;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		auto n = imp->Attribute("name");

		// TODO check def's nullness	
		name = n != nullptr ? n : def->Attribute("value");
	}
};

struct Player {};

struct ItemDrop {
	ItemDrop(InventoryEntry& ie)
		: item(ie) {}

	InventoryEntry item;
};

/**
 * @struct Solid
 * @brief Allows an entity to collide with other objects.
 * When an entity has this component it can collide with the world and other objects.
 */
struct Solid : public Component {
	/**
	 * Constructor that sets the entities dimensions based on what is passed.
	 * @param w The desired width of the entity.
	 * @param h The desired height of the entity.
	 */
	Solid(float w = 0.0f, float h = 0.0f)
		: width(w), height(h), offset(0), passable(true) {}
	Solid(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	void Passable(bool v) { passable = v; }
	bool Passable(void)   { return passable; }

	float width; /**< The width of the entity in units */
	float height; /**< The height of the entity in units */
	vec2 offset; /**< This allows us to make the hitbox in any spot */
	bool passable; /**< This determines whether or not one can pass by the entity */

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp;
		vec2 c;
		if (def->Attribute("value") != nullptr)
			c = def->StrAttribute("value");
		else
			c = vec2(0, 0);

		width = c.x, height = c.y, offset = 0, passable = true;
	}
};

struct SpriteData {
	SpriteData(void) = default;
	
	SpriteData(std::string path, vec2 off):
		offset(off)	{
			tex = Texture(path);
			size = tex.getDim();

			size_tex = vec2(1.0, 1.0);
			
			offset_tex.x = offset.x/size.x;
			offset_tex.y = offset.y/size.y;
	}

	SpriteData(std::string path, vec2 off, vec2 si):
		size(si), offset(off) {
			tex = Texture(path);
			vec2 tmpsize = tex.getDim();

			size_tex.x = size.x/tmpsize.x;
			size_tex.y = size.y/tmpsize.y;
			
			offset_tex.x = offset.x/tmpsize.x;
			offset_tex.y = offset.y/tmpsize.y;
	}

	SpriteData(Texture t)
		: tex(t) {
		size_tex = 1;
		offset_tex = 0;
		size = tex.getDim();
		offset = 0;
	}

	Texture tex;
	vec2 size;
	vec2 offset;
	
	vec2 offset_tex;
	vec2 size_tex;

	unsigned int limb;
};

using Frame = std::vector<std::pair<SpriteData, vec2>>;

std::vector<Frame> developFrame(XMLElement*);

/**
 * @struct Sprite
 * @brief If an entity is visible we want to be able to see it.
 * Each entity is given a sprite, a sprite can consist of manu frames or pieces to make one.
 */
struct Sprite : public Component {
	Sprite(bool left = false)
	 	: faceLeft(left) {}
	Sprite(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	Frame getSprite() {
		return sprite;
	}

	int clearSprite() {
		if (sprite.empty())
			return 0;

		sprite.clear();
		return 1;
	}

	int addSpriteSegment(SpriteData data, vec2 loc) {
		//TODO if sprite is in this spot, do something
		sprite.push_back(std::make_pair(data, loc));
		return 1;
	}

	int changeSpriteSegment(SpriteData data, vec2 loc) {
		for (auto &s : sprite) {
			if (s.second == loc) {
				s.first = data;

				return 1;
			}
		}
		addSpriteSegment(data, loc);
		return 0;
	}

	vec2 getSpriteSize() {
		vec2 st; /** the start location of the sprite (bottom left)*/
		vec2 ed; /** the end ofthe location of the sprite (bottom right)*/
		vec2 dim; /** how wide the sprite is */

		if (sprite.size()) {
			st.x = sprite[0].second.x;
			st.y = sprite[0].second.y;

			ed.x = sprite[0].second.x + sprite[0].first.size.x;
			ed.y = sprite[0].second.y + sprite[0].first.size.y;
		} else {
			return vec2(0.0f, 0.0f);
		}

		for (auto &s : sprite) {
			if (s.second.x < st.x)
				st.x = s.second.x;
			if (s.second.y < st.y)
				st.y = s.second.y;

			if (s.second.x + s.first.size.x > ed.x)
				ed.x = s.second.x + s.first.size.x;
			if (s.second.y + s.first.size.y > ed.y)
				ed.y = s.second.y + s.first.size.y;
		}

		dim = vec2(ed.x - st.x, ed.y - st.y);
		dim.x *= game::HLINE;
		dim.y *= game::HLINE;
		return dim;
	}

	Frame sprite;
	bool faceLeft;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp;
		auto frames = developFrame(def);
		if (!frames.empty())
			sprite = frames.at(0);
	}
};

/**
 * @struct Limb
 * @brief Storage of frames for the limbs of a sprite.
 * This will allow us to only update a certain limb. This was we can do mulitple animation types at once.
 */
struct Limb {
	Limb() {
	}

	// adds frame to the back of the frame stack
	void addFrame(Frame fr) {
		frame.push_back(fr);
	}

	void firstFrame(Frame& duckmyass) {
		// loop through the spritedata of the sprite we wanna change
		for (auto &d : duckmyass) {
			// if the sprite data is the same limb as this limb
			if (d.first.limb == limbID) {
				// rotate through (for safety) the first frame to set the limb
				for (auto &fa : frame.at(0)) {
					if (fa.first.limb == limbID) {
						d.first = fa.first;
						d.second = fa.second;
					}
				}
			}
		}
	}

	void nextFrame(Frame& duckmyass, float dt) {
		updateCurrent -= dt;
		if (updateCurrent <= 0) {
			updateCurrent = updateRate;
		} else {
			return;
		}


		if (index < frame.size() - 1)
			index++;
		else
			index = 0;
		
		for (auto &d : duckmyass) {
			if (d.first.limb == limbID) {
				for (auto &fa : frame.at(index)) {
					if (fa.first.limb == limbID) {
						d.first = fa.first;
						d.second = fa.second;
					}
				}
			}
		}

	}

	float updateRate; /**< How often we will change each frame. */
	float updateCurrent = 0; /**< How much has been updated in the current frame. */
	unsigned int updateType; /**< What the updateRate will base it's updates off of.
						ie: Movement, attacking, jumping. */
	unsigned int limbID; /**< The id of the limb we will be updating */
	
	unsigned int index = 0; /**< The current sprite being used for the limb. */

	std::vector<Frame> frame; /**< The multiple frames of each limb. */
};

//TODO kill clyne
struct Animate : public Component {
	// COMMENT
	std::vector<Limb> limb;
	// COMMENT	
	unsigned int index;

	Animate(){
		index = 0;
	}

	Animate(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	// COMMENT

	void firstFrame(unsigned int updateType, Frame &sprite) {
		unsigned int upid = updateType; //^see todo
		for (auto &l : limb) {
			if (l.updateType == upid) {
				l.firstFrame(sprite);
			}
		}
	}
	 //TODO make updateType an enum
	void updateAnimation(unsigned int updateType, Frame& sprite, float dt) {
		unsigned int upid = updateType; //^see todo
		for (auto &l : limb) {
			if (l.updateType == upid) {
				l.nextFrame(sprite, dt);
			}
		}
	}

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp;

		auto animx = def->FirstChildElement();
		uint limbid = 0;
		float limbupdate = 0;
		uint limbupdatetype = 0;

		while (animx != nullptr) {
			if (std::string(animx->Name()) == "movement") {
				limbupdatetype = 1;

				auto limbx = animx->FirstChildElement();
				while (limbx != nullptr) {
					if (std::string(limbx->Name()) == "limb") {
						auto frames = developFrame(limbx);
						limb.push_back(Limb());
						auto& newLimb = limb.back();
						newLimb.updateType = limbupdatetype;

						if (limbx->QueryUnsignedAttribute("id", &limbid) == XML_NO_ERROR)
							newLimb.limbID = limbid;
						if (limbx->QueryFloatAttribute("update", &limbupdate) == XML_NO_ERROR)
							newLimb.updateRate = limbupdate;
								
						// place our newly developed frames in the entities animation stack
						for (auto &f : frames) {
							newLimb.addFrame(f);
							for (auto &fr : newLimb.frame) {
								for (auto &sd : fr)
									sd.first.limb = limbid;
							}
						}
					}

					limbx = limbx->NextSiblingElement();
				}
			}
				
			animx = animx->NextSiblingElement();
		}
	}
};

/**
 * @struct Visible
 * @brief If an entity is visible we want to be able to draw it.
 */
struct Visible : public Component {
	/**
	 * @brief Decide what layer to draw the entity on.
	 * When stuff is drawn, it is drawn on a "layer". This layer gives more of a 3D effect to the world.
	 * @param z The desired "layer" of the entity.
	 */
	Visible(float z = 0.0f): z(z) {}
	Visible(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	float z; /**< The value along the z axis the entity will be drawn on */

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp;
		if (def->QueryFloatAttribute("value", &z) != XML_NO_ERROR)
			z = 0;
	}
};

struct Dialog : public Component {
	Dialog(int idx = 0)
		: index(idx), rindex((idx == 9999) ? randGet() : idx), talking(false) {}
	Dialog(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	int index;
	int rindex;
	bool talking;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)def;
		bool hasDialog;
		if (imp->QueryBoolAttribute("hasDialog", &hasDialog) != XML_NO_ERROR)
			hasDialog = false;

		index = hasDialog ? 0 : 9999;
		rindex = (index == 9999) ? randGet() : index;
		talking = false;
	}
};

// movement styles

/**
 * Causes the entity to hop around.
 */
struct Hop {
	Hop(float r = 0)
		: hopRatio(r) {}

	float hopRatio;
};

/**
 * Causes the entity to wander about.
 */
struct Wander {
	Wander(float ix = 0, float r = 0)
		: initialX(ix), range(r), countdown(0) {}

	float initialX;
	float range;
	int countdown;
};

/**
 * Causes the entity to get mad at the player, charge and fight.
 */
struct Aggro : public Component {
	Aggro(const std::string& a)
		: arena(a) {}
	Aggro(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	std::string arena;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp;
		// TODO null check..?, imp given
		arena = def->StrAttribute("arena");
	}
};

struct Hit : public Component {
	Hit(int d)
		: damage(d) {}

	int damage;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp;
		(void)def;
	}
};

/**
 * SYSTEMS
 */

class MovementSystem : public entityx::System<MovementSystem> {
public:
	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;
};

class PhysicsSystem : public entityx::System<PhysicsSystem> {
public:
	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;
};

class RenderSystem : public entityx::System<RenderSystem> {
private:
	std::string loadTexString;
	Texture loadTexResult;

public:
	Texture loadTexture(const std::string& file);
	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override
	{ (void)en; (void)ev; (void)dt; }
	void render(void);
};

class DialogSystem : public entityx::System<DialogSystem>, public entityx::Receiver<DialogSystem> {
public:
	void configure(entityx::EventManager&);
	void receive(const MouseClickEvent&);
	void update(entityx::EntityManager&, entityx::EventManager&, entityx::TimeDelta) override;
};

#endif //COMPONENTS_HPP
