#pragma once
#ifndef RENDERER_H

#define RENDERER_H

#include <Ice/Rendering/Material.h>
#include <Ice/Rendering/MeshHolder.h>
#include <Ice/Utils/OBJLoader.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Ice/Core/Component.h>

class Renderer : public Component
{

	void InitializeRenderer();

	WindowManager& windowManager = WindowManager::GetInstance();
	SceneManager& sceneManager = SceneManager::GetInstance();
	LightingManager& lightingManager = LightingManager::GetInstance();

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat3 normalMatrix = glm::mat3(1.0f);

	glm::vec3 lastPosition = glm::vec3(0.0f);
	glm::vec3 lastEulerAngles = glm::vec3(0.0f);
	glm::vec3 lastScale = glm::vec3(1.0f);

	std::string ModelPath;


	static constexpr uint32_t CACHE_VERSION = 1;

	std::string GetCachePath();
	bool IsCacheValid();
	bool LoadFromCache();
	void SaveToCache();
	bool LoadFromOBJ();
	void CreateGLBuffers();

public:
	
	bool castShadows = true;

	Material* material;

	std::vector<MeshHolder> meshHolders;

	Renderer();
	Renderer(std::string modelPath);
	Renderer(std::string modelPath, Material* material);

	virtual ~Renderer();
	
	void Update() override;
	void UpdateShadows();
};

#endif