#include <Ice/Core/Engine.h>

#include <Ice/Core/Input.h>
#include <Ice/Core/LuaManager.h>
#include <Ice/Core/RendererManager.h>
#include <Ice/Core/SceneInitializer.h>
#include <Ice/Utils/FileUtil.h>
#include <Ice/Core/PhysicsManager.h>
#include <Ice/Core/SceneManager.h>

#include "Ice/Core/IGame.h"


#ifdef _DEBUG
#include <Ice/Utils/DebugUtil.h>
#include <Ice/Editor/WebEditorManager.h>
#endif


Engine::Engine()
{
    Init();
}

Engine::~Engine()
{
    LuaManager::GetInstance().Cleanup();
#ifdef _DEBUG
    DebugUtil::GetInstance().Cleanup();
    WebEditorManager::GetInstance().Stop();
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
    
    JPH::RegisterDefaultAllocator();
    PhysicsManager::GetInstance();
    
    Input::GetInstance();
    LuaManager::GetInstance();
    LightingManager::GetInstance().InitializeLighting();
    RendererManager::GetInstance();
    
#ifdef _DEBUG
    DebugUtil::GetInstance();
    WebEditorManager::GetInstance().Start(8080);
#endif
}

// Standalone mode (no game)
void Engine::Run()
{
    // Initialize the SceneInitializer so it initializes the scene
    SceneInitializer::GetInstance();
    
    WindowManager& windowManager = WindowManager::GetInstance();
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

// Game mode
void Engine::Run(IGame* gameInstance)
{
    if (!gameInstance)
    {
        // Fallback to the standalone mode
        Run();
        return;
    }

    isEditor = false;
    game = gameInstance;

    game->OnInit();

    WindowManager& windowManager = WindowManager::GetInstance();

    while (!glfwWindowShouldClose(windowManager.window))
    {
        StartFrame();
        Update();
        EndFrame();
    }

    game->OnShutdown();
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
    SceneManager& sceneManager = SceneManager::GetInstance();
#ifdef _DEBUG
    WebEditorManager& webEditor = WebEditorManager::GetInstance();
    
    // Skip physics and Lua updates if engine is paused from web editor
    bool isPaused = webEditor.IsEnginePaused();
#else
    bool isPaused = false;
#endif
    
    if (!isPaused)
    {
        // Step physics with fixed timestep
        FixedUpdate(sceneManager.deltaTime);

        // Call game update if there is a game
        if (game && !isEditor)
        {
            game->OnUpdate(sceneManager.deltaTime);
        }
    }

#ifdef _DEBUG
    // Update web editor (process queued messages)
    webEditor.Update();
#endif
    
    // Other engine updates (rendering still happens even when paused)
    sceneManager.Update();
}

void Engine::FixedUpdate(float deltaTime)
{
#ifdef _DEBUG
    // Don't update physics if paused
    if (WebEditorManager::GetInstance().IsEnginePaused())
        return;
#endif
    
    // Accumulate time
    physicsAccumulator += deltaTime;
    
    const float maxAcc = fixedDeltaTime * 5.0f;   // 5 physics steps max per frame
    if (physicsAccumulator > maxAcc)
        physicsAccumulator = maxAcc;

    // Step physics in fixed increments
    while (physicsAccumulator >= fixedDeltaTime)
    {
        PhysicsManager::GetInstance().Step(fixedDeltaTime);

        // Call game fixedupdate if in game mode
        if (game && !isEditor)
        {
            game->OnFixedUpdate(fixedDeltaTime);
        }
        
        physicsAccumulator -= fixedDeltaTime;
    }
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