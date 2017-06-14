#ifndef COMPONENTS_SOLID_HPP_
#define COMPONENTS_SOLID_HPP_

#include "base.hpp"

#include <vector2.hpp>

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

#endif // COMPONENTS_SOLID_HPP_
