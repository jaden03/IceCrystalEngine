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


void LightingManager::SetDirectionalLight(DirectionalLight* light)
{
	if (directionalLight != nullptr)
	{
		std::cout << "Directional light already exists, deleting new one" << std::endl;
		if (light->transform != nullptr && light->transform->actor != nullptr)
		{
			light->transform->actor->DeleteComponent(light);
		}
		return;
	}
	directionalLight = light;

	glGenTextures(1, &light->depthMapArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, light->depthMapArray);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, light->shadowMapResolution, light->shadowMapResolution, light->cascadeCount, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, LightingManager::GetInstance().shadowMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, light->depthMapArray, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	
	glGenBuffers(1, &light->cascadeMatricesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, light->cascadeMatricesUBO);
	glBufferData(GL_UNIFORM_BUFFER,  sizeof(glm::mat4x4) * LightingManager::GetInstance().maxCascades, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, light->cascadeMatricesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	light->cascadeMatrices.resize(light->cascadeCount);
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