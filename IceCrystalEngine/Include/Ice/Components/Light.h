#pragma once

#ifndef LIGHT_H

#define LIGHT_H

#include <glm/glm.hpp>
#include <Ice/Core/Component.h>

class DirectionalLight : public Component
{
	SceneManager& sceneManager = SceneManager::GetInstance();

	void Initialize();
	
public:
	
	DirectionalLight();
	DirectionalLight(glm::vec3 color, float strength);

	~DirectionalLight();


	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	float strength = 1;
	
	// this is the resolution of the shadow map, the higher the resolution, the better the shadows will look, but the more performance it will cost
	int shadowMapResolution = 2048;
	bool castShadows = true;
	unsigned int depthMapArray;

	int cascadeCount = 4;
	std::vector<float> cascadeSplits = { 10.0f, 20.0f, 40.0f, 70.0f };
	std::vector<float> worldSpaceCascadeSplits;
	std::vector<glm::mat4> cascadeMatrices;
	unsigned int cascadeMatricesUBO;

	glm::mat4 GetLightSpaceMatrix(float nearPlane, float farPlane);
	void BuildCascades();
};


class PointLight : public Component
{

public:

	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	float strength = 1;
	float radius = 5;

	PointLight();
	PointLight(glm::vec3 color, float strength, float radius);

	~PointLight();
	
};


class SpotLight : public Component
{

	void Initialize();
	
public: 

	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	float strength = 1;
	float distance = 10;
	float angle = 25;
	
	SpotLight();
	SpotLight(glm::vec3 color, float strength, float distance, float angle);
	
	~SpotLight();



	int shadowMapResolution = 1024;
	bool castShadows = true;
	unsigned int depthMap;
	
	glm::mat4 GetLightSpaceMatrix();
};

#endif