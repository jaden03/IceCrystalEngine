#include <Ice/Core/SceneManager.h>

#include <iostream>

#include <Ice/Core/LightingManager.h>
#include <Ice/Core/WindowManager.h>

#include <Ice/Components/Camera.h>
#include <Ice/Components/Renderer.h>
#include <Ice/Components/Light.h>

#include <Ice/Rendering/Shader.h>

LightingManager& lightingManager = LightingManager::GetInstance();
WindowManager& windowManager = WindowManager::GetInstance();

// Constructor
SceneManager::SceneManager()
{
	mainCamera = nullptr;
	deltaTime = 0.0f;
	actors = new std::vector<Actor*>();
}

// Deconstructor
SceneManager::~SceneManager()
{
	for (int i = 0; i < actors->size(); i++)
	{
		delete actors->at(i);
	}
	delete actors;
}

// Update
void SceneManager::Update()
{
	// get the mainCamera
	if (mainCamera == nullptr)
	{
		mainCamera = GetComponentOfType<Camera>();

		if (mainCamera == nullptr)
		{
			// create a fallback camera
			Actor* cameraActor = new Actor("Main Camera", "MainCamera");
			cameraActor->AddComponent<Camera>();

			// TODO : add proper logging
			std::cout << "No Camera Component found in Scene, A fallback Actor with a Camera Component has been created. You can access the Actor by the tag \"MainCamera\"." << std::endl;
		}
	}

	
	// shadows
	
	// bind to the shadow framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, lightingManager.shadowMapFBO);
	// use the shadow shader
	lightingManager.shadowShader->Use();

	// frontface culling
	glCullFace(GL_FRONT);

	// loop through directional lights
	for (int i = 0; i < lightingManager.directionalLights.size(); i++)
	{
		DirectionalLight* light = lightingManager.directionalLights[i];

		// if the light doesnt cast shadows, move on to the next one
		if (!light->castShadows) continue;

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, light->depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		// clear the framebuffer
		glClear(GL_DEPTH_BUFFER_BIT);
		
		// set the light space matrix
		lightingManager.shadowShader->setMat4("lightSpaceMatrix", light->GetLightSpaceMatrix());

		// set the viewport
		glViewport(0, 0, light->shadowMapResolution, light->shadowMapResolution);
		

		// loop through the actors
		for (int j = 0; j < actors->size(); j++)
		{
			Actor* actor = actors->at(j);

			// get the renderer
			Renderer* renderer = actor->GetComponent<Renderer>();

			if (renderer != nullptr)
			{
				// draw the renderer
				renderer->UpdateShadows();
			}
		}
	}

	// bind to the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// reset the viewport
	glViewport(0, 0, windowManager.windowWidth, windowManager.windowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.2f, 0.5f, 0.9f, 1.0f);

	// backface culling
	glCullFace(GL_BACK);

	// loop through actors
	for (int i = 0; i < actors->size(); i++)
	{
		// update the transform
		actors->at(i)->transform->Update();
		// loop through components
		for (int j = 0; j < actors->at(i)->components->size(); j++)
		{
			actors->at(i)->components->at(j)->Update();
		}
	}
}

// Add Actor
void SceneManager::AddActor(Actor* actor)
{
	actors->push_back(actor);
}


// Get Actor by Tag
Actor* SceneManager::GetActorByTag(std::string tag)
{
	for (int i = 0; i < actors->size(); i++)
	{
		if (actors->at(i)->tag == tag)
		{
			return actors->at(i);
		}
	}
	return nullptr;
}

// Get Actors by Tag
std::vector<Actor*> SceneManager::GetActorsByTag(std::string tag)
{
	std::vector<Actor*> actorsWithTag = std::vector<Actor*>();

	for (int i = 0; i < actors->size(); i++)
	{
		if (actors->at(i)->tag == tag)
		{
			actorsWithTag.push_back(actors->at(i));
		}
	}

	return actorsWithTag;
}



// Get Component of Type
template <typename T>
T* SceneManager::GetComponentOfType()
{
	for (int i = 0; i < actors->size(); i++)
	{
		for (int j = 0; j < actors->at(i)->components->size(); j++)
		{
			if (dynamic_cast<T*>(actors->at(i)->components->at(j)) != nullptr)
			{
				return dynamic_cast<T*>(actors->at(i)->components->at(j));
			}
		}
	}
	return nullptr;
}