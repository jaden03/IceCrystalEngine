#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <Ice/Rendering/Material.h>
#include <Ice/Rendering/Shader.h>
#include <Ice/Rendering/Renderer.h>
#include <Ice/Utils/FileUtil.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Ice/Core/SceneManager.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "IceCrystal Engine", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
	
	// Initialize FileUtils
    FileUtil::InitializeStaticMembers();

	// Get a reference to the SceneManager
	SceneManager &sceneManager = SceneManager::GetInstance();

    Actor* amogusActor = new Actor();
	Material* material = new Material(FileUtil::AssetDir + "Materials/amogus.mat");
	Renderer* renderer = new Renderer(FileUtil::AssetDir + "Models/amogus.obj", material);
	amogusActor->AddComponent(renderer);
	
    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);

        // render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update the scene
        sceneManager.Update();

        // call evnets and swap buffers
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

// glfw: whenever the window size changed
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    SCR_WIDTH = width;
	SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}