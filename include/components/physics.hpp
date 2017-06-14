#ifndef COMPONENTS_PHYSICS_HPP_
#define COMPONENTS_PHYSICS_HPP_

#include "base.hpp"

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
	Physics(float g = 0.2f): g(g) {}
	Physics(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	float g; /**< The gravity constant, how fast the object falls */

	void fromXML(XMLElement* imp, XMLElement* def) final {
		if (imp->QueryFloatAttribute("gravity", &g) != XML_NO_ERROR) {
			if (def->QueryFloatAttribute("value", &g) != XML_NO_ERROR)
				g = 0.2f;
		}
	}
};

#endif // COMPONENTS_PHYSICS_HPP_
