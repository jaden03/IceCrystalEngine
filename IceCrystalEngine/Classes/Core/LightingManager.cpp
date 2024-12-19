#include <Ice/Core/LightingManager.h>
#include <iostream>
#include <cstring>
#include <Ice/Core/Actor.h>
#include <Ice/Core/Transform.h>

LightingManager::LightingManager() {}

LightingManager::~LightingManager()
{
    glDeleteBuffers(1, &directionalLightSSBO);
    glDeleteBuffers(1, &pointLightSSBO);
    glDeleteBuffers(1, &spotLightSSBO);
}

void LightingManager::InitializeLighting()
{
    shadowShader = new Shader("{ASSET_DIR}Shaders/shadows.vert", "{ASSET_DIR}Shaders/shadows.frag");
    glGenFramebuffers(1, &shadowMapFBO);

    directionalLightData.clear();
    pointLightData.clear();
    spotLightData.clear();
    directionalLights.clear();
    pointLights.clear();
    spotLights.clear();

    CreateSSBOs();
}

void LightingManager::CreateSSBOs()
{
    glGenBuffers(1, &directionalLightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, directionalLightSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DirectionalLightData) * maxDirectionalLights, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, directionalLightSSBO);

    glGenBuffers(1, &pointLightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PointLightData) * maxPointLights, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pointLightSSBO);

    glGenBuffers(1, &spotLightSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, spotLightSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SpotLightData) * maxSpotLights, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, spotLightSSBO);
}

void LightingManager::AddDirectionalLight(DirectionalLight* light)
{
    DirectionalLightData lightData = {
        glm::vec3(0.0),
        light->color,
        light->strength,
        light->depthMap,
        light->GetLightSpaceMatrix(mainCamera),
        light->castShadows
    };

    directionalLightData.push_back(lightData);
    directionalLights.push_back(light);
    UpdateDirectionalLightSSBO();
}

void LightingManager::AddPointLight(PointLight* light)
{
    PointLightData lightData = {
        glm::vec3(0.0),
        light->color,
        light->strength,
        light->radius
    };

    pointLightData.push_back(lightData);
    pointLights.push_back(light);
    UpdatePointLightSSBO();
}

void LightingManager::AddSpotLight(SpotLight* light)
{
    SpotLightData lightData = {
        glm::vec3(0.0),
        glm::vec3(0.0),
        light->color,
        light->strength,
        light->angle,
        light->angle + 5,
        light->distance,
        light->depthMap,
        light->GetLightSpaceMatrix(),
        light->castShadows
    };

    spotLightData.push_back(lightData);
    spotLights.push_back(light);
    UpdateSpotLightSSBO();
}

void LightingManager::RemoveDirectionalLight(DirectionalLight* light)
{
    auto it = std::find(directionalLights.begin(), directionalLights.end(), light);
    if (it != directionalLights.end())
    {
        int index = std::distance(directionalLights.begin(), it);
        directionalLights.erase(it);
        directionalLightData.erase(directionalLightData.begin() + index);
    }
    UpdateDirectionalLightSSBO();
}

void LightingManager::RemovePointLight(PointLight* light)
{
    auto it = std::find(pointLights.begin(), pointLights.end(), light);
    if (it != pointLights.end())
    {
        int index = std::distance(pointLights.begin(), it);
        pointLights.erase(it);
        pointLightData.erase(pointLightData.begin() + index);
    }
    UpdatePointLightSSBO();
}

void LightingManager::RemoveSpotLight(SpotLight* light)
{
    auto it = std::find(spotLights.begin(), spotLights.end(), light);
    if (it != spotLights.end())
    {
        int index = std::distance(spotLights.begin(), it);
        spotLights.erase(it);
        spotLightData.erase(spotLightData.begin() + index);
    }
    UpdateSpotLightSSBO();
}

void LightingManager::UpdateDirectionalLightSSBO()
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, directionalLightSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(DirectionalLightData) * directionalLightData.size(), &directionalLightData[0]);
}

void LightingManager::UpdatePointLightSSBO()
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(PointLightData) * pointLightData.size(), &pointLightData[0]);
}

void LightingManager::UpdateSpotLightSSBO()
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, spotLightSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(SpotLightData) * spotLightData.size(), &spotLightData[0]);
}

void LightingManager::UpdateDirectionalLightData()
{
    // Sync the data in place (no need to recreate structs every frame)
    for (size_t i = 0; i < directionalLights.size(); ++i)
    {
        DirectionalLight* light = directionalLights[i];
        DirectionalLightData& lightData = directionalLightData[i];
        
        // Only update if the light data has changed (optional)
        lightData.direction = light->transform->forward;
        lightData.strength = light->strength;
        lightData.color = light->color;
        lightData.strength = light->strength;
        lightData.shadowMap = light->depthMap;
        lightData.lightSpaceMatrix = light->GetLightSpaceMatrix(mainCamera);
        lightData.castShadows = light->castShadows;
    }
}

void LightingManager::UpdatePointLightData()
{
    for (size_t i = 0; i < pointLights.size(); ++i)
    {
        PointLight* light = pointLights[i];
        PointLightData& lightData = pointLightData[i];
        
        // Update point light data
        lightData.position = light->transform->position;
        lightData.strength = light->strength;
        lightData.color = light->color;
        lightData.radius = light->radius;
    }
}

void LightingManager::UpdateSpotLightData()
{
    for (size_t i = 0; i < spotLights.size(); ++i)
    {
        SpotLight* light = spotLights[i];
        SpotLightData& lightData = spotLightData[i];
        
        // Update spot light data
        lightData.position = light->transform->position;
        lightData.direction = light->transform->forward;
        lightData.color = light->color;
        lightData.strength = light->strength;
        lightData.angle = light->angle;
        lightData.angle = light->angle + 5;
        lightData.distance = light->distance;
        lightData.shadowMap = light->depthMap;
        lightData.lightSpaceMatrix = light->GetLightSpaceMatrix();
        lightData.castShadows = light->castShadows;
    }
}

void LightingManager::UpdateSSBOs()
{
    // Update the data structs once per frame
    UpdateDirectionalLightData();
    UpdatePointLightData();
    UpdateSpotLightData();

    // Then update the SSBOs with the new data
    UpdateDirectionalLightSSBO();
    UpdatePointLightSSBO();
    UpdateSpotLightSSBO();
}