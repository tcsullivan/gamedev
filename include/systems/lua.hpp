#ifndef SYSTEMS_LUA_HPP_
#define SYSTEMS_LUA_HPP_

#include <string>
#include <lua.hpp>
#include <tuple>
#include <vector>

using LuaVariable = std::tuple<std::string, float&>;
using LuaList = std::vector<LuaVariable>;
using LuaRetList = std::vector<double>;

class LuaScript {
private:
	lua_State* state;
	std::string script;

	void setGlobal(const LuaVariable&) const;
	void getReturns(LuaRetList& rets) const;

	static void replace(std::string& s, const std::string& rid, const std::string& put) {
		for (unsigned int i = 0; i < s.size(); i++) {
			if (s.substr(i, rid.size()) == rid) {
				s.replace(i, rid.size(), put);
				i += put.size() - 1;
			}
		}
	}

public:
	LuaScript(const std::string& sc = "")
		: script(sc) {
		state = luaL_newstate();
		luaL_openlibs(state);
		replace(script, "&lt;", "<");
		replace(script, "&gt;", ">");
		luaL_loadstring(state, script.c_str());
		lua_pcall(state, 0, 0, 0);
	}

	inline lua_State* getState(void)
	{ return state; }

	inline void addFunction(const std::string& name, lua_CFunction func) {
		lua_pushcclosure(state, func, 0);
		lua_setglobal(state, name.c_str());
	}

	void operator()(const std::string& func = "update") const;
	void operator()(const std::string& func, LuaList vars) const;
	void operator()(const std::string& func, LuaRetList& rets, LuaList vars) const;
	void operator()(LuaList vars) const;
	void operator()(LuaRetList& rets, LuaList vars) const;
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
