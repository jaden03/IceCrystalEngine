#include <iostream>
#include <filesystem>

#ifdef _DEBUG
	#include <Ice/Utils/DebugUtil.h>
	#include <imgui/imgui_impl_glfw.h>
	#include <imgui/imgui_impl_opengl3.h>
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Ice/Core/WindowManager.h>
#include <Ice/Core/SceneManager.h>
#include <Ice/Core/SceneInitializer.h>
#include <Ice/Core/LightingManager.h>
#include <Ice/Core/Input.h>
#include <Ice/Utils/FileUtil.h>

#include <Ice/Core/Transform.h>
#include <Ice/Components/Renderer.h>
#include <Ice/Rendering/Material.h>
#include <Ice/Rendering/Shader.h>
#include <Ice/Components/Freecam.h>

int main()
{
    // Initialize FileUtils
    FileUtil::InitializeStaticMembers();

    // Get a reference to the WindowManager (this will initialize the window)
	WindowManager& windowManager = WindowManager::GetInstance();
	
	// Get a reference to the SceneManager (this has to happen before the scene is initialized)
	SceneManager &sceneManager = SceneManager::GetInstance();

    // Get a reference to the LightingManager
    LightingManager& lightingManager = LightingManager::GetInstance();
    lightingManager.InitializeLighting();

    // Get a reference to the SceneInitializer (this will initialize the scene)
    SceneInitializer& sceneInitializer = SceneInitializer::GetInstance();
	
    // Get a reference to Input
	Input& input = Input::GetInstance();

	
#ifdef _DEBUG
	// Get a reference to the debugUtil
	DebugUtil& debugUtil = DebugUtil::GetInstance();
#endif


    Actor* pointLight1 = sceneManager.GetActorByTag("PointLight1");
    Actor* pointLight2 = sceneManager.GetActorByTag("PointLight2");
	Actor* pointLight3 = sceneManager.GetActorByTag("PointLight3");
	
	Actor* sun = sceneManager.GetActorByTag("sun");
	Actor* testActor = sceneManager.GetActorByTag("Test");


	

	
    // program loop
    float lastFrameTime = 0.0f;
    while (!glfwWindowShouldClose(windowManager.window))
    {
        // calculate delta time
		float currentFrameTime = glfwGetTime();
		sceneManager.deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;

		// Tell imGUI to start a new frame
#ifdef _DEBUG
		debugUtil.StartOfFrame();
		if (input.GetKeyDown(GLFW_KEY_GRAVE_ACCENT))
			debugUtil.showConsole = !debugUtil.showConsole;
#endif

		// update the scene (this will update all components)
        sceneManager.Update();

        if (input.GetKey(GLFW_KEY_UP))
			testActor->transform->TranslateDelta(testActor->transform->forward * 5.0f);
		if (input.GetKey(GLFW_KEY_DOWN))
			testActor->transform->TranslateDelta(-testActor->transform->forward * 5.0f);
		if (input.GetKey(GLFW_KEY_LEFT))
			testActor->transform->RotateDelta(0, -90, 0);
		if (input.GetKey(GLFW_KEY_RIGHT))
			testActor->transform->RotateDelta(0, 90, 0);

		if (input.GetKey(GLFW_KEY_0))
		{
			sun->transform->RotateDelta(90, 0, 0);
		}
		if (input.GetKey(GLFW_KEY_9))
		{
			sun->transform->RotateDelta(-90, 0, 0);
		}
		if (input.GetKey(GLFW_KEY_8))
		{
			testActor->transform->RotateDelta(-90, 0, 0);
		}
		if (input.GetKey(GLFW_KEY_7))
		{
			testActor->transform->RotateDelta(90, 0, 0);
		}
		if (input.GetKey(GLFW_KEY_6))
		{
			pointLight1->transform->RotateLocalDelta(90, 0, 0);
		}
		if (input.GetKeyDown(GLFW_KEY_5))
		{
			std::cout << "5" << std::endl;
		}
		if (input.GetKeyDown(GLFW_KEY_4))
		{
			std::cout << "4" << std::endl;
		}

		
		pointLight1->transform->position.x = 0 + 3 * cos(glfwGetTime());
		pointLight1->transform->position.z = 5 + 3 * sin(glfwGetTime());
		
		pointLight2->transform->position.x = 0 + 3 * cos(glfwGetTime() +  2);
        pointLight2->transform->position.z = 5 + 3 * sin(glfwGetTime() + 2);
		
		pointLight3->transform->position.x = 0 + 3 * cos(glfwGetTime() + 4);
        pointLight3->transform->position.z = 5 + 3 * sin(glfwGetTime() + 4);
		

		
		// imgui debug stuff
#ifdef _DEBUG
		debugUtil.EndOfFrame();
#endif

		// swap buffers, clear the input, poll events
        glfwSwapBuffers(windowManager.window);
        input.ClearInput();
        glfwPollEvents();
    }

	// cleanup
#ifdef _DEBUG
	debugUtil.Cleanup();
#endif

    glfwTerminate();
    return 0;
}