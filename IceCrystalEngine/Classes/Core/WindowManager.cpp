#include <Ice/Core/WindowManager.h>

#include <iostream>

#include "Ice/Utils/stb_image.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);

WindowManager::WindowManager()
{
    InitializeWindow();
}


void WindowManager::InitializeWindow()
{
    if (window != nullptr) return; // Already initialized
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Store windowed mode dimensions
    if (!isFullscreen)
    {
        windowedWidth = windowWidth;
        windowedHeight = windowHeight;
    }
    
    // This essentially puts it into borderless-windowed
    if (isFullscreen)
    {
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        windowWidth = mode->width;
        windowHeight = mode->height;
    }


    GLFWwindow* win = glfwCreateWindow(windowWidth, windowHeight, "IceCrystal Engine", NULL, NULL);
    if (win == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(win);
    
    // Turns vSync off (set back to 1 to turn it on)
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
       exit(-1);
    }

    window = win;
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);

    // Set the window icon to Assets/Logo.png
    GLFWimage images[1];
    images[0].pixels = stbi_load("Assets/Logo.png", &images[0].width, &images[0].height, 0, 4); // load rgba channels
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);
}

void WindowManager::ToggleFullscreen()
{
    isFullscreen = !isFullscreen;
    
    if (isFullscreen)
    {
        // Save current windowed dimensions and position
        glfwGetWindowPos(window, &windowedX, &windowedY);
        glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
        
        // Get monitor dimensions
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        
        // glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        
        // Just maximize the window
        glfwMaximizeWindow(window);
        
        windowWidth = mode->width;
        windowHeight = mode->height;
    }
    else
    {
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
        
        // Restore the window
        glfwRestoreWindow(window);
        
        // Restore original position and size
        glfwSetWindowPos(window, windowedX, windowedY);
        glfwSetWindowSize(window, windowedWidth, windowedHeight);
        
        windowWidth = windowedWidth;
        windowHeight = windowedHeight;
    }
}

void WindowManager::SetFullscreen(bool fullscreen)
{
    if (isFullscreen != fullscreen)
    {
        ToggleFullscreen();
    }
}


// glfw: whenever the window size changed
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    WindowManager& windowManager = WindowManager::GetInstance();

    windowManager.windowWidth = width;
    windowManager.windowHeight = height;
    glViewport(0, 0, width, height);
}