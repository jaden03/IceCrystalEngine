#pragma once

class IGame
{
public:
    virtual ~IGame() = default;

    // Called once then the game starts
    virtual void OnInit() = 0;

    // Called every frame
    virtual void OnUpdate(float deltaTime) = 0;

    // Called at fixed timestep
    virtual void OnFixedUpdate(float fixedDeltaTime) = 0;

    // Called once when game shuts down
    virtual void OnShutdown() = 0;
};
