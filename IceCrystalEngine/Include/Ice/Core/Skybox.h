#pragma once

#ifndef SKYBOX_H

#include <Ice/Components/Camera.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Skybox
{
	unsigned int cubemapTextureHandle, skyboxVAO, skyboxVBO;
	Shader* shader = nullptr;
	
	SceneManager& sceneManager = SceneManager::GetInstance();
	WindowManager& windowManager = WindowManager::GetInstance();
	
	void InitializeSkybox();

	Skybox();
	
	Skybox(Skybox const&) = delete; // Delete copy constructor
	// this prevents the copy constructor "SceneManager copy(original);" from working

	void operator=(Skybox const&) = delete; // Delete assignment operator
	// this prevents copying by assignment "SceneManager another = original;" from working

public:

	static Skybox& GetInstance()
	{
		static Skybox instance; // Static local variable ensures a single instance
		return instance;
	}

	std::string SkyboxPath = "{ASSET_DIR}Textures/spaceSkybox"; // this is a path to a folder
	// the folder contains "right.png", "left.png", "top.png", "bottom.png", "front.png", "back.png"

	void Render();
	
};


#endif