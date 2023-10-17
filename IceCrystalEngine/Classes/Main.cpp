#include <iostream>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Ice/Core/WindowManager.h>
#include <Ice/Core/SceneManager.h>
#include <Ice/Core/SceneInitializer.h>
#include <Ice/Core/Input.h>
#include <Ice/Utils/FileUtil.h>

#include <Ice/Core/Transform.h>
#include <Ice/Components/Renderer.h>
#include <Ice/Rendering/Material.h>
#include <Ice/Rendering/Shader.h>
#include <Ice/Components/Freecam.h>

int main()
{
    // Get a reference to the WindowManager (this will initialize the window)
	WindowManager& windowManager = WindowManager::GetInstance();
    GLFWwindow* window = windowManager.window;

    // Enable the depth test
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
	
	// Initialize FileUtils
    FileUtil::InitializeStaticMembers();

	// Get a reference to the SceneManager
	SceneManager &sceneManager = SceneManager::GetInstance();

	// Get a reference to the SceneInitializer (this will initialize the scene)
	SceneInitializer& sceneInitializer = SceneInitializer::GetInstance();

    // Get a reference to Input
	Input& input = Input::GetInstance();

    // program loop
    float lastFrameTime = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        // calculate delta time
		float currentFrameTime = glfwGetTime();
		sceneManager.deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;
		
		// clear the screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// update the scene (this will update all components)
        sceneManager.Update();
		
		// swap buffers, clear the input, poll events
        glfwSwapBuffers(window);
        input.ClearInput();
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}