#include <Ice/Core/WindowManager.h>

#include <iostream>

#include "Ice/Utils/stb_image.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);

WindowManager& WindowManager::GetInstance()
{
    static WindowManager instance; // Static local variable ensures a single instance
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
    
    // This essentially puts it into borderless-windowed
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

    glfwMakeContextCurrent(win);
    
    // Turns vSync off (set back to 1 to turn it on)
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

    // Set the window icon to Assets/Logo.png
    GLFWimage images[1];
    images[0].pixels = stbi_load("Assets/Logo.png", &images[0].width, &images[0].height, 0, 4); // load rgba channels
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);
}

void WindowManager::ToggleFullscreen()
{
    std::cout << "[WindowManager] ToggleFullscreen called, window = " << (void*)window << std::endl;
    
    // Safety check: ensure window exists before toggling
    if (window == nullptr)
    {
        std::cout << "[WindowManager] ERROR: Cannot toggle fullscreen - window is null!" << std::endl;
        return;
    }
    
    isFullscreen = !isFullscreen;
    
    if (isFullscreen)
    {
        // Save current windowed dimensions and position
        glfwGetWindowPos(window, &windowedX, &windowedY);
        glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
        
        // Get primary monitor and its video mode
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        
        // Switch to fullscreen using glfwSetWindowMonitor (proper GLFW method)
        // This creates a borderless fullscreen window
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        
        windowWidth = mode->width;
        windowHeight = mode->height;
        
        std::cout << "[WindowManager] Switched to fullscreen: " << windowWidth << "x" << windowHeight << std::endl;
    }
    else
    {
        // Switch back to windowed mode using glfwSetWindowMonitor
        // Passing NULL as monitor parameter creates a windowed mode window
        glfwSetWindowMonitor(window, NULL, windowedX, windowedY, windowedWidth, windowedHeight, GLFW_DONT_CARE);
        
        windowWidth = windowedWidth;
        windowHeight = windowedHeight;
        
        std::cout << "[WindowManager] Switched to windowed mode: " << windowWidth << "x" << windowHeight << std::endl;
    }
}

void WindowManager::SetFullscreen(bool fullscreen)
{
    std::cout << "[WindowManager] SetFullscreen(" << fullscreen << ") called, current isFullscreen = " << isFullscreen << ", window = " << (void*)window << std::endl;
    
    // Safety check: ensure window exists
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


// glfw: whenever the window size changed
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    WindowManager& windowManager = WindowManager::GetInstance();

    windowManager.windowWidth = width;
    windowManager.windowHeight = height;
    glViewport(0, 0, width, height);
}