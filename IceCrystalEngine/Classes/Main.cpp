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

	// Initialize FileUtils
    FileUtil::InitializeStaticMembers();

    Actor* amogusActor = new Actor();
	Material* material = new Material(FileUtil::AssetDir + "Materials/amogus.mat");
	Renderer* renderer = new Renderer(amogusActor, FileUtil::AssetDir + "Models/amogus.obj", material);
	
	
    unsigned int VBO, UVBO, EBO;
    glGenBuffers(1, &VBO);
	glGenBuffers(1, &UVBO);
	glGenBuffers(1, &EBO);
    glEnable(GL_DEPTH_TEST);

	
    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);

        // render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // Create transformations
        glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
		
        float time = glfwGetTime() * 25;
		float rotationalTime = std::fmod(time, 360);

        model = glm::rotate(model, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotationalTime), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -1.5f));
        projection = glm::perspective(glm::radians(90.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		renderer->Render(view, projection, model, VBO, UVBO, EBO);

        // call evnets and swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
	
 
    glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &UVBO);
	
	
    //shader->Dispose();
    //glDeleteProgram(shaderProgram);

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