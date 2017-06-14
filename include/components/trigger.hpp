#ifndef COMPONENTS_TRIGGER_HPP_
#define COMPONENTS_TRIGGER_HPP_

#include "base.hpp"

#include <string>

struct Trigger : public Component {
	Trigger(const std::string& t)
		: text(t) {}
	Trigger(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	std::string text;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp;
		(void)def;
		text = "You got me!";
	}
};


#endif // COMPONENTS_TRIGGER_HPP_
