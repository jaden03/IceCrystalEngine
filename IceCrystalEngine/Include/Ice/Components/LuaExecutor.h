#pragma once

#ifndef LUA_EXECUTOR_H

#define LUA_EXECUTOR_H

#include <iostream>

#include "Ice/Core/Component.h"

class LuaExecutor : public Component
{
public:
    LuaExecutor();
    LuaExecutor(std::string filePath);
    virtual ~LuaExecutor();

    bool runOnReady = true;
    std::string filePath;

    void Execute();

    void Ready() override;
};

#endif