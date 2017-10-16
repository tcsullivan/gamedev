#include <systems/lua.hpp>

void LuaScript::setGlobal(const LuaVariable& nv) const
{
	lua_pushnumber(state, std::get<float&>(nv));
	lua_setglobal(state, std::get<std::string>(nv).c_str());
}

void LuaScript::getReturns(LuaRetList& rets) const
{
	int count = lua_gettop(state);
	for (int i = 1; i <= count; i++)
		rets.emplace_back(lua_tonumber(state, i));
	lua_pop(state, count);
}

void LuaScript::operator()(const std::string& func, LuaList vars) const
{
	for (auto& v : vars)
		setGlobal(v);
	(*this)(func);
	for (auto& v : vars) {
		lua_getglobal(state, std::get<std::string>(v).c_str());
		std::get<float&>(v) = lua_tonumber(state, -1);
	}
}

void LuaScript::operator()(const std::string& func, LuaRetList& rets,
	LuaList vars) const
{
	for (auto& v : vars)
		setGlobal(v);
	(*this)(func);
	getReturns(rets);
	for (auto& v : vars) {
		lua_getglobal(state, std::get<std::string>(v).c_str());
		std::get<float&>(v) = lua_tonumber(state, -1);
	}
}

void LuaScript::operator()(LuaList vars) const
{
	for (auto& v : vars)
		setGlobal(v);
	(*this)();
	for (auto& v : vars) {
		lua_getglobal(state, std::get<std::string>(v).c_str());
		std::get<float&>(v) = lua_tonumber(state, -1);
	}
}

void LuaScript::operator()(LuaRetList& rets, LuaList vars) const
{
	for (auto& v : vars)
		setGlobal(v);
	(*this)();
	getReturns(rets);
	for (auto& v : vars) {
		lua_getglobal(state, std::get<std::string>(v).c_str());
		std::get<float&>(v) = lua_tonumber(state, -1);
	}
}

void LuaScript::operator()(const std::string& s) const
{
	lua_getglobal(state, s.c_str());
	lua_pcall(state, 0, LUA_MULTRET, 0);
}

