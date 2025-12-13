#include <Ice/Core/WindowManager.h>

#include <iostream>

#include "Ice/Utils/stb_image.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

WindowManager& WindowManager::GetInstance()
{
    static WindowManager instance;
    return instance;
}

WindowManager::WindowManager() : window(nullptr)
{
    std::cout << "[WindowManager] Constructor called" << std::endl;
    InitializeWindow();
    std::cout << "[WindowManager] Constructor complete, window = " << (void*)window << std::endl;
}


void WindowManager::InitializeWindow()
{
    std::cout << "[WindowManager] InitializeWindow called" << std::endl;
    
    if (window != nullptr)
    {
        std::cout << "[WindowManager] Window already initialized, skipping" << std::endl;
        return;
    }
    
    std::cout << "[WindowManager] Calling glfwInit()" << std::endl;
    if (!glfwInit())
    {
        std::cout << "[WindowManager] ERROR: glfwInit() failed!" << std::endl;
        return;
    }
    std::cout << "[WindowManager] glfwInit() succeeded" << std::endl;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Store windowed mode dimensions
    if (!isFullscreen)
    {
        windowedWidth = windowWidth;
        windowedHeight = windowHeight;
    }
    
    // Borderless windowed for "fullscreen"
    if (isFullscreen)
    {
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        windowWidth = mode->width;
        windowHeight = mode->height;
    }

    std::cout << "[WindowManager] Creating window with dimensions: " << windowWidth << "x" << windowHeight << std::endl;
    GLFWwindow* win = glfwCreateWindow(windowWidth, windowHeight, "IceCrystal Engine", NULL, NULL);
    if (win == NULL)
    {
        std::cout << "[WindowManager] ERROR: Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    std::cout << "[WindowManager] Window created successfully: " << (void*)win << std::endl;

    // Position at top-left if borderless fullscreen
    if (isFullscreen)
    {
        glfwSetWindowPos(win, 0, 0);
    }

    glfwMakeContextCurrent(win);
    
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(-1);
    }

    window = win;
    std::cout << "[WindowManager] Window pointer assigned: " << (void*)window << std::endl;
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    std::cout << "[WindowManager] InitializeWindow complete" << std::endl;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);

    // Set the window icon
    GLFWimage images[1];
    images[0].pixels = stbi_load("Assets/Logo.png", &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);
}

void WindowManager::ToggleFullscreen()
{
    std::cout << "[WindowManager] ToggleFullscreen called, window = " << (void*)window << std::endl;
    
    if (window == nullptr)
    {
        std::cout << "[WindowManager] ERROR: Cannot toggle fullscreen - window is null!" << std::endl;
        return;
    }
    
    isFullscreen = !isFullscreen;
    
    if (isFullscreen)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
        glfwSetWindowPos(window, 0, 0);
        glfwSetWindowSize(window, mode->width, mode->height - 1);
    
        windowWidth = mode->width;
        windowHeight = mode->height - 1;
    }
    else
    {
        // Restore decorations and windowed size/position
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
        glfwSetWindowSize(window, windowedWidth, windowedHeight);
        glfwSetWindowPos(window, windowedX, windowedY);
        
        windowWidth = windowedWidth;
        windowHeight = windowedHeight;
        
        std::cout << "[WindowManager] Switched to windowed mode: " << windowWidth << "x" << windowHeight << std::endl;
    }
}

void WindowManager::SetFullscreen(bool fullscreen)
{
    std::cout << "[WindowManager] SetFullscreen(" << fullscreen << ") called, current isFullscreen = " << isFullscreen << ", window = " << (void*)window << std::endl;
    
    if (window == nullptr)
    {
        std::cout << "[WindowManager] ERROR: Cannot set fullscreen - window is null!" << std::endl;
        return;
    }
    
    if (isFullscreen != fullscreen)
    {
        std::cout << "[WindowManager] Fullscreen state change requested, calling ToggleFullscreen()" << std::endl;
        ToggleFullscreen();
        std::cout << "[WindowManager] ToggleFullscreen() returned" << std::endl;
    }
    else
    {
        std::cout << "[WindowManager] Already in requested fullscreen state, no action needed" << std::endl;
    }
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    WindowManager& windowManager = WindowManager::GetInstance();

    windowManager.windowWidth = width;
    windowManager.windowHeight = height;
    glViewport(0, 0, width, height);
}