#ifndef COMPONENTS_DAMAGE_HPP_
#define COMPONENTS_DAMAGE_HPP_

#include "base.hpp"

struct Damage : public Component {
	Damage(int p = 0)
		: pain(p) {}
	Damage(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	int pain;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp;
		if (def->QueryIntAttribute("value", &pain) != XML_NO_ERROR)
			pain = 0;
	}
};

#endif // COMPONENTS_DAMAGE_HPP_
