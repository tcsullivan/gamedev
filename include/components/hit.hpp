#ifndef COMPONENTS_HIT_HPP_
#define COMPONENTS_HIT_HPP_

#include "base.hpp"

#include <texture.hpp>

struct Hit : public Component {
	Hit(int d, bool p = false)
		: damage(d), pierce(p) {}

	int damage;
	bool pierce;
	TextureIterator effect;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp;
		(void)def;
	}
};

#endif // COMPONENTS_HIT_HPP_
