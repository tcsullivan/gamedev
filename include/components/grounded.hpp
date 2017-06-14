#ifndef COMPONENTS_GROUNDED_HPP_
#define COMPONENTS_GROUNDED_HPP_

#include "base.hpp"

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

#endif // COMPONENTS_GROUNDED_HPP_
