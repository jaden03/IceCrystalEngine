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
    void EndFrame();

    float lastFrameTime = 0.0f;
};
