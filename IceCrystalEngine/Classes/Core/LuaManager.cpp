#include <chrono>
#include <Ice/Core/LuaManager.h>
#include <iostream>
#include <sstream>

#include "Ice/Components/Camera.h"
#include "Ice/Core/Component.h"

#include "Ice/Components/Light.h"
#include "Ice/Utils/DebugUtil.h"

#include "Ice/Core/Actor.h"
#include "Ice/Core/Transform.h"
#include "Ice/core/SceneManager.h"
#include <Ice/Core/Input.h>

#include "Ice/Components/Physics/RigidBody.h"

LuaManager::LuaManager()
{
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::package, sol::lib::table); // Load standard libraries

    // Register wait function
    lua["wait"] = &LuaManager::LuaWait;

    // Replace the global print function
    lua["print"] = &LuaManager::LuaPrint;

    // Add type_name helper
    lua["type_name"] = [](sol::object obj) -> std::string {
        if (!obj.is<sol::table>()) return obj.get_type() == sol::type::nil ? "nil" : "primitive";
        sol::table mt = obj.as<sol::table>().get<sol::table>(sol::metatable_key);
        if (!mt.valid()) return "table";
        sol::optional<std::string> name = mt["__name"];
        return name ? *name : "table";
    };

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


void LuaManager::RegisterInputBindings()
{
#pragma region Keycodes
    sol::table keycode_table = lua.create_table();
    
    // Printable keys + symbols
    keycode_table["Space"]         = GLFW_KEY_SPACE;
    keycode_table["Apostrophe"]    = GLFW_KEY_APOSTROPHE;
    keycode_table["Comma"]         = GLFW_KEY_COMMA;
    keycode_table["Minus"]         = GLFW_KEY_MINUS;
    keycode_table["Period"]        = GLFW_KEY_PERIOD;
    keycode_table["Slash"]         = GLFW_KEY_SLASH;
    keycode_table["Semicolon"]     = GLFW_KEY_SEMICOLON;
    keycode_table["Equal"]         = GLFW_KEY_EQUAL;
    keycode_table["LeftBracket"]   = GLFW_KEY_LEFT_BRACKET;
    keycode_table["RightBracket"]  = GLFW_KEY_RIGHT_BRACKET;
    keycode_table["Backslash"]     = GLFW_KEY_BACKSLASH;
    keycode_table["GraveAccent"]   = GLFW_KEY_GRAVE_ACCENT;
    
    // Numbers
    for (int i = 0; i <= 9; ++i) {
        keycode_table[std::to_string(i)] = GLFW_KEY_0 + i;
    }

    // Letters A-Z
    for (char c = 'A'; c <= 'Z'; ++c) {
        keycode_table[std::string(1, c)] = GLFW_KEY_A + (c - 'A');
    }

    // Function keys
    keycode_table["Escape"]     = GLFW_KEY_ESCAPE;
    keycode_table["Enter"]      = GLFW_KEY_ENTER;
    keycode_table["Tab"]        = GLFW_KEY_TAB;
    keycode_table["Backspace"]  = GLFW_KEY_BACKSPACE;
    keycode_table["Insert"]     = GLFW_KEY_INSERT;
    keycode_table["Delete"]     = GLFW_KEY_DELETE;
    keycode_table["Right"]      = GLFW_KEY_RIGHT;
    keycode_table["Left"]       = GLFW_KEY_LEFT;
    keycode_table["Down"]       = GLFW_KEY_DOWN;
    keycode_table["Up"]         = GLFW_KEY_UP;
    keycode_table["PageUp"]     = GLFW_KEY_PAGE_UP;
    keycode_table["PageDown"]   = GLFW_KEY_PAGE_DOWN;
    keycode_table["Home"]       = GLFW_KEY_HOME;
    keycode_table["End"]        = GLFW_KEY_END;
    keycode_table["CapsLock"]   = GLFW_KEY_CAPS_LOCK;
    keycode_table["ScrollLock"] = GLFW_KEY_SCROLL_LOCK;
    keycode_table["NumLock"]    = GLFW_KEY_NUM_LOCK;
    keycode_table["PrintScreen"]= GLFW_KEY_PRINT_SCREEN;
    keycode_table["Pause"]      = GLFW_KEY_PAUSE;
    keycode_table["Menu"]       = GLFW_KEY_MENU;

    // F1-F25
    for (int i = 1; i <= 25; ++i) {
        keycode_table["F" + std::to_string(i)] = GLFW_KEY_F1 + (i - 1);
    }

    // Numpad
    for (int i = 0; i <= 9; ++i) {
        keycode_table["KP_" + std::to_string(i)] = GLFW_KEY_KP_0 + i;
    }
    keycode_table["KP_Decimal"] = GLFW_KEY_KP_DECIMAL;
    keycode_table["KP_Divide"]  = GLFW_KEY_KP_DIVIDE;
    keycode_table["KP_Multiply"]= GLFW_KEY_KP_MULTIPLY;
    keycode_table["KP_Subtract"]= GLFW_KEY_KP_SUBTRACT;
    keycode_table["KP_Add"]     = GLFW_KEY_KP_ADD;
    keycode_table["KP_Enter"]   = GLFW_KEY_KP_ENTER;
    keycode_table["KP_Equal"]   = GLFW_KEY_KP_EQUAL;

    // Modifiers
    keycode_table["LeftShift"]    = GLFW_KEY_LEFT_SHIFT;
    keycode_table["RightShift"]   = GLFW_KEY_RIGHT_SHIFT;
    keycode_table["LeftControl"]  = GLFW_KEY_LEFT_CONTROL;
    keycode_table["RightControl"] = GLFW_KEY_RIGHT_CONTROL;
    keycode_table["LeftAlt"]      = GLFW_KEY_LEFT_ALT;
    keycode_table["RightAlt"]     = GLFW_KEY_RIGHT_ALT;
    keycode_table["LeftSuper"]    = GLFW_KEY_LEFT_SUPER;
    keycode_table["RightSuper"]   = GLFW_KEY_RIGHT_SUPER;

    // Make it globally accessible
    lua["Key"] = keycode_table;
#pragma endregion

#pragma region Input
    // Input
    lua.new_usertype<Input>("Input",
        sol::no_constructor, // Prevent creating new instances
        "GetInstance", &Input::GetInstance,

        // Properties
        "scrolledUp", []() { return Input::scrolledUp; },
        "scrolledDown", []() { return Input::scrolledDown; },

        "lockCursor", sol::var(Input::lockCursor),
        "hideCursor", sol::var(Input::hideCursor),
        
        // Methods
        "GetKeyDown", &Input::GetKeyDown,
        "GetKeyUp", &Input::GetKeyUp,
        "GetKey", &Input::GetKey,

        "GetMouseButtonDown", &Input::GetMouseButtonDown,
        "GetMouseButtonUp", &Input::GetMouseButtonUp,
        "GetMouseButton", &Input::GetMouseButton,

        "GetMousePosition", sol::overload(
            sol::resolve<glm::vec2()>(&Input::GetMousePosition),
            [](double* x, double* y) { Input::GetMousePosition(x, y); }
        ),

        "CreateAxis", &Input::CreateAxis,
        "GetAxis", &Input::GetAxis
    );
#pragma endregion
}

