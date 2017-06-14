#ifndef COMPONENTS_DIALOG_HPP_
#define COMPONENTS_DIALOG_HPP_

#include "base.hpp"

#include <random.hpp>

struct Dialog : public Component {
	Dialog(int idx = 0)
		: index(idx), rindex((idx == 9999) ? randGet() : idx), talking(false) {}
	Dialog(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	int index;
	int rindex;
	bool talking;

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)def;
		bool hasDialog;
		if (imp->QueryBoolAttribute("hasDialog", &hasDialog) != XML_NO_ERROR)
			hasDialog = false;

		index = hasDialog ? 0 : 9999;
		rindex = (index == 9999) ? randGet() : index;
		talking = false;
	}
};

#endif // COMPONENTS_DIALOG_HPP_
