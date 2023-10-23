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
	
	float exposure = 1.0f;
	bool bloom = true;
	

private:

	WindowManager& windowManager = WindowManager::GetInstance();
	LightingManager& lightingManager = LightingManager::GetInstance();

	int lastScreenWidth;
	int lastScreenHeight;

	Shader* hdrShader;
	unsigned int colorBuffers[2];
	unsigned int hdrFBO;
	unsigned int depthRBO;

	
	// Bloom
	Shader* blurShader;
	unsigned int pingpongFBO[2];
	unsigned int pingpongBuffer[2];
	

	void RenderFullscreenQuad();

	PostProcessor();

	PostProcessor(PostProcessor const&) = delete; // Delete copy constructor
	// this prevents the copy constructor "SceneManager copy(original);" from working

	void operator=(PostProcessor const&) = delete; // Delete assignment operator
	// this prevents copying by assignment "SceneManager another = original;" from working

};

#endif