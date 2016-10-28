/**
 * @file components.hpp
 * @brief Where all of an enities possible components are stored.
 * Using an ECS (Entity component system) the entities are given components on the fly,
 * this allows the entity to change stats or skills on the go. This also allows every "object"
 * the be an entity, and it gives the game a much better customizability over xml.
 */

#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include <entityx/entityx.h>
#include <common.hpp>
#include <texture.hpp>
#include <events.hpp>

/**
 * @struct Position
 * @brief Stores the position of an entity on the xy plane.
 */
struct Position {
	/**
	 * Constructor that sets the position of the object, if nothing is passed it will default to 0.
	 * @param x The x position the object will be placed at.
	 * @param y the y position the object will be placed at.
	 */
	Position(float x = 0.0f, float y = 0.0f): x(x), y(y) {}

	float x; /**< The x position in the world */
	float y; /**< The y position in the world */
};

/**
 * @struct Direction
 * @brief Store an entities velocity.
 * This allows the entity to move throughout the world.
 */
struct Direction {
	/**
	 * Constructor that sets the velocity, if no position is passed, it defaults to (0,0).
	 * @param x The velocity of the object on the x axis.
	 * @param y The velocity of the object on the y axis.
	 */
	Direction(float x = 0.0f, float y = 0.0f): x(x), y(y) {}

	float x; /**< Velocity the object is moving in the x direction, this is added to the position */
	float y; /**< Velocity the object is moving in the y direction, this is added to the position */
};

/**
 * @struct Physics
 * @brief Allows and entity to react to gravity and frictions.
 * When an entity inherits this component it will react with gravity and move with friction.
 */
struct Physics {
	/**
	 * Constructor that sets the gravity constant, if not specified it becomes 0.
	 * @param g The non default gravity constant.
	 */
	Physics(float g = 0.0f): g(g) {}

	float g; /**< The gravity constant, how fast the object falls */
};

/**
 * @struct Grounded
 * @brief Places an entity without physics on the ground.
 * This is used so we don't have to update the physics of a non-moving object every loop.
 */
struct Grounded {
	//TODO possibly make a way to change this
	bool grounded = false;
};

/**
 * @struct Health
 * @brief Gives and entity health and stuff.
 */
struct Health {
	/**
	 * Constructor that sets the variables, with 0 health as default.
	 */
	Health(int h = 0, int m = 0) : health(h), maxHealth(m) {}

	int health;
	int maxHealth;
};

struct Portal {
	Portal(std::string tf = "") : toFile(tf) {}

	std::string toFile;
};

struct Name {
	Name(std::string n = "") : name(n) {}

	std::string name;
};

/**
 * @struct Solid
 * @brief Allows an entity to collide with other objects.
 * When an entity has this component it can collide with the world and other objects.
 */
struct Solid {
	/**
	 * Constructor that sets the entities dimensions based on what is passed.
	 * @param w The desired width of the entity.
	 * @param h The desired height of the entity.
	 */
	Solid(float w = 0.0f, float h = 0.0f): width(w), height(h) {offset = 0.0f; passable = true; }
	//Solid(float w = 0.0f, float h = 0.0f, vec2 offset = 0.0f): width(w), height(h), offset(offset) {passable = true; }
	
	void Passable(bool v) {passable = v;}
	bool Passable(void)   {return passable;}

	float width; /**< The width of the entity in units */
	float height; /**< The height of the entity in units */
	vec2 offset; /**< This allows us to make the hitbox in any spot */
	bool passable; /**< This determines whether or not one can pass by the entity */
};

struct SpriteData {
	
	SpriteData(std::string path, vec2 offset): 
		offset(offset) {
			pic = Texture::loadTexture(path);
			size = Texture::imageDim(path);
		}
	
	GLuint pic;
	vec2 offset;
	vec2 size;
};

//TODO
/**
 * @struct Sprite
 * @brief If an entity is visible we want to be able to see it.
 * Each entity is given a sprite, a sprite can consist of manu frames or pieces to make one.
 */
struct Sprite {
	Sprite(bool left = false)
	 	: faceLeft(left) {}

	std::vector<std::pair<SpriteData, vec2>> getSprite() {
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
		vec2 st; /** the start location of the sprite */
		vec2 dim; /** how wide the sprite is */
		
		if (sprite.size()) {
			st.x = sprite[0].second.x;
			st.y = sprite[0].second.y;
		} else {
			return vec2(0.0f, 0.0f);
		}

		for (auto &s : sprite) {
			if (s.second.x < st.x)
				st.x = s.second.x;
			if (s.second.y < st.y)
				st.y = s.second.y;

			if (s.second.x + s.first.size.x > dim.x)
				dim.x = s.second.x + s.first.size.x;
			if (s.second.y + s.first.size.y > dim.y)
				dim.y = s.second.y + s.first.size.y;
		}

		return dim;
	}

	std::vector<std::pair<SpriteData, vec2>> sprite;
	bool faceLeft;
};

//TODO
struct Animate {
	std::vector<std::pair<SpriteData, vec2>> sprite_e;
	std::vector<std::pair<SpriteData, vec2>> sprite_c;
};

//TODO
struct Input {

};

/**
 * @struct Visible
 * @brief If an entity is visible we want to be able to draw it.
 */
struct Visible {
	/**
	 * @brief Decide what layer to draw the entity on.
	 * When stuff is drawn, it is drawn on a "layer". This layer gives more of a 3D effect to the world.
	 * @param z The desired "layer" of the entity.
	 */
	Visible(float z = 0.0f): z(z) {}

	float z; /**< The value along the z axis the entity will be drawn on */
};

struct Dialog {
	Dialog(int idx = 0)
		: index(idx) {}

	int index;
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
public:
	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;
};

class DialogSystem : public entityx::System<DialogSystem>, public entityx::Receiver<DialogSystem> {
public:
	void configure(entityx::EventManager&);
	void receive(const MouseClickEvent&);
	void update(entityx::EntityManager&, entityx::EventManager&, entityx::TimeDelta) override;
};

#endif //COMPONENTS_HPP
