#pragma once

#ifndef LIGHTING_MANAGER_H
#define LIGHTING_MANAGER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <Ice/Rendering/Shader.h>
#include <Ice/Components/Light.h>

class LightingManager
{
public:
	static LightingManager& GetInstance()
	{
		static LightingManager instance;
		return instance;
	}

	float ambientLightingStrength = 0.15f;
	glm::vec3 ambientLightingColor = glm::vec3(1.0f, 1.0f, 1.0f);

	Camera* mainCamera;
	
	int maxPointLights = 64;
	int maxSpotLights = 16;
	int maxDirectionalLights = 5;

	std::vector<DirectionalLightData> directionalLightData;
	std::vector<PointLightData> pointLightData;
	std::vector<SpotLightData> spotLightData;

	std::vector<DirectionalLight*> directionalLights;
	std::vector<PointLight*> pointLights;
	std::vector<SpotLight*> spotLights;

	Shader* shadowShader;
	unsigned int shadowMapFBO = 0;

	GLuint directionalLightSSBO = 0;
	GLuint pointLightSSBO = 0;
	GLuint spotLightSSBO = 0;

	void InitializeLighting();
	void AddDirectionalLight(DirectionalLight* light);
	void AddPointLight(PointLight* light);
	void AddSpotLight(SpotLight* light);
	void RemoveDirectionalLight(DirectionalLight* light);
	void RemovePointLight(PointLight* light);
	void RemoveSpotLight(SpotLight* light);
	void UpdateSSBOs();

private:
	
	LightingManager();
	~LightingManager();

	LightingManager(LightingManager const&) = delete;
	void operator=(LightingManager const&) = delete;

	void CreateSSBOs();
	
	void UpdateDirectionalLightSSBO();
	void UpdatePointLightSSBO();
	void UpdateSpotLightSSBO();

	void UpdateDirectionalLightData();
	void UpdatePointLightData();
	void UpdateSpotLightData();
};

#endif
