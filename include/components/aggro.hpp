#ifndef COMPONENTS_AGGRO_HPP_
#define COMPONENTS_AGGRO_HPP_

#include "base.hpp"

/**
 * Causes the entity to get mad at the player, charge and fight.
 */
struct Aggro : public Component {
	Aggro(bool y = false)
		: yes(y) {}
	Aggro(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	bool yes;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp, (void)def;
		// TODO
	}
};

#endif // COMPONENTS_AGGRO_HPP_
