#include <Ice/Core/LightingManager.h>

#include <Ice/Components/Light.h>
#include <iostream>

std::vector<DirectionalLight*> directionalLights;
std::vector<PointLight*> pointLights;

LightingManager::LightingManager() {}

LightingManager::~LightingManager()
{
	
}


void LightingManager::InitializeLighting()
{
	shadowShader = new Shader("{ASSET_DIR}Shaders/shadows");
	shadowsCascadedShader = new Shader("{ASSET_DIR}Shaders/shadowsCascaded");
	glGenFramebuffers(1, &shadowMapFBO);
	
	pointLights = std::vector<PointLight*>();
	spotLights = std::vector<SpotLight*>();
}


void LightingManager::AddDirectionalLight(DirectionalLight* light)
{
	if (directionalLight != nullptr)
	{
		std::cout << "Directional light already exists, deleting the new one bud" << std::endl;
		if (light->transform != nullptr)
			light->transform->actor->DeleteComponent(light);
		else
			delete &light;
	}
	directionalLight = light;
}

void LightingManager::AddPointLight(PointLight* light)
{
	pointLights.push_back(light);
}

void LightingManager::AddSpotLight(SpotLight* light)
{
	spotLights.push_back(light);
}


void LightingManager::RemoveDirectionalLight(DirectionalLight* light)
{
	if (directionalLight == light)
		directionalLight = nullptr;
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

void LightingManager::RemoveSpotLight(SpotLight* light)
{
	for (int i = 0; i < spotLights.size(); i++)
	{
		if (spotLights[i] == light)
		{
			spotLights.erase(spotLights.begin() + i);
			return;
		}
	}
}