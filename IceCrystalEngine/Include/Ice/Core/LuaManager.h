#pragma once

#ifndef LUA_MANAGER_H
#define LUA_MANAGER_H

#include <sol/sol.hpp>
#include <lua/lua.hpp>
#include <thread>

#include "Ice/Components/LuaExecutor.h"

struct ComponentRegistryEntry {
	std::function<sol::object(Actor&)> addFn;
	std::function<sol::object(Actor&)> getFn;
	std::function<bool(Actor&)> hasFn;
};
static std::unordered_map<std::string, ComponentRegistryEntry> componentRegistry;


class LuaManager
{
public:
	sol::state lua;

	struct LuaTask {
		sol::thread thread;
		sol::coroutine co;
		sol::environment env;
		double wakeTime = 0.0;
	};
	std::vector<LuaTask> tasks;

	static LuaManager& GetInstance()
	{
		static LuaManager instance; // Static local variable ensures a single instance
		return instance;
	}

	void Update(double now);

	// Run a LuaExecutor
	static void RunExecutor(LuaExecutor* executor)
	{
		auto& manager = LuaManager::GetInstance();
		auto& lua = manager.lua;

		sol::environment env(lua, sol::create, lua.globals());
		env["actor"] = executor->owner;
		env["transform"] = executor->transform;

		sol::load_result loader = lua.load_file(executor->filePath);

		if (!loader.valid()) {
			sol::error e = loader;
			fprintf(stderr, "Executor Load Error: %s\n", e.what());
			return;
		}
		
		manager.StartCoroutine(env, std::move(loader));
	}


	// Run a Lua file on a separate thread
	static void RunFile(const char* filePath)
	{
		auto& manager = LuaManager::GetInstance();
		auto& lua = manager.lua;

		sol::environment env(lua, sol::create, lua.globals());

		
		sol::load_result loader = lua.load_file(filePath);

		if (!loader.valid()) {
			sol::error e = loader;
			fprintf(stderr, "Lua Load Error: %s\n", e.what());
			return;
		}

		manager.StartCoroutine(env, std::move(loader));
	}


	// Run a Lua string on a separate thread
	static void RunString(const char* string)
	{
		auto& manager = LuaManager::GetInstance();
		auto& lua = manager.lua;

		sol::environment env(lua, sol::create, lua.globals());

		sol::load_result loader = lua.load(string);

		if (!loader.valid()) {
			sol::error e = loader;
			fprintf(stderr, "Lua Load Error: %s\n", e.what());
			return;
		}

		manager.StartCoroutine(env, std::move(loader));
	}


	template<typename T>
	void RegisterComponent(const std::string& name, sol::state_view lua) {
		componentRegistry[name] = {
			// Add<T>
			[](Actor& a) -> sol::object {
				T* comp = a.AddComponent<T>();
				return sol::make_object(LuaManager::GetInstance().lua, comp);
			},
			// Get<T>
			[](Actor& a) -> sol::object {
				if (T* comp = a.GetComponent<T>())
					return sol::make_object(LuaManager::GetInstance().lua, comp);
				return sol::lua_nil;
			},
			// Has<T>
			[](Actor& a) -> bool {
				return a.HasComponent<T>();
			}
		};
	}

	void Cleanup()
	{
		tasks.clear();
		tasks.shrink_to_fit();

		componentRegistry.clear();

		lua.collect_garbage();
		lua.collect_garbage();
	}

private:
	void RegisterBindings(); // Bind C++ classes and functions to Lua
	void RegisterInputBindings();

	static int LuaWait(lua_State* L);
	static int LuaPrint(lua_State* L);
	
	void StartCoroutine(sol::environment env, sol::load_result&& fx)
	{
		// Attach the environment to the loaded function
		sol::function f = fx;
		sol::set_environment(env, f);

		// Create a coroutine from the main state, with the proper environment
		sol::coroutine co = sol::coroutine(lua, f);

		LuaTask task;
		task.env = std::move(env);
		task.co = std::move(co);

		tasks.push_back(std::move(task));
	}

	LuaManager();

	LuaManager(LuaManager const&) = delete; // Delete copy constructor
	void operator=(LuaManager const&) = delete; // Delete assignment operator
};

#endif
