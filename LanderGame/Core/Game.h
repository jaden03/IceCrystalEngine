#pragma once

#include <Ice/Core/IGame.h>

#include "Ice/Components/Physics/RigidBody.h"
#include "Ice/Core/Actor.h"

class Game : public IGame
{
public:
    void OnInit() override;
    void OnUpdate(float deltaTime) override;
    void OnFixedUpdate(float fixedDeltaTime) override;
    void OnShutdown() override;
private:

    void CreateLander();
    Actor* lander;
    RigidBody* landerRB;
};
