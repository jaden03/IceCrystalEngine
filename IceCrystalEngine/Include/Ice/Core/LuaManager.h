#pragma once

#ifndef LUA_MANAGER_H
#define LUA_MANAGER_H


#include <lua/lua.hpp>

class LuaManager
{
public:

	lua_State* L;
	static LuaManager& GetInstance()
	{
		static LuaManager instance; // Static local variable ensures a single instance
		return instance;
	}
	static void RunFile(const char* filePath)
	{
		if (luaL_dofile(LuaManager::GetInstance().L, filePath) != LUA_OK) {
			const char* error = lua_tostring(LuaManager::GetInstance().L, -1);
			// Log or handle the error
			fprintf(stderr, "Error running file: %s\n", error);
			lua_pop(LuaManager::GetInstance().L, 1); // Remove error message from stack
		}
	}
	static void RunString(const char* string)
	{
		if (luaL_dostring(LuaManager::GetInstance().L, string) != LUA_OK) {
			const char* error = lua_tostring(LuaManager::GetInstance().L, -1);
			// Log or handle the error
			fprintf(stderr, "Error running string: %s\n", error);
			lua_pop(LuaManager::GetInstance().L, 1); // Remove error message from stack
		} 
	}
	

private:

	void InitializeLuaState();
	static int LuaPrint(lua_State* L);

	LuaManager(); // Private constructor to ensure a single instance
	~LuaManager();

	LuaManager(LuaManager const&) = delete; // Delete copy constructor
	void operator=(LuaManager const&) = delete; // Delete assignment operator
};

#endif