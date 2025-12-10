#include <Ice/Core/Engine.h>

#include <Ice/Core/Input.h>
#include <Ice/Core/LuaManager.h>
#include <Ice/Core/RendererManager.h>
#include <Ice/Core/SceneInitializer.h>
#include <Ice/Utils/FileUtil.h>
#include <Ice/Core/PhysicsManager.h>
#include <Ice/Editor/WebEditorManager.h>
#include <Ice/Editor/GizmoRenderer.h>
#include <Ice/Editor/EditorUI.h>
#include <Ice/Core/SceneManager.h>

#include "Ice/Core/IGame.h"


#ifdef _DEBUG
#include <Ice/Utils/DebugUtil.h>
#endif


Engine::Engine()
{
    Init();
}

Engine::~Engine()
{
    // Stop web editor if running
    WebEditorManager::GetInstance().Stop();
    
    LuaManager::GetInstance().Cleanup();
    EditorUI::GetInstance().Cleanup();
#ifdef _DEBUG
    // DebugUtil::GetInstance().Cleanup(); // Disabled - see Init()
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
    
    // Initialize gizmo renderer for web editor
    GizmoRenderer::GetInstance().Initialize();
    
    // Initialize the editor UI (this also initializes ImGui)
    EditorUI::GetInstance().Initialize();
    
#ifdef _DEBUG
    // NOTE: DebugUtil is disabled because EditorUI now handles ImGui initialization
    // If you need DebugUtil, modify it to not initialize ImGui context
    // DebugUtil::GetInstance();
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
        // DebugUtil disabled - EditorUI handles console now
        // DebugUtil& debugUtil = DebugUtil::GetInstance();
        // if (input.GetKeyDown(GLFW_KEY_GRAVE_ACCENT))
        //     debugUtil.showConsole = !debugUtil.showConsole;
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

    isEditor = true;
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

    EditorUI::GetInstance().BeginFrame();
    // Note: DebugUtil is disabled in favor of EditorUI
}

void Engine::Update()
{
    SceneManager& sceneManager = SceneManager::GetInstance();
    WebEditorManager& webEditor = WebEditorManager::GetInstance();
    
    // Skip physics and Lua updates if engine is paused from web editor
    bool isPaused = webEditor.IsEnginePaused();
    
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

    // Update web editor (process queued messages)
    webEditor.Update();

    // Other engine updates (rendering still happens even when paused)
    sceneManager.Update();
}

void Engine::FixedUpdate(float deltaTime)
{
    // Don't update physics if paused
    bool isEnginePaused = EditorUI::GetInstance().IsEnginePaused() || WebEditorManager::GetInstance().IsEnginePaused();
    if (isEnginePaused)
        return;
    
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

    EditorUI::GetInstance().RenderEditor();
    EditorUI::GetInstance().EndFrame();
    // Note: DebugUtil is disabled in favor of EditorUI

    glfwSwapBuffers(windowManager.window);
    input.ClearInput();
    glfwPollEvents();
}