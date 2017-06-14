#ifndef COMPONENTS_NAME_HPP_
#define COMPONENTS_NAME_HPP_

#include "base.hpp"

#include <string>

struct Name : public Component {
	Name(std::string n = "")
		: name(n) {}
	Name(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	std::string name;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		auto n = imp->Attribute("name");

		// TODO check def's nullness	
		name = n != nullptr ? n : def->Attribute("value");
	}
};
#endif // COMPONENTS_NAME_HPP_
