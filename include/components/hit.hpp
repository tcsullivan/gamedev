#ifndef COMPONENTS_HIT_HPP_
#define COMPONENTS_HIT_HPP_

#include "base.hpp"

struct Hit : public Component {
	Hit(int d, bool p = false)
		: damage(d), pierce(p) {}

	int damage;
	bool pierce;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp;
		(void)def;
	}
};

#endif // COMPONENTS_HIT_HPP_
