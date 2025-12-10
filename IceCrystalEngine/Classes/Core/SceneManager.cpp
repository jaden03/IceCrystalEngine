#include <Ice/Core/SceneManager.h>

#include <iostream>

#include <Ice/Rendering/PostProcessor.h>
#include <Ice/Core/LightingManager.h>
#include <Ice/Core/WindowManager.h>
#include <Ice/Core/UIManager.h>
#include <Ice/Utils/FileUtil.h>
#include <Ice/Core/LuaManager.h>
#include <Ice/Core/RendererManager.h>
#include <Ice/Core/Input.h>

#include <Ice/Components/Camera.h>
#include <Ice/Components/Renderer.h>
#include <Ice/Components/Light.h>
#include <Ice/Components/Freecam.h>
#include <Ice/Components/Physics/RigidBody.h>

#include <Ice/Core/Skybox.h>
#include <Ice/Editor/GizmoRenderer.h>
#include <Ice/Editor/WebEditorManager.h>
#include <Ice/Editor/EditorUI.h>

#include <imgui/imgui.h>
#include <typeinfo>

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
	WebEditorManager& webEditor = WebEditorManager::GetInstance();
	EditorUI& editorUI = EditorUI::GetInstance();
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
	
	Input& input = Input::GetInstance();

	// Sync EditorUI pause state with WebEditor (if web editor is running, it takes precedence)
	bool isEnginePaused = editorUI.IsEnginePaused() || (webEditor.IsRunning() && webEditor.IsEnginePaused());

	// Only update Lua/scripts when engine is not paused (allows editing in pause mode)
	if (!isEnginePaused)
	{
		LuaManager::GetInstance().Update(gameTime);
	}

	// Handle gizmo keyboard shortcuts when engine is paused
	if (isEnginePaused && !input.GetKeyDown(GLFW_MOUSE_BUTTON_2))
	{
		GizmoRenderer& gizmoRenderer = GizmoRenderer::GetInstance();
		
		// Q = Deselect/Select mode
		if (input.GetKeyDown(GLFW_KEY_Q))
		{
			if (editorUI.HasSelectedActor())
			{
				// Clear selection
				editorUI.SetSelectedActor(nullptr);
				webEditor.SetSelectedActor(0);
				std::cout << "[Editor] Cleared selection" << std::endl;
			}
		}
		
		// W = Translate mode
		if (input.GetKeyDown(GLFW_KEY_W))
		{
			gizmoRenderer.SetMode(GizmoMode::Translate);
			std::cout << "[Editor] Gizmo mode: Translate" << std::endl;
		}
		
		// E = Rotate mode
		if (input.GetKeyDown(GLFW_KEY_E))
		{
			gizmoRenderer.SetMode(GizmoMode::Rotate);
			std::cout << "[Editor] Gizmo mode: Rotate" << std::endl;
		}
		
		// R = Scale mode
		if (input.GetKeyDown(GLFW_KEY_R))
		{
			gizmoRenderer.SetMode(GizmoMode::Scale);
			std::cout << "[Editor] Gizmo mode: Scale" << std::endl;
		}
		
		// G = Toggle gizmo visibility
		if (input.GetKeyDown(GLFW_KEY_G))
		{
			gizmoRenderer.SetEnabled(!gizmoRenderer.IsEnabled());
			std::cout << "[Editor] Gizmos " << (gizmoRenderer.IsEnabled() ? "enabled" : "disabled") << std::endl;
		}
	}

	// Handle gizmo input and actor selection when engine is paused
	if (isEnginePaused && mainCamera != nullptr)
	{
		// Check if ImGui wants to capture mouse input (user is interacting with UI)
		ImGuiIO& io = ImGui::GetIO();
		bool imguiWantsMouse = io.WantCaptureMouse;
		
		glm::vec2 mousePos = input.GetMousePosition();
		glm::vec2 screenSize(windowManager.windowWidth, windowManager.windowHeight);
		
		// Use EditorUI's selected actor (priority), fallback to WebEditor's
		Actor* selectedActor = editorUI.GetSelectedActor();
		if (!selectedActor && webEditor.IsRunning())
		{
			selectedActor = webEditor.GetSelectedActor();
		}
		
		// If we have a selected actor, handle gizmo interaction (only if ImGui doesn't want the mouse)
		if (selectedActor != nullptr && !imguiWantsMouse)
		{
			GizmoRenderer& gizmoRenderer = GizmoRenderer::GetInstance();
			if (gizmoRenderer.IsEnabled())
			{
				
				// Continuously update hovered axis for highlighting (even when not dragging)
				if (!gizmoRenderer.IsDragging())
				{
					glm::vec3 gizmoPos = selectedActor->transform->position;
					gizmoRenderer.GetHoveredAxis(mousePos, screenSize, mainCamera->view, mainCamera->projection, gizmoPos);
				}
				
				// Handle mouse input for gizmo interaction
				if (input.GetMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT))
				{
					gizmoRenderer.HandleMouseDown(mousePos, screenSize, mainCamera->view, mainCamera->projection, selectedActor);
				}
				
				if (input.GetMouseButton(GLFW_MOUSE_BUTTON_LEFT))
				{
					gizmoRenderer.HandleMouseMove(mousePos, screenSize, mainCamera->view, mainCamera->projection);
				}
				
				if (input.GetMouseButtonUp(GLFW_MOUSE_BUTTON_LEFT))
				{
					gizmoRenderer.HandleMouseUp();
				}
			}
		}
		
		// If left click and not dragging gizmo, try to select an actor by clicking on it (only if ImGui doesn't want the mouse)
		if (input.GetMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT) && !imguiWantsMouse)
		{
			GizmoRenderer& gizmoRenderer = GizmoRenderer::GetInstance();
			
			// Only allow actor selection if not currently dragging a gizmo
			if (!gizmoRenderer.IsDragging())
			{
				// Check if we clicked on a gizmo first
				bool clickedOnGizmo = false;
				if (selectedActor != nullptr && gizmoRenderer.IsEnabled())
				{
					glm::vec3 gizmoPos = selectedActor->transform->position;
					GizmoAxis hitAxis = gizmoRenderer.GetHoveredAxis(mousePos, screenSize, mainCamera->view, mainCamera->projection, gizmoPos);
					clickedOnGizmo = (hitAxis != GizmoAxis::None);
				}
				
				// If we didn't click on a gizmo, check for actor selection
				if (!clickedOnGizmo && hoveredActor != nullptr)
				{
					// Select the hovered actor in both editors
					editorUI.SetSelectedActor(hoveredActor);
					if (webEditor.IsRunning())
					{
						int actorId = static_cast<int>(reinterpret_cast<intptr_t>(hoveredActor));
						webEditor.SetSelectedActor(actorId);
					}
					std::cout << "[Editor] Selected actor: " << hoveredActor->name << std::endl;
				}
			}
		}
	}

	// update UBOs for renderning
	rendererManager.UpdateUBOs();

	Actor* currentHoveredActor = nullptr;
	glm::vec3 hoveredColor = postProcessor.hoveredActorColor;
	// loop through actors to update components (also get the hovered actor here to save having to loop elsewhere)
	for (int i = 0; i < actors->size(); i++)
	{
		// Always update transforms for rendering (child position calculations)
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
			Component* component = actors->at(i)->components->at(j);
			
			// Always update rendering components (Camera, Renderer, DirectionalLight, PointLight, SpotLight)
			// and editor components (Freecam for camera control, RigidBody for transform sync)
			// Only update gameplay components when not paused
			bool isRenderingComponent = (dynamic_cast<Camera*>(component) != nullptr ||
			                              dynamic_cast<Renderer*>(component) != nullptr ||
			                              dynamic_cast<DirectionalLight*>(component) != nullptr ||
			                              dynamic_cast<PointLight*>(component) != nullptr ||
			                              dynamic_cast<SpotLight*>(component) != nullptr ||
			                              dynamic_cast<Freecam*>(component) != nullptr ||
			                              dynamic_cast<RigidBody*>(component) != nullptr);
			
			if (isRenderingComponent || !isEnginePaused)
			{
				component->Update();
			}
		}
	}
	hoveredActor = currentHoveredActor;
	
	// loop through actors again for LateUpdate and transform update
	for (int i = 0; i < actors->size(); i++)
	{
		// Always update transform LateUpdate for rendering (calculates forward/right/up vectors and rotation)
		actors->at(i)->transform->LateUpdate();
		
		// loop through components
		for (int j = 0; j < actors->at(i)->components->size(); j++)
		{
			Component* component = actors->at(i)->components->at(j);
			
			// Always update rendering components (Camera, Renderer, DirectionalLight, PointLight, SpotLight)
			// and editor components (Freecam for camera control, RigidBody for transform sync)
			// Only update gameplay components when not paused
			bool isRenderingComponent = (dynamic_cast<Camera*>(component) != nullptr ||
			                              dynamic_cast<Renderer*>(component) != nullptr ||
			                              dynamic_cast<DirectionalLight*>(component) != nullptr ||
			                              dynamic_cast<PointLight*>(component) != nullptr ||
			                              dynamic_cast<SpotLight*>(component) != nullptr ||
			                              dynamic_cast<Freecam*>(component) != nullptr ||
			                              dynamic_cast<RigidBody*>(component) != nullptr);
			
			if (isRenderingComponent || !isEnginePaused)
			{
				component->LateUpdate();
			}
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

	// Render gizmos for selected actor only when engine is paused (edit mode)
	//WebEditorManager& webEditor = WebEditorManager::GetInstance();
	if (isEnginePaused && mainCamera != nullptr)
	{
		GizmoRenderer& gizmoRenderer = GizmoRenderer::GetInstance();
		if (gizmoRenderer.IsEnabled())
		{
			// Use EditorUI's selected actor (priority), fallback to WebEditor's
			Actor* selectedActor = editorUI.GetSelectedActor();
			if (!selectedActor && webEditor.IsRunning())
			{
				selectedActor = webEditor.GetSelectedActor();
			}
			
			if (selectedActor != nullptr)
			{
				gizmoRenderer.RenderGizmos(selectedActor, mainCamera);
			}
		}
	}

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
	return static_cast<int>(actors->size());
}