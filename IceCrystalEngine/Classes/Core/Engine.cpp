#include <Ice/Core/Engine.h>

#include <Ice/Core/Input.h>
#include <Ice/Core/LuaManager.h>
#include <Ice/Core/RendererManager.h>
#include <Ice/Core/SceneInitializer.h>
#include <Ice/Utils/FileUtil.h>
#include <Ice/Core/PhysicsManager.h>
#include <Ice/Editor/WebEditorManager.h>
#include <Ice/Core/SceneManager.h>
#include <Ice/Editor/GizmoRenderer.h>


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
    
    JPH::RegisterDefaultAllocator();
    PhysicsManager::GetInstance();
    
    Input::GetInstance();
    LuaManager::GetInstance();
    LightingManager::GetInstance().InitializeLighting();
    SceneInitializer::GetInstance();
    RendererManager::GetInstance();
    
#ifdef _DEBUG
    DebugUtil::GetInstance();
    GizmoRenderer::GetInstance().Initialize();
#endif

    // Initialize Web Editor (optional - can be started via console command later)
#ifdef _DEBUG
    WebEditorManager::GetInstance().Start(8080);
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
    SceneManager& sceneManager = SceneManager::GetInstance();
    WebEditorManager& webEditor = WebEditorManager::GetInstance();
    
    // Skip physics and Lua updates if engine is paused from web editor
    bool isPaused = webEditor.IsEnginePaused();
    
    if (!isPaused)
    {
        // Step physics with fixed timestep
        FixedUpdate(sceneManager.deltaTime);
    }

    // Update web editor (process queued messages)
    webEditor.Update();

    // Other engine updates (rendering still happens even when paused)
    sceneManager.Update();
}

void Engine::FixedUpdate(float deltaTime)
{
    // Don't update physics if paused
    if (WebEditorManager::GetInstance().IsEnginePaused())
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