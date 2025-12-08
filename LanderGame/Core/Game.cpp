#include "Game.h"

#include <iostream>
#include <ostream>

#include <Ice/Core/Actor.h>
#include <Ice/Utils/FileUtil.h>

#include "Ice/Components/Renderer.h"
#include "Ice/Components/Light.h"

#include <Ice/Components/Freecam.h>

#include "Ice/Components/Physics/BoxCollider.h"
#include "Ice/Components/Physics/MeshCollider.h"
#include "Ice/Components/Physics/RigidBody.h"

#include "Ice/Components/LuaExecutor.h"

#include "CameraController.h"

void CreateSun()
{
    // Sun
    Actor* sun = new Actor("Sun", "sun");
    DirectionalLight* sunLight = sun->AddComponent<DirectionalLight>();
    sunLight->cascadeSplits = std::vector<float> { 15.0f, 30.0f, 50.0f, 75.0f };
    sunLight->strength = .2f;
    
    // Angle the sun with a parent Actor
    Actor* sunTilt = new Actor("Sun Tilt");
    sun->transform->SetParent(sunTilt->transform);
    sunTilt->transform->Rotate(glm::vec3(0.0f, 45.0f, 0.0f));
    //// set the rotation quat to be at an angle and angled down
    sun->transform->TranslateLocal(0, 5, -12.5f);
    sun->transform->RotateLocal(-50, 0, 0);
    sun->transform->scale = glm::vec3(0.2f, 0.2f, 0.2f);
}

void CreateMoon()
{
    // Moon Surface
    Material* moonMaterial = new Material(FileUtil::AssetDir + "Materials/moon.mat");
    Actor* moon = new Actor("Moon", "Moon");
    Renderer* moonRenderer = new Renderer(FileUtil::AssetDir + "Models/moon.obj", moonMaterial);
    moon->AddComponent(moonRenderer);
    moon->transform->scale = glm::vec3(4, 4, 4);
    moon->transform->Translate(0, -15, 0);
    moon->AddComponent<MeshCollider>(moonRenderer->meshHolders[0].vertices, moonRenderer->meshHolders[0].indices, moon->transform->scale);
    RigidBody* rb = moon->AddComponent<RigidBody>(0.0f);
}

void Game::CreateLander()
{
    // Lander
    Material* landerMaterial = new Material(FileUtil::AssetDir + "Materials/lander.mat");
    lander = new Actor("Lander", "Lander");
    Renderer* landerRenderer = new Renderer(FileUtil::AssetDir + "Models/lander.obj", landerMaterial);
    lander->transform->SetPosition(0, 100, 0);
    lander->AddComponent(landerRenderer);
    lander->AddComponent<BoxCollider>(glm::vec3(1.25, 2.5, 1.25));
    landerRB = lander->AddComponent<RigidBody>(1.0f);
    lander->AddComponent<LuaExecutor>(FileUtil::AssetDir + "LuaScripts/LanderController.lua");
    landerRB->GetBody()->GetMotionProperties()->SetAngularDamping(0.0f);
    landerRB->GetBody()->GetMotionProperties()->SetLinearDamping(0.0f);
    
    // Engine Light
    Actor* engineLight = new Actor("Engine Light", "engineLight");
    PointLight* enginePointLight = engineLight->AddComponent<PointLight>();
    enginePointLight->transform->position = lander->transform->position - glm::vec3(0, 1.5, 0);
    enginePointLight->color = glm::vec3(242.0 / 255.0, 128.0 / 255.0, 56.0 / 255.0);
    enginePointLight->enabled = false;
    enginePointLight->transform->SetParent(lander->transform);
}

void CreateCamera()
{
    // Camera
    Actor* cameraActor = new Actor("Main Camera");
    Camera* cam = cameraActor->AddComponent<Camera>();
    cam->farClippingPlane = 200.0f;
    // cameraActor->AddComponent<LuaExecutor>(FileUtil::AssetDir + "LuaScripts/CameraRotation.lua");
    cameraActor->AddComponent<CameraController>();
}



void Game::OnInit()
{
    std::cout << "Initializing Game..." << std::endl;

    // Set the gravity to zero, we will apply forces towards the center of world
    // PhysicsManager::GetInstance().GetSystem().SetGravity(JPH::Vec3(0.0f, -1.62f, 0.0f));
    PhysicsManager::GetInstance().GetSystem().SetGravity(JPH::Vec3(0.0f, 0.0f, 0.0f));
    
    // Create the horizontal and vertical input axis'
    Input::CreateAxis("horizontal", GLFW_KEY_D, GLFW_KEY_A);
    Input::CreateAxis("vertical", GLFW_KEY_W, GLFW_KEY_S);

    // Lock and hide cursor
    Input::lockCursor = true;
    Input::hideCursor = true;
    
    CreateSun();
    CreateMoon();
    CreateLander();
    CreateCamera();

    // Windowed borderless
    WindowManager::GetInstance().SetFullscreen(true);
}

void Game::OnUpdate(float deltaTime)
{
    if (Input::GetKeyDown(GLFW_KEY_TAB))
    {
        // Lock and hide cursor
        Input::lockCursor = !Input::lockCursor;
        Input::hideCursor = !Input::hideCursor;
    }
}

void Game::OnFixedUpdate(float fixedDeltaTime)
{
    // Apply gravity towards 0, 0, 0
    landerRB->AddForce(-lander->transform->position * 1.62f * 100.0f);
}


void Game::OnShutdown()
{
}