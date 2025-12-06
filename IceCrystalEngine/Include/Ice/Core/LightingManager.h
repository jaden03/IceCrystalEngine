#pragma once

#ifndef LIGHTING_MANAGER_H

#define LIGHTING_MANAGER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

#include <Ice/Rendering/Shader.h>

class DirectionalLight;
class PointLight;
class SpotLight;

class LightingManager
{

public:

    static LightingManager& GetInstance()
    {
        static LightingManager instance; // Static local variable ensures a single instance
        return instance;
    }

    float ambientLightingStrength = 0.15f;
    glm::vec3 ambientLightingColor = glm::vec3(1.0f, 1.0f, 1.0f);
	
    static constexpr int maxPointLights = 64;
    static constexpr int maxSpotLights = 64;
    static constexpr int maxCascades = 4;

    static constexpr int directionalShadowMapUnit = 5; // this is the texture bound for the directional shadow map

    DirectionalLight* directionalLight = nullptr;
    std::vector<PointLight*> pointLights = std::vector<PointLight*>();
    std::vector<SpotLight*> spotLights = std::vector<SpotLight*>();

    Shader* shadowShader;
    Shader* shadowsCascadedShader;
    unsigned int shadowMapFBO = 0;


    void InitializeLighting();

    void AddDirectionalLight(DirectionalLight* light);
    void AddPointLight(PointLight* light);
    void AddSpotLight(SpotLight* light);

    void RemoveDirectionalLight(DirectionalLight* light);
    void RemovePointLight(PointLight* light);
    void RemoveSpotLight(SpotLight* light);

private:

    LightingManager(); // Private constructor to ensure a single instance
    ~LightingManager();

    LightingManager(LightingManager const&) = delete; // Delete copy constructor
    // this prevents the copy constructor "SceneManager copy(original);" from working

    void operator=(LightingManager const&) = delete; // Delete assignment operator
    // this prevents copying by assignment "SceneManager another = original;" from working

};

#endif