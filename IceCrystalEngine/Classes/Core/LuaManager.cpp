#include <Ice/Core/LuaManager.h>
#include <iostream>
#include <sstream>

#include "Ice/Utils/DebugUtil.h"

LuaManager::LuaManager()
{
    InitializeLuaState();
}

LuaManager::~LuaManager()
{
    lua_close(L);
}


void LuaManager::InitializeLuaState()
{
    L = luaL_newstate();
    if (L == nullptr) {
        std::cout << "Failed to create Lua state" << std::endl;
    }
    luaL_openlibs(L);

    // Replace the global Lua print function
    lua_register(L, "print", LuaPrint);
}

int LuaManager::LuaPrint(lua_State* L) {
    int nargs = lua_gettop(L); // Number of arguments
    std::stringstream output;

    for (int i = 1; i <= nargs; i++) {
        if (lua_isstring(L, i)) {
            output << lua_tostring(L, i);
        } else {
            output << lua_typename(L, lua_type(L, i));
        }

        if (i < nargs) {
            output << "\t";
        }
    }
    output << "\n";

    // Redirect output to your custom console
    DebugUtil::GetInstance().ss << output.str();

    return 0; // Return no values to Lua
}