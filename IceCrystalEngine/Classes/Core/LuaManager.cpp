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
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table); // Load standard libraries

    // Register wait function
    lua_register(lua, "wait", LuaWait);
    
    // Replace the global print function with LuaPrint
    lua_pushcfunction(lua, LuaManager::LuaPrint);
    lua_setglobal(lua, "print");


    // Operating on GLM vecs
    auto mult_overloads = sol::overload(
        [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return v1*v2; },
        [](const glm::vec3& v1, float f) -> glm::vec3 { return v1*f; },
        [](float f, const glm::vec3& v1) -> glm::vec3 { return f*v1; }
    );
    lua.new_usertype<glm::vec3>("vec3",
        sol::meta_function::addition, [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return v1 + v2; },
        sol::meta_function::subtraction, [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return v1 - v2; },
        sol::meta_function::division, sol::overload(
            [](const glm::vec3& v1, float f) -> glm::vec3 { return v1 / f; },
            [](float f, const glm::vec3& v1) -> glm::vec3 { return glm::vec3(f / v1.x, f / v1.y, f / v1.z); }
        ),
        sol::meta_function::multiplication, mult_overloads
    );

    RegisterBindings();
}


int LuaManager::LuaWait(lua_State* L)
{
    if (!lua_isnumber(L, 1))
    {
        lua_pushstring(L, "Expected a number as the argument.");
        lua_error(L);
    }

    int milliseconds = lua_tointeger(L, 1);

    // Pause the thread
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));

    return 0;
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

    // Redirect to your custom logging system
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
            static_cast<void (Transform::*)(float, float, float)>(&Transform::LookAt),
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::LookAt)
        ),

        // Additive methods
        "Translate", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::Translate),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::Translate)
        ),
        "Rotate", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::Rotate),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::Rotate)
        ),
        "Scale", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::Scale),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::Scale)
        ),
        "TranslateLocal", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::TranslateLocal),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::TranslateLocal)
        ),
        "RotateLocal", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::RotateLocal),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::RotateLocal)
        ),
        "ScaleLocal", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::ScaleLocal),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::ScaleLocal)
        ),
        // Additive delta methods
        "TranslateDelta", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::TranslateDelta),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::TranslateDelta)
        ),
        "RotateDelta", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::RotateDelta),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::RotateDelta)
        ),
        "ScaleDelta", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::ScaleDelta),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::ScaleDelta)
        ),
        "TranslateLocalDelta", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::TranslateLocalDelta),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::TranslateLocalDelta)
        ),
        "RotateLocalDelta", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::RotateLocalDelta),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::RotateLocalDelta)
        ),
        "ScaleLocalDelta", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::ScaleLocalDelta),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::ScaleLocalDelta)
        ),
        // Delarative methods
        "SetPosition", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::SetPosition),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::SetPosition)
        ),
        "SetRotation", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::SetRotation),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::SetRotation)
        ),
        "SetScale", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::SetScale),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::SetScale)
        ),
        "SetLocalPosition", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::SetLocalPosition),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::SetLocalPosition)
        ),
        "SetLocalRotation", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::SetLocalRotation),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::SetLocalRotation)
        ),
        "SetLocalScale", sol::overload(
            static_cast<void (Transform::*)(glm::vec3)>(&Transform::SetLocalScale),
            static_cast<void (Transform::*)(float, float, float)>(&Transform::SetLocalScale)
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
}
