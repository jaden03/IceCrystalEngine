#pragma once

#include "Ice/Components/Camera.h"
#include <glm/glm.hpp>

#include "Ice/Rendering/Shader.h"

// Global Data
struct RendererGlobalData
{
    glm::mat4 view; // 0 - 63
    glm::mat4 projection; // 64 - 127
    glm::vec3 viewPos; // 128 - 143
    float time;
    float nearPlane;
    float farPlane;
    // pad to be a multiple of 16 bytes
    float _padding0;
    float _padding1;
};

// Lighting Data
struct RendererLightingData
{
    int directionalLightExists;
    int pointLightCount;
    int spotLightCount;

    float ambientLightStrength;
    glm::vec3 ambientLightColor;

    int _padding0;
};

// Directional Light Data
struct RendererCascadeSplitData
{
    float value;
    float _padding0, _padding1, _padding2;
};
struct RendererDirectionalLightData
{
    glm::vec3 direction;
    int enabled;
    
    glm::vec3 color;
    int _padding1;

    // 48-63
    float strength;
    int castShadows;
    int cascadeCount;
    int _padding2;

    // 64-(64 + 4*16)
    RendererCascadeSplitData cascadeSplits[LightingManager::maxCascades];
};

// Point Light Data
struct RendererIndividualPointLightData
{
    glm::vec3 position;
    int enabled;

    glm::vec3 color;
    float strength;

    glm::vec3 _padding1;
    float radius;
};
struct RendererPointLightData
{
    RendererIndividualPointLightData pointLights[LightingManager::maxPointLights];
};

class RendererManager
{
public:
    static RendererManager& GetInstance()
    {
        static RendererManager instance; // Static local variable ensures a single instance
        return instance;
    }

    Shader* lineRendererShader;

    unsigned int GlobalDataUBO; // 0
    unsigned int LightingDataUBO; // 1
    unsigned int DirectionalLightUBO; // 2
    // cascades for the directional light is 3
    unsigned int PointLightUBO; // 4
    unsigned int SpotLightUBO; // 5

    RendererGlobalData GlobalData;
    RendererLightingData LightingData;
    RendererDirectionalLightData DirectionalLightData;
    RendererPointLightData PointLightData;

    void Initialize();

    void UpdateUBOs();

private:
    RendererManager();

    RendererManager(RendererManager const&) = delete; // Delete copy constructor
    void operator=(RendererManager const&) = delete; // Delete assignment operator
};