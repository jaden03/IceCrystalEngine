#pragma once

class Engine
{
public:
    Engine();
    ~Engine();

    void Run();

private:
    void Init();
    void StartFrame();
    void Update();
    void FixedUpdate(float deltaTime);
    void EndFrame();

    float lastFrameTime = 0.0f;

    float physicsAccumulator = 0.0f;
    const float fixedDeltaTime = 1.0f / 60.0f; // 60 hz physics
};
