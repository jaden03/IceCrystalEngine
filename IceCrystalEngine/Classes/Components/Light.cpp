#include <Ice/Components/Light.h>

#include <Ice/Core/LightingManager.h>
#include <Ice/Components/Camera.h>

#include <array>
#include <iostream>
#include <ostream>

#include "glm/gtx/string_cast.hpp"

DirectionalLight::DirectionalLight() : Component() 
{
	Initialize();
}

DirectionalLight::DirectionalLight(glm::vec3 color, float strength) : Component()
{
	this->color = color;
	this->strength = strength;

	Initialize();
}

DirectionalLight::~DirectionalLight()
{
	LightingManager::GetInstance().RemoveDirectionalLight(this);
}

void DirectionalLight::Initialize()
{
	glGenTextures(1, &depthMapArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, depthMapArray);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, shadowMapResolution, shadowMapResolution, cascadeCount, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
	
	glGenBuffers(1, &cascadeMatricesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, cascadeMatricesUBO);
	glBufferData(GL_UNIFORM_BUFFER,  sizeof(glm::mat4x4) * 4, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, cascadeMatricesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	cascadeMatrices.resize(cascadeCount);

	LightingManager::GetInstance().AddDirectionalLight(this);
}




std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview)
{
	const auto inv = glm::inverse(projview);

	std::vector<glm::vec4> frustumCorners;
	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
			{
				const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
				frustumCorners.push_back(pt / pt.w);
			}
		}
	}

	return frustumCorners;
}
std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
	return getFrustumCornersWorldSpace(proj * view);
}

glm::mat4 DirectionalLight::GetLightSpaceMatrix(float nearPlane, float farPlane)
{
	Camera* cam = sceneManager.mainCamera;
	
	auto proj = glm::perspective(glm::radians(cam->fieldOfView), (float)WindowManager::GetInstance().windowWidth / (float)WindowManager::GetInstance().windowHeight, nearPlane, farPlane);
	auto corners = getFrustumCornersWorldSpace(proj, cam->view);

	glm::vec3 center = glm::vec3(0, 0, 0);
	for (const auto& v : corners)
	{
		center += glm::vec3(v);
	}
	center /= corners.size();

	auto lightView = glm::lookAt(center - transform->forward, center, glm::vec3(0.0f, 1.0f, 0.0f));

	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();
	for (const auto& v : corners)
	{
		const auto trf = lightView * v;
		minX = std::min(minX, trf.x);
		maxX = std::max(maxX, trf.x);
		minY = std::min(minY, trf.y);
		maxY = std::max(maxY, trf.y);
		minZ = std::min(minZ, trf.z);
		maxZ = std::max(maxZ, trf.z);
	}

	float zMult = 10.0f;
	if (minZ < 0)
	{
		minZ *= zMult;
	}
	else
	{
		minZ /= zMult;
	}
	if (maxZ < 0)
	{
		maxZ /= zMult;
	}
	else
	{
		maxZ *= zMult;
	}

	glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
	return lightProjection * lightView;
}

void DirectionalLight::BuildCascades()
{
	Camera* cam = sceneManager.mainCamera;

	for (int i = 0; i < cascadeCount; i++)
	{
		float nearP = (i == 0) ? cam->nearClippingPlane : cascadeSplits[i - 1];
		float farP  = (i == cascadeCount - 1) ?
			cam->farClippingPlane : cascadeSplits[i];

		cascadeMatrices[i] = GetLightSpaceMatrix(nearP, farP);
	}
}







PointLight::PointLight() : Component() 
{
	LightingManager::GetInstance().AddPointLight(this);
}

PointLight::PointLight(glm::vec3 color, float strength, float radius) : Component()
{
	this->color = color;
	this->strength = strength;
	this->radius = radius;

	LightingManager::GetInstance().AddPointLight(this);
}

PointLight::~PointLight()
{
	LightingManager::GetInstance().RemovePointLight(this);
}




SpotLight::SpotLight() : Component()
{
	Initialize();
}

SpotLight::SpotLight(glm::vec3 color, float strength, float distance, float angle) : Component()
{
	this->color = color;
	this->strength = strength;
	this->distance = distance;
	this->angle = angle;

	Initialize();
}

SpotLight::~SpotLight()
{
	LightingManager::GetInstance().RemoveSpotLight(this);
}


void SpotLight::Initialize()
{
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapResolution, shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	
	LightingManager::GetInstance().AddSpotLight(this);
}

glm::mat4 SpotLight::GetLightSpaceMatrix()
{
	glm::mat4 lightProjection = glm::perspective(glm::radians((angle + 5) * 2.0f), 1.0f, 0.1f, 100.0f);

	glm::mat4 lightView = glm::lookAt(transform->position, transform->position + transform->forward, transform->up);
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	return lightSpaceMatrix;
}