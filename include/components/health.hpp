#ifndef COMPONENTS_HEALTH_HPP_
#define COMPONENTS_HEALTH_HPP_

#include "base.hpp"

#include <SDL2/SDL_mixer.h>

/**
 * @struct Health
 * @brief Gives and entity health and stuff.
 */
struct Health : public Component {
	/**
	 * Constructor that sets the variables, with 1 health as default.
	 */
	Health(int m = 1, int h = 0)
		: health(h != 0 ? h : m), maxHealth(m) {}
	Health(XMLElement* imp, XMLElement* def) {
		fromXML(imp, def);
	}

	int health; /**< The current amount of health */
	int maxHealth; /**< The maximum amount of health */
	Mix_Chunk* ouch; /**< Sound made when attacked */

	void fromXML(XMLElement* imp, XMLElement* def) final {
		(void)imp;
		(void)def;
		// TODO
		if (def->QueryIntAttribute("value", &health) != XML_NO_ERROR)
			health = 1;
		maxHealth = health;
		auto o = def->Attribute("ouch");
		if (o != nullptr)
			ouch = Mix_LoadWAV(o);
		else
			ouch = nullptr;
	}
};


#endif // COMPONENTS_HEALTH_HPP_
