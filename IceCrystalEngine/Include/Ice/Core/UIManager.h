#pragma once
#ifndef UI_MANAGER_H

#define UI_MANAGER_H

#include <glm/glm.hpp>
#include <Ice/Core/WindowManager.h>
#include <Ice/Rendering/Material.h>

class UIManager
{

public:

    static UIManager& GetInstance()
    {
        static UIManager instance;
        return instance;
    }

    void RenderTest(glm::vec2 position, glm::vec2 size);

private:

    unsigned int uiManagerVAO, uiManagerVBO;
    
    WindowManager& windowManager = WindowManager::GetInstance();

    Material* material = nullptr;
    
    UIManager();

    UIManager(UIManager const&) = delete;
    void operator=(UIManager const&) = delete;
    
};

#endif