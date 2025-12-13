#pragma once

#ifndef LUA_MANAGER_H
#define LUA_MANAGER_H

#include <sol/sol.hpp>
#include <lua/lua.hpp>
#include <thread>

#include "Ice/Components/LuaExecutor.h"

#pragma comment(lib, "lua54.lib")

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

	struct LuaTask
	{
		sol::thread thread;
		sol::environment env;
		sol::coroutine co;
		double wakeTime;
	};
	std::vector<LuaTask> tasks;

	static LuaManager& GetInstance()
	{
		static LuaManager instance; // Static local variable ensures a single instance
		return instance;
	}

	void Update(double now);

	// Run a LuaExecutor
	void RunExecutor(LuaExecutor* executor)
	{
		auto& manager = LuaManager::GetInstance();
		auto& lua = manager.lua;

		sol::thread thread = sol::thread::create(lua);
		sol::state_view thread_lua = thread.state();
		
		sol::function f = thread_lua.load_file(executor->filePath);

		sol::environment env(thread_lua, sol::create, lua.globals());
		env["actor"] = executor->owner;
		env["transform"] = executor->transform;
		
		sol::set_environment(env, f);
		
		sol::coroutine co(thread_lua, f);

		LuaTask task;
		task.thread = thread;
		task.env = std::move(env);
		task.co = std::move(co);
		task.wakeTime = 0.0;
		tasks.push_back(std::move(task));
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

	LuaManager();

	LuaManager(LuaManager const&) = delete; // Delete copy constructor
	void operator=(LuaManager const&) = delete; // Delete assignment operator
};

class RunService
{
public:
	static RunService& GetInstance()
	{
		static RunService instance;
		return instance;
	}

	void FireUpdate(float deltaTime);
	void FireFixedUpdate(float fixedDeltaTime);
	void FireLateUpdate(float deltaTime);

	void ConnectUpdate(sol::function callback);
	void ConnectFixedUpdate(sol::function callback);
	void ConnectLateUpdate(sol::function callback);
private:
	RunService() = default;

	std::vector<sol::function> updateCallbacks;
	std::vector<sol::function> fixedUpdateCallbacks;
	std::vector<sol::function> lateUpdateCallbacks;
};

#endif