void LuaManager::RegisterBindings() {

#pragma region Scene Manager
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
#pragma endregion

#pragma region Transform
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
#pragma endregion 

#pragma region GLM
    // Operating on GLM vec3
    lua.new_usertype<glm::vec3>("vec3",
        sol::call_constructor, sol::factories(
            []() { return glm::vec3(0.0f); },
            [](float x, float y, float z) { return glm::vec3(x, y, z); },
            [](float s) { return glm::vec3(s); }
        ),

        // Component access
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
        ),
        sol::meta_function::unary_minus, [](const glm::vec3& v) {
            return -v;
        },
        sol::meta_function::equal_to, [](const glm::vec3& a, const glm::vec3& b) {
            return a == b;
        },

        // Vector operations
        "length", [](const glm::vec3& v) {
            return glm::length(v);
        },
        "lengthSquared", [](const glm::vec3& v) {
            return glm::dot(v, v);
        },
        "normalized", [](const glm::vec3& v) {
            return glm::normalize(v);
        },
        "dot", [](const glm::vec3& a, const glm::vec3& b) {
            return glm::dot(a, b);
        },
        "cross", [](const glm::vec3& a, const glm::vec3& b) {
            return glm::cross(a, b);
        },
        "distance", [](const glm::vec3& a, const glm::vec3& b) {
            return glm::distance(a, b);
        },
        "distanceSquared", [](const glm::vec3& a, const glm::vec3& b) {
            glm::vec3 diff = a - b;
            return glm::dot(diff, diff);
        },
        "lerp", [](const glm::vec3& a, const glm::vec3& b, float t) {
            return glm::mix(a, b, t);
        },
        "reflect", [](const glm::vec3& v, const glm::vec3& normal) {
            return glm::reflect(v, normal);
        },
        "refract", [](const glm::vec3& v, const glm::vec3& normal, float eta) {
            return glm::refract(v, normal, eta);
        },

        // Clamping and min/max
        "clamp", [](const glm::vec3& v, float min, float max) {
            return glm::clamp(v, min, max);
        },
        "min", [](const glm::vec3& a, const glm::vec3& b) {
            return glm::min(a, b);
        },
        "max", [](const glm::vec3& a, const glm::vec3& b) {
            return glm::max(a, b);
        },

        // Static constants
        "zero", sol::var(glm::vec3(0.0f)),
        "one", sol::var(glm::vec3(1.0f)),
        "up", sol::var(glm::vec3(0.0f, 1.0f, 0.0f)),
        "down", sol::var(glm::vec3(0.0f, -1.0f, 0.0f)),
        "left", sol::var(glm::vec3(-1.0f, 0.0f, 0.0f)),
        "right", sol::var(glm::vec3(1.0f, 0.0f, 0.0f)),
        "forward", sol::var(glm::vec3(0.0f, 0.0f, 1.0f)),
        "back", sol::var(glm::vec3(0.0f, 0.0f, -1.0f)),

        // String
        sol::meta_function::to_string, [](const glm::vec3& v) {
            return std::format("vec3({:.3f}, {:.3f}, {:.3f})", v.x, v.y, v.z);
        }
    );

    lua.new_usertype<glm::vec2>("vec2",
        sol::call_constructor, sol::factories(
            []() { return glm::vec2(0.0f); },
            [](float x, float y) { return glm::vec2(x, y); },
            [](float s) { return glm::vec2(s); }
        ),

        // Component access
        "x", &glm::vec2::x,
        "y", &glm::vec2::y,

        // Operators
        sol::meta_function::addition, [](const glm::vec2& a, const glm::vec2& b) {
            return a + b;
        },
        sol::meta_function::subtraction, [](const glm::vec2& a, const glm::vec2& b) {
            return a - b;
        },
        sol::meta_function::multiplication, sol::overload(
            [](const glm::vec2& a, const glm::vec2& b) { return a * b; },
            [](const glm::vec2& a, float s) { return a * s; },
            [](float s, const glm::vec2& a) { return s * a; }
        ),
        sol::meta_function::division, sol::overload(
            [](const glm::vec2& a, float s) { return a / s; },
            [](float s, const glm::vec2& a) { return glm::vec2(s / a.x, s / a.y); }
        ),
        sol::meta_function::unary_minus, [](const glm::vec2& a) {
            return -a;
        },
        sol::meta_function::equal_to, [](const glm::vec2& a, const glm::vec2& b) {
            return a == b;
        },

        // Vector operations
        "length", [](const glm::vec2& v) {
            return glm::length(v);
        },
        "lengthSquared", [](const glm::vec2& v) {
            return glm::dot(v, v);
        },
        "normalized", [](const glm::vec2& v) {
            return glm::normalize(v);
        },
        "dot", [](const glm::vec2& a, const glm::vec2& b) {
            return glm::dot(a, b);
        },
        "distance", [](const glm::vec2& a, const glm::vec2& b) {
            return glm::distance(a, b);
        },
        "distanceSquared", [](const glm::vec2& a, const glm::vec2& b) {
            glm::vec2 diff = a - b;
            return glm::dot(diff, diff);
        },
        "lerp", [](const glm::vec2& a, const glm::vec2& b, float t) {
            return glm::mix(a, b, t);
        },
        "reflect", [](const glm::vec2& v, const glm::vec2& normal) {
            return glm::reflect(v, normal);
        },
        "refract", [](const glm::vec2& v, const glm::vec2& normal, float eta) {
            return glm::refract(v, normal, eta);
        },

        // Rotation (2D specific)
        "rotate", [](const glm::vec2& v, float angleDegrees) {
            float rad = glm::radians(angleDegrees);
            float c = std::cos(rad);
            float s = std::sin(rad);
            return glm::vec2(v.x * c - v.y * s, v.x * s + v.y * c);
        },
        "angle", [](const glm::vec2& v) {
            return glm::degrees(std::atan2(v.y, v.x));
        },
        "perpendicular", [](const glm::vec2& v) {
            return glm::vec2(-v.y, v.x);
        },

        // Clamping and min/max
        "clamp", [](const glm::vec2& v, float min, float max) {
            return glm::clamp(v, min, max);
        },
        "min", [](const glm::vec2& a, const glm::vec2& b) {
            return glm::min(a, b);
        },
        "max", [](const glm::vec2& a, const glm::vec2& b) {
            return glm::max(a, b);
        },

        // Static constants
        "zero", sol::var(glm::vec2(0.0f)),
        "one", sol::var(glm::vec2(1.0f)),
        "up", sol::var(glm::vec2(0.0f, 1.0f)),
        "down", sol::var(glm::vec2(0.0f, -1.0f)),
        "left", sol::var(glm::vec2(-1.0f, 0.0f)),
        "right", sol::var(glm::vec2(1.0f, 0.0f)),

        // String
        sol::meta_function::to_string, [](const glm::vec2& v) {
            return std::format("vec2({:.3f}, {:.3f})", v.x, v.y);
        }
    );

    // GLM quats
    lua.new_usertype<glm::quat>("quat",
            sol::call_constructor, sol::factories(
            []() { return glm::quat(1, 0, 0, 0); },                     // identity
            [](float w, float x, float y, float z) { return glm::quat(w, x, y, z); },
            [](float angle, const glm::vec3& axis) {
                return glm::angleAxis(glm::radians(angle), axis);
            }
        ),

        // Components (w, x, y, z)
        "w", &glm::quat::w,
        "x", &glm::quat::x,
        "y", &glm::quat::y,
        "z", &glm::quat::z,

        // Static constructor from Euler angles (degrees)
        "Euler", sol::overload(
            [](float pitch, float yaw, float roll) {
                return glm::quat(glm::radians(glm::vec3(pitch, yaw, roll)));
            },
            [](const glm::vec3& eulerAngles) {
                return glm::quat(glm::radians(eulerAngles));
            }
        ),

        "LookRotation", sol::overload(
            [](const glm::vec3& forward) {
                glm::vec3 f = glm::normalize(forward);
                // Use lookAt like your working code
                glm::vec3 fakePos = glm::vec3(0, 0, 0);
                glm::mat4 viewMatrix = glm::lookAt(fakePos, fakePos - f, glm::vec3(0, 1, 0));
                return glm::quat_cast(viewMatrix);
            },
            [](const glm::vec3& forward, const glm::vec3& up) {
                glm::vec3 f = glm::normalize(forward);
                glm::vec3 u = glm::normalize(up);
                glm::vec3 fakePos = glm::vec3(0, 0, 0);
                glm::mat4 viewMatrix = glm::lookAt(fakePos, fakePos - f, u);
                return glm::quat_cast(viewMatrix);
            }
        ),
        
        // Convert to Euler angles (degrees)
        "to_euler", [](const glm::quat& q) {
            return glm::degrees(glm::eulerAngles(q));
        },

        // Normalize
        "normalized", [](const glm::quat& q) {
            return glm::normalize(q);
        },

        // Get angle (in degrees) + axis
        "get_angle", [](const glm::quat& q) {
            return glm::degrees(glm::angle(q));
        },
        "get_axis", [](const glm::quat& q) {
            return glm::axis(q);
        },

        // Spherical linear interpolation
        "slerp", [](const glm::quat& a, const glm::quat& b, float t) {
            return glm::slerp(a, b, t);
        },

        // Linear interpolation (faster but less accurate)
        "lerp", [](const glm::quat& a, const glm::quat& b, float t) {
            return glm::lerp(a, b, t);
        },

        // Dot product
        "dot", [](const glm::quat& a, const glm::quat& b) {
            return glm::dot(a, b);
        },

        // Inverse
        "inverse", [](const glm::quat& q) {
            return glm::inverse(q);
        },

        // Conjugate
        "conjugate", [](const glm::quat& q) {
            return glm::conjugate(q);
        },

        // Rotate a vector
        "rotate_vector", [](const glm::quat& q, const glm::vec3& v) {
            return q * v;
        },

        // Multiply two quaternions
        sol::meta_function::multiplication, sol::overload(
            [](const glm::quat& a, const glm::quat& b) {
                return a * b;
            },
            // quat * vec3 → vec3
            [](const glm::quat& q, const glm::vec3& v) {
                return q * v;
            }
        ),

        // To string
        sol::meta_function::to_string, [](const glm::quat& q) {
            return std::format("Quaternion({}, {}, {}, {})", q.w, q.x, q.y, q.z);
        }
    );
