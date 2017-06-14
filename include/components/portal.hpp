#ifndef COMPONENTS_PORTAL_HPP_
#define COMPONENTS_PORTAL_HPP_

#include "base.hpp"

#include <string>

struct Portal : public Component {
	Portal(std::string tf = "")
		: toFile(tf) {}
	Portal(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	std::string toFile;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)def;
		toFile = imp->StrAttribute("inside");
	}
};

#endif // COMPONENTS_PORTAL_HPP_
