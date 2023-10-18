#include <Ice/Core/LightingManager.h>

#include <Ice/Components/Light.h>
#include <iostream>

std::vector<DirectionalLight*> directionalLights;
std::vector<PointLight*> pointLights;

LightingManager::LightingManager()
{
}


void LightingManager::InitializeLighting()
{
	directionalLights = std::vector<DirectionalLight*>();
	glGenBuffers(1, &directionalLightSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, directionalLightSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat3) * maxDirectionalLights, NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, directionalLightSSBO);


	pointLights = std::vector<PointLight*>();
	glGenBuffers(1, &pointLightSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat3) * maxPointLights, NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pointLightSSBO);
}


void LightingManager::AddDirectionalLight(DirectionalLight* light)
{
	if (directionalLights.size() < maxDirectionalLights)
	{
		directionalLights.push_back(light);
	}
	else
	{
		// TODO : ADD BETTER LOGGING
		std::cout << "Max directional lights reached!" << std::endl;
	}
}

void LightingManager::AddPointLight(PointLight* light)
{
	if (pointLights.size() < maxPointLights)
	{
		pointLights.push_back(light);
	}
	else
	{
		// TODO : ADD BETTER LOGGING
		std::cout << "Max point lights reached!" << std::endl;
	}
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




void LightingManager::UpdateLights()
{
	// Directional Lights
	glm::mat3* directionalLightData = new glm::mat3[directionalLights.size()];
	for (int i = 0; i < directionalLights.size(); i++)
	{
		directionalLightData[i] = directionalLights[i]->GetLightData();
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, directionalLightSSBO);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::mat3) * directionalLights.size(), directionalLightData);
	delete[] directionalLightData;
	
	// Point Lights
	glm::mat3* pointLightData = new glm::mat3[pointLights.size()];
	for (int i = 0; i < pointLights.size(); i++)
	{
		pointLightData[i] = pointLights[i]->GetLightData();
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointLightSSBO);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::mat3) * pointLights.size(), pointLightData);
	delete[] pointLightData;
}