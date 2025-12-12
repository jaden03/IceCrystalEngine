#include "Ice/Core/RendererManager.h"

#include <iostream>

#include "glm/gtx/string_cast.hpp"
#include "Ice/Components/Light.h"
#include "Ice/Core/SceneManager.h"
#include "Ice/Core/LightingManager.h"
#include "Ice/Utils/FileUtil.h"

RendererManager::RendererManager()
{
    Initialize();
}


void RendererManager::Initialize()
{
    // Init the GlobalData UBO
    glGenBuffers(1, &GlobalDataUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, GlobalDataUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(RendererGlobalData), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, GlobalDataUBO);

    // Init the LightingData UBO
    glGenBuffers(1, &LightingDataUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, LightingDataUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(RendererLightingData), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, LightingDataUBO);
    
    // Init the DirectionalLight UBO
    glGenBuffers(1, &DirectionalLightUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, DirectionalLightUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(RendererDirectionalLightData), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, DirectionalLightUBO);

    // Cascades use the 4th UBO
    
    // Init the PointLight UBO
    glGenBuffers(1, &PointLightUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, PointLightUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(RendererPointLightData), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 4, PointLightUBO);
    
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Setup the line renderer shader
    lineRendererShader = new Shader(FileUtil::EngineAssetDir + "Shaders/line");
}

void RendererManager::UpdateUBOs()
{
    // Global Data
    SceneManager &sceneManager = SceneManager::GetInstance();

    // Check if mainCamera exists before accessing it
    if (sceneManager.mainCamera == nullptr)
        return;

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

    // Lighting Data
    LightingManager &lightingManager = LightingManager::GetInstance();

    LightingData.directionalLightExists = (int)(lightingManager.directionalLight != nullptr);
    LightingData.pointLightCount = lightingManager.pointLights.size();
    LightingData.spotLightCount = lightingManager.spotLights.size();
    LightingData.ambientLightStrength = lightingManager.ambientLightingStrength;
    LightingData.ambientLightColor = lightingManager.ambientLightingColor;

    glBindBuffer(GL_UNIFORM_BUFFER, LightingDataUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(RendererLightingData), &LightingData);
    
        // Directional Light
    DirectionalLight* directionalLight = lightingManager.directionalLight;

    if (directionalLight != nullptr)
    {
        DirectionalLightData.direction = directionalLight->transform->forward;
        DirectionalLightData.enabled = (int)directionalLight->enabled;
        DirectionalLightData.color = directionalLight->color;
        DirectionalLightData.strength = directionalLight->strength;
        DirectionalLightData.castShadows = (int)directionalLight->castShadows;
        DirectionalLightData.cascadeCount = directionalLight->cascadeCount;
        for (unsigned int i = 0; i < LightingManager::maxCascades; i++)
        {
            if (i < directionalLight->cascadeSplits.size())
                DirectionalLightData.cascadeSplits[i].value = directionalLight->cascadeSplits[i];
            else
                DirectionalLightData.cascadeSplits[i].value = 0.0f;
        }
        // bind the directional shadow map to the reserved unit (5)
        glActiveTexture(GL_TEXTURE0 + lightingManager.directionalShadowMapUnit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, directionalLight->depthMapArray);
    }
    
    glBindBuffer(GL_UNIFORM_BUFFER, DirectionalLightUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(RendererDirectionalLightData), &DirectionalLightData);

        // Point Lights
    for (unsigned int i = 0; i < lightingManager.maxPointLights; i++)
    {
        if (i < lightingManager.pointLights.size())
        {
            PointLight* point = lightingManager.pointLights[i];
            PointLightData.pointLights[i].position = point->transform->position;
            PointLightData.pointLights[i].enabled = (int)point->enabled;
            PointLightData.pointLights[i].color = point->color;
            PointLightData.pointLights[i].strength = point->strength;
            PointLightData.pointLights[i].radius = point->radius;
        }
        else
        {
            PointLightData.pointLights[i].position = glm::vec3(0.0f);
            PointLightData.pointLights[i].color = glm::vec3(0.0f);
            PointLightData.pointLights[i].strength = 0.0f;
            PointLightData.pointLights[i].radius = 0.0f;
        }
    }
    
    glBindBuffer(GL_UNIFORM_BUFFER, PointLightUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(RendererPointLightData), &PointLightData);

    // int numberOfPointLights = lightingManager.pointLights.size();
    // int maxPointLights = lightingManager.maxPointLights;
    // if (numberOfPointLights > maxPointLights)
    //     numberOfPointLights = maxPointLights;
    //
    // for (int i = 0; i < numberOfPointLights; i++)
    // {
    //     std::string prefix = "pointLights[" + std::to_string(i) + "].";
    //     material->shader->setVec3(prefix + "position", lightingManager.pointLights[i]->transform->position);
    //     material->shader->setVec3(prefix + "color", lightingManager.pointLights[i]->color);
    //     material->shader->setFloat(prefix + "strength", lightingManager.pointLights[i]->strength);
    //     material->shader->setFloat(prefix + "radius", lightingManager.pointLights[i]->radius);
    // }
    
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
