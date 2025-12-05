#pragma once

#include "Ice/Components/Camera.h"
#include <glm/glm.hpp>

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

class RendererManager
{
public:
    static RendererManager& GetInstance()
    {
        static RendererManager instance; // Static local variable ensures a single instance
        return instance;
    }

    unsigned int GlobalDataUBO; // 0
    unsigned int DirectionalLightUBO; // 1
    // cascades for the directional light is 2
    unsigned int PointLightUBO; // 3
    unsigned int SpotLightUBO; // 4

    RendererGlobalData GlobalData;

    void Initialize();

    void UpdateUBOs();

private:
    RendererManager();

    RendererManager(RendererManager const&) = delete; // Delete copy constructor
    void operator=(RendererManager const&) = delete; // Delete assignment operator
};