#include <systems/lua.hpp>

void LuaScript::setGlobal(const LuaVariable& nv)
{
	lua_pushnumber(state, std::get<float&>(nv));
	lua_setglobal(state, std::get<std::string>(nv).c_str());
}

void LuaScript::operator()(std::vector<LuaVariable> vars)
{
	for (auto& v : vars)
		setGlobal(v);
	(*this)();
	for (auto& v : vars) {
		lua_getglobal(state, std::get<std::string>(v).c_str());
		std::get<float&>(v) = lua_tonumber(state, -1);
	}
}

void LuaScript::operator()(void)
{
	lua_getglobal(state, "update");
	lua_pcall(state, 0, LUA_MULTRET, 0);
}

