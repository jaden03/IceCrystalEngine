#pragma once

#include <Ice/Core/IGame.h>

#include "Ice/Components/Physics/RigidBody.h"
#include "Ice/Components/LineRenderer.h"
#include "Ice/Core/Actor.h"
#include "Ice/Rendering/Material.h"

class Game : public IGame
{
public:
    void OnInit() override;
    void OnUpdate(float deltaTime) override;
    void OnFixedUpdate(float fixedDeltaTime) override;
    void OnShutdown() override;
private:

    void CreateLander();
    void CreateWorld();
    void CreateObject(std::string type, glm::vec3 position, glm::quat rotation);
    
    Actor* lander;
    RigidBody* landerRB;
    LineRenderer* landerLineRenderer;
    
    Material* mainMaterial;
    Material* unlitMaterial;
};
