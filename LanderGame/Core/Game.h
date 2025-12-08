#pragma once

#include <Ice/Core/IGame.h>

class Game : public IGame
{
public:
    void OnInit() override;
    void OnUpdate(float deltaTime) override;
    void OnFixedUpdate(float fixedDeltaTime) override;
    void OnShutdown() override;
private:
    
};
