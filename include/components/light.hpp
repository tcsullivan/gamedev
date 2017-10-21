#ifndef COMPONENTS_LIGHT_HPP_
#define COMPONENTS_LIGHT_HPP_

#include <components/base.hpp>
#include <vector2.hpp>
#include <color.hpp>
#include <systems/light.hpp>

struct Illuminate : public Component {
	int index;

	Illuminate(vec2 pos, float radius, Color color) {
		index = LightSystem::addLight(pos, radius, color);
	}
	Illuminate(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp;
		float radius = def->FloatAttribute("radius");
		index = LightSystem::addLight(vec2(), radius, Color(1, 1, 0));
	}
};

#endif // COMPONENTS_LIGHT_HPP_
