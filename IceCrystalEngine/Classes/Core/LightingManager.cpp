#include <Ice/Core/LightingManager.h>

#include <Ice/Components/Light.h>
#include <iostream>

std::vector<DirectionalLight*> directionalLights;
std::vector<PointLight*> pointLights;

LightingManager::LightingManager() {}

LightingManager::~LightingManager()
{
	delete[] directionalLightData;
	delete[] pointLightData;
}


void LightingManager::InitializeLighting()
{
	shadowShader = new Shader("{ASSET_DIR}Shaders/shadows.vert", "{ASSET_DIR}Shaders/shadows.frag");
	glGenFramebuffers(1, &shadowMapFBO);

	directionalLights = std::vector<DirectionalLight*>();
	pointLights = std::vector<PointLight*>();
}


void LightingManager::AddDirectionalLight(DirectionalLight* light)
{
	directionalLights.push_back(light);
}

void LightingManager::AddPointLight(PointLight* light)
{
	pointLights.push_back(light);
}


void LightingManager::RemoveDirectionalLight(DirectionalLight* light)
{
	for (int i = 0; i < directionalLights.size(); i++)
	{
		if (directionalLights[i] == light)
		{
			directionalLights.erase(directionalLights.begin() + i);
			return;
		}
	}
}

void LightingManager::RemovePointLight(PointLight* light)
{
	for (int i = 0; i < pointLights.size(); i++)
	{
		if (pointLights[i] == light)
		{
			pointLights.erase(pointLights.begin() + i);
			return;
		}
	}
}