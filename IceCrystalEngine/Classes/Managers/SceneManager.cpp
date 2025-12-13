#include <Ice/Managers/SceneManager.h>

#include <iostream>

SceneManager& SceneManager::GetInstance()
{
    static SceneManager instance; // Static local variable ensures a single instance
    return instance;
}

#include <Ice/Rendering/PostProcessor.h>
#include <Ice/Managers/LightingManager.h>
#include <Ice/Managers/WindowManager.h>
#include <Ice/Core/UIManager.h>
#include <Ice/Utils/FileUtil.h>
#include <Ice/Managers/LuaManager.h>
#include <Ice/Managers/RendererManager.h>
#include <Ice/Core/Input.h>

#include <Ice/Components/Camera.h>
#include <Ice/Components/Rendering/Renderer.h>
#include <Ice/Components/Rendering/Light.h>
#include <Ice/Components/Freecam.h>
#include <Ice/Components/Physics/RigidBody.h>

#include <Ice/Core/Skybox.h>
#include <Ice/IEditor/GizmoRenderer.h>
#include <Ice/IEditor/EditorCamera.h>
#include <Ice/IEditor/WebEditorManager.h>
#include <Ice/IEditor/EditorUI.h>

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
			mainCamera = cameraActor->AddComponent<Camera>();
		
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

	// Bind to the appropriate framebuffer (viewport in editor, or HDR for post-processing)
	#ifdef _DEBUG
	if (editorUI.IsViewportActive())
	{
		// Render to editor viewport framebuffer
		unsigned int fbo = editorUI.GetViewportFramebuffer();
		int vpWidth = editorUI.GetViewportWidth();
		int vpHeight = editorUI.GetViewportHeight();
		
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		
		// IMPORTANT: Explicitly set draw buffers after binding FBO
		// OpenGL may not remember this from framebuffer creation
		GLenum drawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, drawBuffers);
		
		glViewport(0, 0, vpWidth, vpHeight);
		glClearColor(0.1f, 0.1f, 0.15f, 1.0f); // Slightly visible clear color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	else
	#endif
	{
		// Render to HDR framebuffer for post-processing
		postProcessor.Bind();
		glViewport(0, 0, windowManager.windowWidth, windowManager.windowHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}


	// backface culling
	glCullFace(GL_BACK);
	
	Input& input = Input::GetInstance();

	// Get current play mode
	PlayMode currentPlayMode = editorUI.GetPlayMode();
	bool isEditMode = (currentPlayMode == PlayMode::EDIT);
	bool isPlayingGame = (currentPlayMode == PlayMode::PLAY);

	// Global shortcut: Ctrl+P to toggle play mode
	if (input.GetKeyDown(GLFW_KEY_P) && (input.GetKey(GLFW_KEY_LEFT_CONTROL) || input.GetKey(GLFW_KEY_RIGHT_CONTROL)))
	{
		if (currentPlayMode == PlayMode::EDIT)
		{
			editorUI.SetPlayMode(PlayMode::PLAY);
			std::cout << "[Editor] Starting Play Mode (Ctrl+P)" << std::endl;
		}
		else
		{
			editorUI.SetPlayMode(PlayMode::EDIT);
			std::cout << "[Editor] Stopping - Returning to Edit Mode (Ctrl+P)" << std::endl;
		}
	}

	// Only update Lua/scripts when game is actually running (PLAY mode)
	if (isPlayingGame)
	{
		LuaManager::GetInstance().Update(gameTime);
	}

	// Handle gizmo keyboard shortcuts only in EDIT mode
	if (isEditMode && !input.GetKeyDown(GLFW_MOUSE_BUTTON_2))
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

	// Update editor camera when in EDIT mode and viewport is active
	#ifdef _DEBUG
	if (isEditMode && editorUI.IsViewportActive())
	{
		EditorCamera& editorCam = EditorCamera::GetInstance();
		editorCam.UpdateProjectionMatrix((float)editorUI.GetViewportWidth(), (float)editorUI.GetViewportHeight());
		
		// Always update camera in edit mode (it handles its own focus/input logic)
		bool isMouseInViewport = editorUI.IsMouseInViewport();
		glm::vec2 viewportMousePos = editorUI.GetViewportMousePos();
		editorCam.Update(deltaTime, isMouseInViewport, viewportMousePos);
	}
	#endif

	// Handle gizmo input and actor selection only in EDIT mode
	if (isEditMode && mainCamera != nullptr)
	{
		// Check if ImGui wants to capture mouse input (user is interacting with UI)
		// Only check if ImGui context exists (it won't exist in non-editor builds)
		bool imguiWantsMouse = false;
		if (ImGui::GetCurrentContext() != nullptr)
		{
			ImGuiIO& io = ImGui::GetIO();
			imguiWantsMouse = io.WantCaptureMouse;
		}
		
		// Use viewport-relative coordinates when viewport is active
		glm::vec2 mousePos;
		glm::vec2 screenSize;
		bool useViewportCoords = false;
		Camera* cameraToUse = mainCamera;
		
		#ifdef _DEBUG
		if (editorUI.IsViewportActive())
		{
			// Always use editor camera when viewport is active (regardless of mouse position)
			EditorCamera& editorCam = EditorCamera::GetInstance();
			mainCamera->view = editorCam.view;
			mainCamera->projection = editorCam.projection;
			cameraToUse = mainCamera;
			
			// Use viewport-relative mouse coordinates if mouse is in viewport
			if (editorUI.IsMouseInViewport())
			{
				mousePos = editorUI.GetViewportMousePos();
				screenSize = editorUI.GetViewportSize();
				useViewportCoords = true;
			}
			else
			{
				// Mouse outside viewport - use window coords but still use editor camera
				mousePos = input.GetMousePosition();
				screenSize = glm::vec2(windowManager.windowWidth, windowManager.windowHeight);
			}
		}
		else
		#endif
		{
			// Use full window coordinates
			mousePos = input.GetMousePosition();
			screenSize = glm::vec2(windowManager.windowWidth, windowManager.windowHeight);
		}
		
		// Use EditorUI's selected actor (priority), fallback to WebEditor's
		Actor* selectedActor = editorUI.GetSelectedActor();
		if (!selectedActor && webEditor.IsRunning())
		{
			selectedActor = webEditor.GetSelectedActor();
		}
		
		// If we have a selected actor, handle gizmo interaction
		// Only allow interaction if mouse is in viewport (in editor mode)
		bool allowGizmoInteraction = useViewportCoords;
		
		if (selectedActor != nullptr && allowGizmoInteraction)
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
		
		// If left click and not dragging gizmo, try to select an actor by clicking on it
		if (input.GetMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT) && allowGizmoInteraction)
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
				
				// If we didn't click on a gizmo, handle actor selection
				if (!clickedOnGizmo)
				{
					if (hoveredActor != nullptr)
					{
						// Select the hovered actor
						editorUI.SetSelectedActor(hoveredActor);
						if (webEditor.IsRunning())
						{
							int actorId = static_cast<int>(reinterpret_cast<intptr_t>(hoveredActor));
							webEditor.SetSelectedActor(actorId);
						}
						std::cout << "[Editor] Selected actor: " << hoveredActor->name << std::endl;
					}
					else
					{
						// Clicked on empty space - deselect
						if (selectedActor != nullptr)
						{
							std::cout << "[Editor] Deselected actor: " << selectedActor->name << std::endl;
							editorUI.SetSelectedActor(nullptr);
							if (webEditor.IsRunning())
							{
								webEditor.SetSelectedActor(0);
							}
						}
					}
				}
			}
		}
	}

	// update UBOs for rendering
	rendererManager.UpdateUBOs();

	// Update transforms hierarchically - only in PLAY mode (not in EDIT mode)
	// In EDIT mode, transforms are updated by gizmo manipulation only
	for (int i = 0; i < actors->size(); i++)
	{
		if (actors->at(i)->transform->parent == nullptr)
		{
			actors->at(i)->transform->Update();
		}
	}

	Actor* currentHoveredActor = nullptr;
	glm::vec3 hoveredColor;
	
	// Will read picking color AFTER rendering is complete
	
	// loop through actors to update components (also get the hovered actor here to save having to loop elsewhere)
	for (int i = 0; i < actors->size(); i++)
	{
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
			
			if (isRenderingComponent || isPlayingGame)
			{
				component->Update();
			}
		}
	}

	// lateupdate
	RunService::GetInstance().FireLateUpdate(deltaTime);
	for (int i = 0; i < actors->size(); i++)
	{
		// update the transform
		//actors->at(i)->transform->LateUpdate();
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
			
			if (isRenderingComponent || isPlayingGame)
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

	// Render gizmos for selected actor BEFORE unbinding FBO (so they appear in viewport)
	// Only in EDIT mode
	#ifdef _DEBUG
	if (isEditMode && mainCamera != nullptr)
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
				// Use editor camera when rendering to viewport
				Camera* renderCamera = mainCamera;
				if (editorUI.IsViewportActive())
				{
					EditorCamera& editorCam = EditorCamera::GetInstance();
					mainCamera->view = editorCam.view;
					mainCamera->projection = editorCam.projection;
				}
				
				gizmoRenderer.RenderGizmos(selectedActor, mainCamera);
			}
		}
	}
	#endif

	// render the scene onto the quad
	// Only render post-processing if not rendering to viewport
	#ifdef _DEBUG
	if (!editorUI.IsViewportActive())
	#endif
	{
		postProcessor.Render();
	}
	
	// Unbind framebuffer when rendering to viewport
	#ifdef _DEBUG
	if (editorUI.IsViewportActive())
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	#endif

	// NOW read picking color AFTER scene has been rendered to viewport
	#ifdef _DEBUG
	if (isEditMode && editorUI.IsViewportActive())
	{
		// Use viewport picking texture in edit mode - read AFTER rendering
		hoveredColor = editorUI.GetViewportPickedColor();
	}
	else
	#endif
	{
		// Use post-processor picking in game mode
		hoveredColor = postProcessor.hoveredActorColor;
	}
	
	// Find which actor matches the hovered color
	for (int i = 0; i < actors->size(); i++)
	{
		if (actors->at(i)->uniqueColor == hoveredColor)
		{
			currentHoveredActor = actors->at(i);
			break;
		}
	}
	
	// Update hovered actor and log changes
	#ifdef _DEBUG
	if (isEditMode && hoveredActor != currentHoveredActor)
	{
		if (currentHoveredActor != nullptr)
		{
			std::cout << "[Editor] Hovering over: " << currentHoveredActor->name << std::endl;
		}
		else if (hoveredActor != nullptr)
		{
			std::cout << "[Editor] No longer hovering" << std::endl;
		}
	}
	#endif
	hoveredActor = currentHoveredActor;

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
	return static_cast<int>(actors->size());
}