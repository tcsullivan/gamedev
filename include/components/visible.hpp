#ifndef COMPONENTS_VISIBLE_HPP_
#define COMPONENTS_VISIBLE_HPP_

#include "base.hpp"

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

#endif // COMPONENTS_VISIBLE_HPP_
