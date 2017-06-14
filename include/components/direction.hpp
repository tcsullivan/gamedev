#ifndef COMPONENTS_DIRECTION_HPP_
#define COMPONENTS_DIRECTION_HPP_

#include "base.hpp"

#include <vector2.hpp>

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

#endif // COMPONENTS_DIRECTION_HPP_
