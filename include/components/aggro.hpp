#ifndef COMPONENTS_AGGRO_HPP_
#define COMPONENTS_AGGRO_HPP_

#include "base.hpp"

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

#endif // COMPONENTS_AGGRO_HPP_
