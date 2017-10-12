#ifndef COMPONENTS_HIT_HPP_
#define COMPONENTS_HIT_HPP_

#include "base.hpp"

#include <attack.hpp>
#include <texture.hpp>

struct Hit : public Component {
	Hit(Attack* a)
		: attack(a) {}

	Attack* attack;
	TextureIterator effect;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp;
		(void)def;
	}
};

#endif // COMPONENTS_HIT_HPP_
