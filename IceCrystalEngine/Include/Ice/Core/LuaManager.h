#pragma once

#ifndef LUA_MANAGER_H
#define LUA_MANAGER_H

#include <sol/sol.hpp>
#include <lua/lua.hpp>
#include <thread>

class LuaManager
{
public:
	sol::state lua;

	static LuaManager& GetInstance()
	{
		static LuaManager instance; // Static local variable ensures a single instance
		return instance;
	}

	// Run a Lua file on a separate thread
	static void RunFile(const char* filePath)
	{
		std::thread([filePath]() {
			try {
				LuaManager::GetInstance().lua.script_file(filePath);
			} catch (const sol::error& e) {
				fprintf(stderr, "Error running file: %s\n", e.what());
			}
		}).detach(); // Detach the thread to run independently
	}

	// Run a Lua string on a separate thread
	static void RunString(const char* string)
	{
		std::thread([string]() {
			try {
				LuaManager::GetInstance().lua.script(string);
			} catch (const sol::error& e) {
				fprintf(stderr, "Error running string: %s\n", e.what());
			}
		}).detach(); // Detach the thread to run independently
	}

private:
	void RegisterBindings(); // Bind C++ classes and functions to Lua

	static int LuaWait(lua_State* L);
	static int LuaPrint(lua_State* L);
	
	LuaManager();
	// ~LuaManager();

	LuaManager(LuaManager const&) = delete; // Delete copy constructor
	void operator=(LuaManager const&) = delete; // Delete assignment operator
};

#endif
