#include <chrono>
#include <Ice/Core/LuaManager.h>
#include <iostream>
#include <sstream>

#include "Ice/Utils/DebugUtil.h"

#include "Ice/core/SceneManager.h"
#include "Ice/Core/Actor.h"
#include "Ice/Core/Transform.h"
#include "Ice/core/Component.h"

LuaManager::LuaManager()
{
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::package, sol::lib::table); // Load standard libraries

    // Register wait function
    lua_register(lua, "wait", LuaWait);
    
    // Replace the global print function with LuaPrint
    lua_pushcfunction(lua, LuaManager::LuaPrint);
    lua_setglobal(lua, "print");

    RegisterBindings();
}

void LuaManager::Update(double now)
{
    for (size_t i = 0; i < tasks.size(); )
    {
        auto& task = tasks[i];

        // If waiting, skip until time is up
        if (task.wakeTime > now)
        {
            ++i;
            continue;
        }

        // Resume the coroutine
        sol::protected_function_result result = task.co();

        if (result.status() == sol::call_status::yielded)
        {
            // Coroutine yielded → check if it yielded a number (from wait())
            if (lua_gettop(result.lua_state()) >= 1 && lua_isnumber(result.lua_state(), -1))
            {
                double seconds = lua_tonumber(result.lua_state(), -1);
                task.wakeTime = now + seconds;
            }
            else
            {
                // Yielded but no delay specified → resume next frame
                task.wakeTime = now;
            }
            ++i; // Keep task alive
        }
        else if (result.status() == sol::call_status::ok)
        {
            // Finished normally
            tasks.erase(tasks.begin() + i);
            // Do NOT increment i
        }
        else
        {
            // Runtime error
            sol::error err = result;
            printf("Lua Error: %s\n", err.what());
            tasks.erase(tasks.begin() + i);
            // Do NOT increment i
        }
    }
}




int LuaManager::LuaWait(lua_State* L)
{
    // expects milliseconds
    if (!lua_isnumber(L, 1))
    {
        return luaL_error(L, "Expected a number (seconds)");
    }

    double ms = luaL_checknumber(L, 1);
    double seconds = ms / 1000.0;

    // Push the duration so the scheduler can pick it up
    lua_pushnumber(L, seconds);

    // Yield the coroutine, returning 1 value
    return lua_yield(L, 1);
}


int LuaManager::LuaPrint(lua_State* L) {
    int nargs = lua_gettop(L); // Number of arguments
    std::stringstream output;

    // Use Lua's internal `print` formatting for type handling
    for (int i = 1; i <= nargs; i++) {
        const char* str = luaL_tolstring(L, i, NULL); // Converts any type to a string
        output << str;
        lua_pop(L, 1); // Remove the string result from the stack

        if (i < nargs) {
            output << "\t"; // Add tab between arguments
        }
    }
    output << "\n";

    // Redirect to logging system
    DebugUtil::GetInstance().ss << output.str();

    return 0; // Return no values to Lua
}


