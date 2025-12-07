#include <Ice/Components/LuaExecutor.h>

#include "Ice/Core/LuaManager.h"

LuaExecutor::LuaExecutor(std::string path)
{
    filePath = path;
}

void LuaExecutor::Execute()
{
    if (filePath.size() > 0)
    LuaManager::GetInstance().RunExecutor(this);
}

void LuaExecutor::Ready()
{
    if (filePath.size() > 0 && runOnReady)
        LuaManager::GetInstance().RunExecutor(this);
}

LuaExecutor::~LuaExecutor() {}