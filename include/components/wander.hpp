#ifndef COMPONENTS_WANDER_HPP_
#define COMPONENTS_WANDER_HPP_

#include <string>

#include <systems/lua.hpp>

/**
 * Causes the entity to wander about.
 */
struct Wander {
	Wander(const std::string& s = "")
		: script(LuaSystem::makeScript(s)) {}

	LuaScript script;
};

#endif // COMPONENTS_WANDER_HPP_
