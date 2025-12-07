#pragma once

#ifndef POST_PROCESSOR_H

#define POST_PROCESSOR_H

#include <Ice/Core/WindowManager.h>
#include <Ice/Core/LightingManager.h>

class Shader;

class PostProcessor
{
	
public:

	static PostProcessor& GetInstance()
	{
		static PostProcessor instance; // Static local variable ensures a single instance
		return instance;
	}

	void Bind();
	void Render();
	
	// Post Processing
	float exposure = 1.0f;
	
	bool bloom = true;

	// this will blur the entire scene, useful if you want to blur the background
	// when you display UI or something
	int blurIterations = 0;

	// this is for the poor mans raycasting
	glm::vec3 hoveredActorColor;

private:

	WindowManager& windowManager = WindowManager::GetInstance();
	LightingManager& lightingManager = LightingManager::GetInstance();

	int lastScreenWidth;
	int lastScreenHeight;

	unsigned int multisampledColorBuffers[3];
	unsigned int multisampledFBO;
	unsigned int multisampledRBO;

	Shader* hdrShader;
	unsigned int colorBuffers[3];
	unsigned int hdrFBO;
	unsigned int depthRBO;

	
	// Bloom
	Shader* blurShader;
	unsigned int bloomPingpongFBO[2];
	unsigned int bloomPingpongBuffer[2];

	// Blur
	unsigned int blurPingpongFBO[2];
	unsigned int blurPingpongBuffer[2];
	

	void RenderFullscreenQuad();

	PostProcessor();

	PostProcessor(PostProcessor const&) = delete; // Delete copy constructor
	// this prevents the copy constructor "SceneManager copy(original);" from working

	void operator=(PostProcessor const&) = delete; // Delete assignment operator
	// this prevents copying by assignment "SceneManager another = original;" from working

};

#endif