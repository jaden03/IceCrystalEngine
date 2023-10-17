#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <Ice/Core/WindowManager.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

WindowManager::WindowManager()
{
    InitializeWindow();
}


void WindowManager::InitializeWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* win = glfwCreateWindow(windowWidth, windowHeight, "IceCrystal Engine", NULL, NULL);
    if (win == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(win);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
		exit(-1);
    }

	window = win;
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
}



// glfw: whenever the window size changed
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	WindowManager::GetInstance().windowWidth = width;
	WindowManager::GetInstance().windowHeight = height;
    glViewport(0, 0, width, height);
}