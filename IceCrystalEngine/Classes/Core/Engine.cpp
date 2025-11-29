#include <Ice/Core/Engine.h>

#include "Ice/Core/Input.h"
#include "Ice/Core/LuaManager.h"
#include "Ice/Core/SceneInitializer.h"
#include "Ice/Utils/FileUtil.h"


#ifdef _DEBUG
#include <Ice/Utils/DebugUtil.h>
#endif

Engine::Engine()
{
    Init();
}

Engine::~Engine()
{
#ifdef _DEBUG
    DebugUtil::GetInstance().Cleanup();
#endif
    glfwTerminate();
}

void Engine::Init()
{
    // Setup the static members within FileUtil
    FileUtil::InitializeStaticMembers();

    // Initialize core singletons
    WindowManager::GetInstance();
    SceneManager::GetInstance();
    Input::GetInstance();
    LuaManager::GetInstance();
    LightingManager::GetInstance().InitializeLighting();
    SceneInitializer::GetInstance();

#ifdef _DEBUG
    DebugUtil::GetInstance();
#endif
}

void Engine::Run()
{
    WindowManager& windowManager = WindowManager::GetInstance();
    SceneManager& sceneManager = SceneManager::GetInstance();
    Input& input = Input::GetInstance();

    while (!glfwWindowShouldClose(windowManager.window))
    {
        StartFrame();

        // Update
#ifdef _DEBUG
        DebugUtil& debugUtil = DebugUtil::GetInstance();
        if (input.GetKeyDown(GLFW_KEY_GRAVE_ACCENT))
            debugUtil.showConsole = !debugUtil.showConsole;
#endif

        Update();

        EndFrame();
    }
}

void Engine::StartFrame()
{
    float current = glfwGetTime();
    SceneManager::GetInstance().deltaTime = current - lastFrameTime;
    lastFrameTime = current;

#ifdef _DEBUG
    DebugUtil::GetInstance().StartOfFrame();
#endif
}

void Engine::Update()
{
    SceneManager::GetInstance().Update();
}

void Engine::EndFrame()
{
    WindowManager& windowManager = WindowManager::GetInstance();
    Input& input = Input::GetInstance();

#ifdef _DEBUG
    DebugUtil::GetInstance().EndOfFrame();
#endif

    glfwSwapBuffers(windowManager.window);
    input.ClearInput();
    glfwPollEvents();
}