#pragma endregion

#pragma region Components

    // Default Component
    lua.new_usertype<Component>("Component",
        sol::no_constructor,
        "owner", sol::readonly_property(&Component::owner),
        "transform", sol::readonly_property(&Component::transform)
    );

    // Camera
    lua.new_usertype<Camera>("Camera",
        sol::no_constructor,
        sol::base_classes, sol::bases<Component>(),
        "fieldOfView", &Camera::fieldOfView,
        "nearClippingPlane", &Camera::nearClippingPlane,
        "farClippingPlane", &Camera::farClippingPlane
    );
    RegisterComponent<Camera>("Camera", lua);

    // Directional Light
    lua.new_usertype<DirectionalLight>("DirectionalLight",
        sol::no_constructor,
        sol::base_classes, sol::bases<Component>(),
        "color", &DirectionalLight::color,
        "strength", &DirectionalLight::strength,
        "castShadows", &DirectionalLight::castShadows,
        "enabled", &DirectionalLight::enabled
    );
    RegisterComponent<DirectionalLight>("DirectionalLight", lua);

    // Point Light
    lua.new_usertype<PointLight>("PointLight",
        sol::no_constructor,
        sol::base_classes, sol::bases<Component>(),
        "color", &PointLight::color,
        "strength", &PointLight::strength,
        "radius", &PointLight::radius,
        "enabled", &PointLight::enabled
    );
    RegisterComponent<PointLight>("PointLight", lua);

    // Spotlight
    lua.new_usertype<SpotLight>("SpotLight",
        sol::no_constructor,
        sol::base_classes, sol::bases<Component>(),
        "color", &SpotLight::color,
        "strength", &SpotLight::strength,
        "distance", &SpotLight::distance,
        "angle", &SpotLight::angle,
        "castShadows", &SpotLight::castShadows,
        "enabled", &SpotLight::enabled
    );
    RegisterComponent<SpotLight>("SpotLight", lua);

    // RigidBody
    lua.new_usertype<RigidBody>("RigidBody",
        sol::no_constructor,
        sol::base_classes, sol::bases<Component>(),
        // Properties
        "mass", &RigidBody::mass,
        "isTrigger", &RigidBody::isTrigger,

        // Callback bindings
        "OnContactStarted", &RigidBody::OnContactStarted,
        "OnContacting", &RigidBody::OnContacting,
        "OnContactEnded", &RigidBody::OnContactEnded,

        "OnTriggerEntered", &RigidBody::OnTriggerEntered,
        "OnTriggerStay", &RigidBody::OnTriggerStay,
        "OnTriggerExited", &RigidBody::OnTriggerExited,
        
        // Methods
        "AddForce", &RigidBody::AddForce,
        "AddTorque", &RigidBody::AddTorque,
        "AddImpulse", &RigidBody::AddImpulse,
        "AddAngularImpulse", &RigidBody::AddAngularImpulse,

        "SetLinearVelocity", &RigidBody::SetLinearVelocity,
        "GetLinearVelocity", &RigidBody::GetLinearVelocity,

        "SetAngularVelocity", &RigidBody::SetAngularVelocity,
        "GetAngularVelocity", &RigidBody::GetAngularVelocity,

        "IsActive", &RigidBody::IsActive,

        "SetKinematic", &RigidBody::SetKinematic,
        "IsKinematic", &RigidBody::IsKinematic
    );
    RegisterComponent<RigidBody>("RigidBody", lua);

#pragma endregion

#pragma region Actor
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

        // HasComponent
        "HasComponent", [](Actor& self, const std::string& typeName) -> bool {
            auto it = componentRegistry.find(typeName);
            if (it == componentRegistry.end()) {
                throw std::runtime_error("Unknown Component type: " + typeName);
            }
            return it->second.hasFn(self);
        },

        // GetComponent
        "GetComponent", [](sol::this_state ts, Actor& self, const std::string& typeName) -> sol::object {
            auto it = componentRegistry.find(typeName);
            if (it == componentRegistry.end()) {
                return sol::nil;
            }

            return it->second.getFn(self);
            // if (!comp) {
            //     return sol::nil;
            // }
            //
            // sol::state_view lua(ts);
            // return sol::make_object(lua, comp);
        },

        // AddComponent
        "AddComponent", [](sol::this_state ts, Actor& self, const std::string& typeName) -> sol::object {
            auto it = componentRegistry.find(typeName);
            if (it == componentRegistry.end()) {
                throw std::runtime_error("Unknown component type: " + typeName);
            }

            return it->second.addFn(self);
            // sol::state_view lua(ts);
            // return sol::make_object(lua, comp);
        }
    );
#pragma endregion


    RegisterInputBindings();
}
