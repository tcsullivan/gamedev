#ifndef COMPONENTS_DROP_HPP
#define COMPONENTS_DROP_HPP

#include "base.hpp"

#include <inventory.hpp>
#include <random.hpp>

#include <string>
#include <vector>

using DropInfo = std::pair<std::string, int>;

struct Drop : public Component {
	Drop(void) {}
	Drop(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	std::vector<DropInfo> items;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp;
		
		auto dxml = def->FirstChildElement("item");
		while (dxml != nullptr) {
			int min = dxml->IntAttribute("min");
			int max = dxml->IntAttribute("max");
			int count = randGet() % (max - min) + min;

			items.emplace_back(dxml->StrAttribute("name"), count);

			dxml = dxml->NextSiblingElement();
		}
	}

};

#endif // COMPONENTS_DROP_HPP 
