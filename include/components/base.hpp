#ifndef COMPONENTS_BASE_HPP_
#define COMPONENTS_BASE_HPP_

#include <entityx/entityx.h>
#include <tinyxml2.h>

using namespace tinyxml2;

/**
 * @class Component
 * @brief A base class for all components, insures all components have similar
 * base functionalities.
 */
class Component : public entityx::Component<Component> {
public:
	/**
	 * Constructs the component from the two given XML tags.
	 *
	 * Components can get information from two places: where the entity is defined
	 * (it's implementation, e.g. in town.xml) or from the tag's definition (e.g. entities.xml).
	 * The definition tag should be used for default values.
	 *
	 * @param imp tag for the implementation of the entity
	 * @param def tag for the definition of the component
	 */
	virtual void fromXML(XMLElement* imp, XMLElement* def) = 0;
};

#endif // COMPONENTS_BASE_APP_
