#ifndef SYSTEMS_LUA_HPP_
#define SYSTEMS_LUA_HPP_

#include <string>
#include <lua.hpp>
#include <tuple>
#include <vector>

using LuaVariable = std::tuple<std::string, float&>;

class LuaScript {
private:
	lua_State* state;
	std::string script;

	void setGlobal(const LuaVariable&);

public:
	LuaScript(const std::string& sc = "")
		: script(sc) {
		state = luaL_newstate();
		luaL_openlibs(state);
		luaL_loadstring(state, script.c_str());
		lua_pcall(state, 0, 0, 0);
	}

	void operator()(std::vector<LuaVariable> vars);
	void operator()(void);
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
