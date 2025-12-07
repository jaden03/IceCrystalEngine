#include <Ice/Core/SceneManager.h>

#include <iostream>

#include <Ice/Rendering/PostProcessor.h>
#include <Ice/Core/LightingManager.h>
#include <Ice/Core/WindowManager.h>
#include <Ice/Core/UIManager.h>
#include <Ice/Utils/FileUtil.h>
#include <Ice/Core/LuaManager.h>
#include <Ice/Core/RendererManager.h>

#include <Ice/Components/Camera.h>
#include <Ice/Components/Renderer.h>
#include <Ice/Components/Light.h>

#include <Ice/Core/Skybox.h>

using namespace std::chrono;

PostProcessor& postProcessor = PostProcessor::GetInstance();
LightingManager& lightingManager = LightingManager::GetInstance();
WindowManager& windowManager = WindowManager::GetInstance();
UIManager& uiManager = UIManager::GetInstance();
Skybox& skybox = Skybox::GetInstance();
RendererManager& rendererManager = RendererManager::GetInstance();

// Constructor
SceneManager::SceneManager()
{
	mainCamera = nullptr;
	deltaTime = 0.0f;
	actors = new std::vector<Actor*>();
	usedActorColors = std::vector<glm::vec3>();
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
	usedTextureCount = 10;
	
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
	mainCamera->Update();
	
	// bind to the shadow framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, lightingManager.shadowMapFBO);

	// frontface culling
	glCullFace(GL_FRONT);

	// directional light
	DirectionalLight* directionalLight = lightingManager.directionalLight;
	// if the light doesnt cast shadows, move on to the next one
	if (directionalLight != nullptr && directionalLight->castShadows)
	{
		// use the cascaded shadow shader
		lightingManager.shadowsCascadedShader->Use();
		
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, directionalLight->depthMapArray, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		// clear the framebuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// set the viewport
		glViewport(0, 0, directionalLight->shadowMapResolution, directionalLight->shadowMapResolution);

		// Setup the data for the UBO
		directionalLight->BuildCascades();
		glBindBuffer(GL_UNIFORM_BUFFER, directionalLight->cascadeMatricesUBO);
		for (int i = 0; i < directionalLight->cascadeCount; ++i)
		{
			glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &directionalLight->cascadeMatrices[i]);
		}
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		
		// loop through the actors
		for (int j = 0; j < actors->size(); j++)
		{
			Actor* actor = actors->at(j);

			// get the renderer
			Renderer* renderer = actor->GetComponent<Renderer>();

			if (renderer != nullptr && renderer->castShadows)
			{
				// draw the renderer
				renderer->UpdateShadows();
			}
		}
	}
	

	// use the normal shadow shader
	lightingManager.shadowShader->Use();

	// loop through spotlights
	for (int i = 0; i < lightingManager.spotLights.size(); i++)
	{
		SpotLight* light = lightingManager.spotLights[i];

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

			if (renderer != nullptr && renderer->castShadows)
			{
				// draw the renderer
				renderer->UpdateShadows();
			}
		}
	}

	// bind to the hdr framebuffer
	postProcessor.Bind();
	// reset the viewport
	glViewport(0, 0, windowManager.windowWidth, windowManager.windowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// backface culling
	glCullFace(GL_BACK);
	
	// update LuaManager
	LuaManager::GetInstance().Update(gameTime);

	// update UBOs for renderning
	rendererManager.UpdateUBOs();

	Actor* currentHoveredActor = nullptr;
	glm::vec3 hoveredColor = postProcessor.hoveredActorColor;
	// loop through actors to update components (also get the hovered actor here to save having to loop elsewhere)
	for (int i = 0; i < actors->size(); i++)
	{
		// for some reason I needed to update the child positions in Update and the rotations in LateUpdate
		actors->at(i)->transform->Update();

		// Hovered actor
		if (actors->at(i)->uniqueColor == hoveredColor)
		{
			// return the actor
			currentHoveredActor = actors->at(i);
		}
		
		// loop through components
		for (int j = 0; j < actors->at(i)->components->size(); j++)
		{
			actors->at(i)->components->at(j)->Update();
		}
	}
	hoveredActor = currentHoveredActor;
	
	// loop through actors again for LateUpdate and transform update
	for (int i = 0; i < actors->size(); i++)
	{
		// update the transform
		actors->at(i)->transform->LateUpdate();
		// loop through components
		for (int j = 0; j < actors->at(i)->components->size(); j++)
		{
			actors->at(i)->components->at(j)->LateUpdate();
		}
	}

	// Get the current polygon mode
	GLint currentPolygonMode;
	glGetIntegerv(GL_POLYGON_MODE, &currentPolygonMode);

	// set the polygon mode to fill
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	// render the skybox
	skybox.Render();

	// render the scene onto the quad
	postProcessor.Render();

	// render UI
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// loop through actors again for OverlayUpdate
	for (int i = 0; i < actors->size(); i++)
	{
		// loop through components
		for (int j = 0; j < actors->at(i)->components->size(); j++)
		{
			actors->at(i)->components->at(j)->OverlayUpdate();
		}
	}
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// set the polygon mode back to what it was before
	glPolygonMode(GL_FRONT_AND_BACK, currentPolygonMode);

	// Get current time
	double now = duration<double>(steady_clock::now().time_since_epoch()).count();
	// First frame bootstrap
	if (lastFrameTime == 0.0)
	{
		lastFrameTime = now;
	}
	// deltaTime = difference
	deltaTime = static_cast<float>(now - lastFrameTime);
	// Accumulate game time
	gameTime += deltaTime;
	// Store for next frame
	lastFrameTime = now;
}

// Add Actor
void SceneManager::AddActor(Actor* actor)
{
	actors->push_back(actor);
	
	// generate a random color 1-255 1-255 1-255
	glm::vec3 color = glm::vec3(rand() % 255 + 1, rand() % 255 + 1, rand() % 255 + 1);
	// if the color is already used, generate a new one
	while (std::find(usedActorColors.begin(), usedActorColors.end(), color) != usedActorColors.end())
	{
		color = glm::vec3(rand() % 255 + 1, rand() % 255 + 1, rand() % 255 + 1);
	}
	// add the color to the used colors
	usedActorColors.push_back(color);
	// set the color
	actor->uniqueColor = color;
}

// Remove Actor
void SceneManager::RemoveActor(Actor* actor)
{
	// loop through the actors
	for (int i = 0; i < actors->size(); i++)
	{
		// if the actor is found
		if (actors->at(i) == actor)
		{
			// remove actor->uniqueColor from usedActorColors
			usedActorColors.erase(std::remove(usedActorColors.begin(), usedActorColors.end(), actor->uniqueColor), usedActorColors.end());
			// remove the actor
			actors->erase(actors->begin() + i);
			// break out of the loop
			break;
		}
	}
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



// Get Actor Count
int SceneManager::GetActorCount()
{
	return actors->size();
}