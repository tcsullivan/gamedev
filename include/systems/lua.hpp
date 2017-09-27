#ifndef SYSTEMS_LUA_HPP_
#define SYSTEMS_LUA_HPP_

#include <string>
#include <lua.hpp>

#include <vector2.hpp>

class LuaScript {
private:
	lua_State* state;
	std::string script;

public:
	LuaScript(const std::string& sc = "")
		: script(sc) {
		state = luaL_newstate();
		luaL_openlibs(state);
		luaL_loadstring(state, script.c_str());
		lua_pcall(state, 0, 0, 0);
	}

	inline auto operator()(void) {
		lua_getglobal(state, "update");
		lua_pcall(state, 0, LUA_MULTRET, 0);
		if (lua_gettop(state) != 2)
			return vec2();
		vec2 ret (lua_tonumber(state, 1), lua_tonumber(state, 2));
		lua_pop(state, 2);
		return ret;
	}
};

class LuaSystem {
private:
	class LuaInterpreter {
	private:
		lua_State* state;

	public:
		LuaInterpreter(void) {
			state = luaL_newstate();
			luaL_openlibs(state);
		}

		void registerFunc(const std::string& name, lua_CFunction func) {
			lua_pushcclosure(state, func, 0);
			lua_setglobal(state, name.c_str());
		}

		void loadFile(const std::string& name) {
			luaL_dofile(state, name.c_str());
			lua_pcall(state, 0, 0, 0); // 'prime' the file
		}

		void runFunc(const std::string& name) {
			lua_getglobal(state, name.c_str());
			lua_pcall(state, 0, 0, 0);
		}
	};

public:
	inline static LuaScript makeScript(const std::string& s) {
		return LuaScript(s);
	}
};

#endif // SYSTEMS_LUA_HPP_
