#ifndef COMPONENTS_POSITION_HPP_
#define COMPONENTS_POSITION_HPP_

#include "base.hpp"

#include <vector2.hpp>

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

#endif // COMPONENTS_POSITION_HPP_
