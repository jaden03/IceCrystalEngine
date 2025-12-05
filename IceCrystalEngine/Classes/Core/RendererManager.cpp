#include "Ice/Core/RendererManager.h"

#include <iostream>

#include "glm/gtx/string_cast.hpp"
#include "Ice/Core/SceneManager.h"

RendererManager::RendererManager()
{
    Initialize();
}


void RendererManager::Initialize()
{
    glGenBuffers(1, &GlobalDataUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, GlobalDataUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(RendererGlobalData), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, GlobalDataUBO);
}

void RendererManager::UpdateUBOs()
{
    SceneManager &sceneManager = SceneManager::GetInstance();
    
    GlobalData.view = sceneManager.mainCamera->view;
    GlobalData.projection = sceneManager.mainCamera->projection;
    GlobalData.viewPos = sceneManager.mainCamera->transform->position;
    GlobalData.time = (float)glfwGetTime();
    GlobalData.nearPlane = sceneManager.mainCamera->nearClippingPlane;
    GlobalData.farPlane = sceneManager.mainCamera->farClippingPlane;
    GlobalData._padding0 = 420;
    GlobalData._padding1 = 420;
    
    glBindBuffer(GL_UNIFORM_BUFFER, GlobalDataUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(RendererGlobalData), &GlobalData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
