#include <iostream>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Ice/Core/WindowManager.h>
#include <Ice/Core/SceneManager.h>
#include <Ice/Utils/FileUtil.h>

#include <Ice/Core/Transform.h>
#include <Ice/Components/Renderer.h>
#include <Ice/Rendering/Material.h>
#include <Ice/Rendering/Shader.h>

void processInput(GLFWwindow* window);

int main()
{
    // Get a reference to the WindowManager (this will initialize the window)
	WindowManager& windowManager = WindowManager::GetInstance();
    GLFWwindow* window = windowManager.window;

    // Enable the depth test
    glEnable(GL_DEPTH_TEST);
	
	// Initialize FileUtils
    FileUtil::InitializeStaticMembers();

	// Get a reference to the SceneManager
	SceneManager &sceneManager = SceneManager::GetInstance();


    Actor* cameraActor = new Actor("Main Camera");
    cameraActor->AddComponent<Camera>();

    Actor* testActor = new Actor("Test Actor", "Test");
	Material* material = new Material(FileUtil::AssetDir + "Materials/object.mat");
	Renderer* renderer = new Renderer(FileUtil::AssetDir + "Models/finch.obj", material);
    testActor->AddComponent(renderer);

    testActor->transform->Translate(0, -4, -5);

    // program loop
    float lastFrameTime = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        // calculate delta time
		float currentFrameTime = glfwGetTime();
		sceneManager.deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;
		
        // input
        processInput(window);

		// clear the screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// update the scene (this will update all components)
        sceneManager.Update();

        testActor->transform->TranslateDelta(0, 0, 1);


        // call events and swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// process all input
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}