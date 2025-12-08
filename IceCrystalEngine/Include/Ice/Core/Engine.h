#pragma once

class IGame;

class Engine
{
public:
    static Engine& GetInstance()
    {
        static Engine instance;
        return instance;
    }

    // Run Stantalone (this will run the SceneInitializer)
    void Run();
    // Run with a game (SceneInitializer will not run, the game is expected to do setup on init)
    void Run(IGame* game);

private:
    Engine() {Init();}
    ~Engine();
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    
    void Init();
    void StartFrame();
    void Update();
    void FixedUpdate(float deltaTime);
    void EndFrame();

    IGame* game = nullptr;
    float lastFrameTime = 0.0f;
    float physicsAccumulator = 0.0f;
    const float fixedDeltaTime = 1.0f / 60.0f; // 60 hz physics
    // Will be false if hooked onto from a game
    bool isEditor = true;
};