void LuaManager::RegisterBindings() {

    // SceneManager
    lua.new_usertype<SceneManager>("SceneManager",
        sol::no_constructor, // Prevent creating new instances
        "GetInstance", &SceneManager::GetInstance,
        
        // Properties
        "deltaTime", sol::readonly_property(&SceneManager::deltaTime), // Read-only

        // Methods
        "GetActorCount", &SceneManager::GetActorCount,
        "GetHoveredActor", &SceneManager::GetHoveredActor,
        "GetActorByTag", &SceneManager::GetActorByTag,
        "GetActorsByTag", &SceneManager::GetActorsByTag
    );
    
    // Register Transform
    lua.new_usertype<Transform>("Transform",
        sol::constructors<Transform(Actor*)>(),

        // Member variables
        "actor", &Transform::actor,
        
        "position", &Transform::position,
        "rotation", &Transform::rotation,
        "eulerAngles", &Transform::eulerAngles,
        "scale", &Transform::scale,

        "localPosition", &Transform::localPosition,
        "localRotation", &Transform::localRotation,
        "localEulerAngles", &Transform::localEulerAngles,
        "localScale", &Transform::localScale,

        "forward", &Transform::forward,
        "up", &Transform::up,
        "right", &Transform::right,

        // Parent-child relationships
        "parent", &Transform::parent,
        "children", &Transform::children,

        // Methods
        "SetParent", &Transform::SetParent,
        "LookAt", sol::overload(
            [](Transform& self, float x, float y, float z) {self.LookAt(x, y, z);},
            [](Transform& self, glm::vec3 vec3) {self.LookAt(vec3);}
        ),

        // Additive methods
        "Translate", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.Translate(vec3);},
            [](Transform& self, float x, float y, float z) {self.Translate(x, y, z);}
        ),
        "Rotate", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.Rotate(vec3);},
            [](Transform& self, float x, float y, float z) {self.Rotate(x, y, z);}
        ),
        "Scale", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.Scale(vec3);},
            [](Transform& self, float x, float y, float z) {self.Scale(x, y, z);}
        ),
        "TranslateLocal", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.TranslateLocal(vec3);},
            [](Transform& self, float x, float y, float z) {self.TranslateLocal(x, y, z);}
        ),
        "RotateLocal", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.RotateLocal(vec3);},
            [](Transform& self, float x, float y, float z) {self.RotateLocal(x, y, z);}
        ),
        "ScaleLocal", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.ScaleLocal(vec3);},
            [](Transform& self, float x, float y, float z) {self.ScaleLocal(x, y, z);}
        ),
        // Additive delta methods
        "TranslateDelta", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.TranslateDelta(vec3);},
            [](Transform& self, float x, float y, float z) {self.TranslateDelta(x, y, z);}
        ),
        "RotateDelta", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.RotateDelta(vec3);},
            [](Transform& self, float x, float y, float z) {self.RotateDelta(x, y, z);}
        ),
        "ScaleDelta", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.ScaleDelta(vec3);},
            [](Transform& self, float x, float y, float z) {self.ScaleDelta(x, y, z);}
        ),
        "TranslateLocalDelta", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.TranslateLocalDelta(vec3);},
            [](Transform& self, float x, float y, float z) {self.TranslateLocalDelta(x, y, z);}
        ),
        "RotateLocalDelta", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.RotateLocalDelta(vec3);},
            [](Transform& self, float x, float y, float z) {self.RotateLocalDelta(x, y, z);}
        ),
        "ScaleLocalDelta", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.ScaleLocalDelta(vec3);},
            [](Transform& self, float x, float y, float z) {self.ScaleLocalDelta(x, y, z);}
        ),
        // Delarative methods
        "SetPosition", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.SetPosition(vec3);},
            [](Transform& self, float x, float y, float z) {self.SetPosition(x, y, z);}
        ),
        "SetRotation", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.SetRotation(vec3);},
            [](Transform& self, float x, float y, float z) {self.SetRotation(x, y, z);}
        ),
        "SetScale", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.SetScale(vec3);},
            [](Transform& self, float x, float y, float z) {self.SetScale(x, y, z);}
        ),
        "SetLocalPosition", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.SetLocalPosition(vec3);},
            [](Transform& self, float x, float y, float z) {self.SetLocalPosition(x, y, z);}
        ),
        "SetLocalRotation", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.SetLocalRotation(vec3);},
            [](Transform& self, float x, float y, float z) {self.SetLocalRotation(x, y, z);}
        ),
        "SetLocalScale", sol::overload(
            [](Transform& self, glm::vec3 vec3) {self.SetLocalScale(vec3);},
            [](Transform& self, float x, float y, float z) {self.SetLocalScale(x, y, z);}
        )
    );
    
    // Register Actor
    lua.new_usertype<Actor>("Actor",
        sol::constructors<
            Actor(const std::string&, const std::string&),
            Actor(const std::string&),
            Actor()
        >(),

        // Member variables
        "name", &Actor::name,
        "tag", &Actor::tag,
        "transform", &Actor::transform,

        // Methods
        "HasComponent", [](Actor& self, const std::string& componentType) -> bool {
            // Example: Dynamically checking components
            return self.HasComponent<Component>(); // Replace with appropriate type lookup logic
        },

        "AddComponent", [](Actor& self, const std::string& componentType) -> Component* {
            // Example: Dynamically adding components
            return self.AddComponent<Component>(); // Replace with appropriate type logic
        },

        "GetComponent", [](Actor& self, const std::string& componentType) -> Component* {
            // Example: Dynamically getting components
            return self.GetComponent<Component>(); // Replace with appropriate type logic
        }
    );





    // Operating on GLM vecs
    lua.new_usertype<glm::vec3>("vec3",
        sol::call_constructor, sol::factories(
            []() { return glm::vec3(0.0f); },
            [](float x, float y, float z) { return glm::vec3(x, y, z); },
            [](float s) { return glm::vec3(s); }
        ),

        // Allow `.x .y .z` fields
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z,

        // Operators
        sol::meta_function::addition, [](const glm::vec3& a, const glm::vec3& b) {
            return a + b;
        },

        sol::meta_function::subtraction, [](const glm::vec3& a, const glm::vec3& b) {
            return a - b;
        },

        sol::meta_function::multiplication, sol::overload(
            [](const glm::vec3& a, const glm::vec3& b) { return a * b; },
            [](const glm::vec3& a, float f) { return a * f; },
            [](float f, const glm::vec3& a) { return f * a; }
        ),

        sol::meta_function::division, sol::overload(
            [](const glm::vec3& a, float f) { return a / f; },
            [](float f, const glm::vec3& a) {
                return glm::vec3(f / a.x, f / a.y, f / a.z);
            }
        )
    );
}
