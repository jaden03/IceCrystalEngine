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

#include <Ice/Utils/MathUtils.h>

#include "Ice/Components/RawImage.h"

void CreateSun()
{
    // Sun
    Actor* sun = new Actor("Sun", "sun");
    Renderer* sunRenderer = new Renderer(FileUtil::AssetDir + "Models/cone.obj");
    DirectionalLight* sunLight = sun->AddComponent<DirectionalLight>();
    sunLight->cascadeSplits = std::vector<float> { 15.0f, 30.0f, 50.0f, 75.0f };
    sunLight->strength = .2f;
    
    // Angle the sun with a parent Actor
    Actor* sunTilt = new Actor("Sun Tilt");
    sun->transform->SetParent(sunTilt->transform);
    sunTilt->transform->Rotate(glm::vec3(0.0f, 45.0f, 0.0f));
    //// set the rotation quat to be at an angle and angled down
    sunTilt->transform->Translate(0, 125, 0);
    sun->transform->RotateLocal(-50, 0, 0);
    sun->transform->scale = glm::vec3(0.2f, 0.2f, 0.2f);
}

void CreateMoon()
{
    // Moon Surface
    Material* moonMaterial = new Material(FileUtil::AssetDir + "Materials/moon.mat");
    Actor* moon = new Actor("Moon", "moon");
    Renderer* moonRenderer = new Renderer(FileUtil::AssetDir + "Models/moon.obj", moonMaterial);
    moon->AddComponent(moonRenderer);
    moon->transform->scale = glm::vec3(4, 4, 4);
    moon->AddComponent<MeshCollider>(moonRenderer->meshHolders[0].vertices, moonRenderer->meshHolders[0].indices, moon->transform->scale);
    RigidBody* rb = moon->AddComponent<RigidBody>(0.0f);
}

void Game::CreateLander()
{
    // Lander
    lander = new Actor("Lander", "Lander");
    Renderer* landerRenderer = new Renderer(FileUtil::AssetDir + "Models/lander.obj", mainMaterial);
    lander->transform->SetPosition(0, 100, 0);
    lander->AddComponent(landerRenderer);
    lander->AddComponent<BoxCollider>(glm::vec3(1.25, 2.45, 1.25));
    landerRB = lander->AddComponent<RigidBody>(5000.0f);
    lander->AddComponent<LuaExecutor>(FileUtil::AssetDir + "LuaScripts/LanderController.lua");
    landerRB->GetBody()->GetMotionProperties()->SetAngularDamping(0.0f);
    landerRB->GetBody()->GetMotionProperties()->SetLinearDamping(0.0f);
    landerRB->GetBody()->SetFriction(10.0f);
    
    // // Set up trigger callbacks
    // landerRB->OnTriggerEntered = [](RigidBody* other) {
    //     if (other->owner->tag == "refuelTrigger")
    //     {
    //         Game::inRefuelZone = true;
    //     }
    // };
    //
    // landerRB->OnTriggerExited = [](RigidBody* other) {
    //     if (other->owner->tag == "refuelTrigger")
    //     {
    //         Game::inRefuelZone = false;
    //     }
    // };
    
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



void Game::CreatePad(glm::vec3 position, glm::quat rotation)
{
    // Pad
    Actor* pad = new Actor("Pad", "pad");
    Renderer* padRenderer = pad->AddComponent<Renderer>(FileUtil::AssetDir + "Models/pad.obj", mainMaterial);
    pad->transform->SetPosition(position);
    pad->transform->SetRotation(rotation);
    pad->AddComponent<MeshCollider>(padRenderer->meshHolders[0].vertices, padRenderer->meshHolders[0].indices, pad->transform->scale);
    pad->AddComponent<RigidBody>(0.0f);
    // Lights
    Actor* padLights = new Actor("Pad Lights");
    padLights->transform->SetPosition(position);
    padLights->transform->SetRotation(rotation);
    padLights->transform->SetParent(pad->transform);
    padLights->AddComponent<Renderer>(FileUtil::AssetDir + "Models/padLights.obj", unlitMaterial);
    // Refuel Trigger
    Actor* padRefuelTrigger = new Actor("Pad Refuel Trigger", "refuelTrigger");
    padRefuelTrigger->transform->SetRotation(rotation);
    padRefuelTrigger->transform->SetScale(4.0f, 0.2f, 4.0f);
    padRefuelTrigger->transform->SetParent(pad->transform);
    // padRefuelTrigger->transform->SetLocalPosition(0, 2.5f, 0); // cant do this because the actual position would be calculated and set on the next frame
    // but the physics need the position to already be set when initializing the rigidbody, so we have to calculate out the actual world pos and set it
    padRefuelTrigger->transform->position = pad->transform->position + (padRefuelTrigger->transform->up * 0.4f);
    padRefuelTrigger->transform->localPosition = glm::vec3(0.0f, 0.4f, 0.0f);
    padRefuelTrigger->AddComponent<BoxCollider>(padRefuelTrigger->transform->scale);
    padRefuelTrigger->AddComponent<RigidBody>(0.0f, true);
    // padRefuelTrigger->AddComponent<Renderer>(FileUtil::AssetDir + "Models/cube.obj", unlitMaterial);
}
void Game::CreateWorld()
{
    CreatePad(MathUtils::BlenderToEngine(glm::vec3(44.58f, -34.67f, 68.55f)), MathUtils::BlenderToEngineQuat(glm::quat(0.959001f, 0.149376f, 0.240800f, 0.004277f)));
    CreatePad(MathUtils::BlenderToEngine(glm::vec3(-85.22f, -13.97f, 52.32f)), MathUtils::BlenderToEngineQuat(glm::quat(0.860231f, -0.107791f, -0.419877f, 0.268492f)));
    CreatePad(MathUtils::BlenderToEngine(glm::vec3(3.16f, 88.14f, 20.63f)), MathUtils::BlenderToEngineQuat(glm::quat(0.744106f, -0.631242f, -0.130073f, -0.175845f)));
    CreatePad(MathUtils::BlenderToEngine(glm::vec3(-9.45f, 35.79f, 88.66f)), MathUtils::BlenderToEngineQuat(glm::quat(0.888069f, -0.226797f, 0.129038f, 0.378479f)));
    CreatePad(MathUtils::BlenderToEngine(glm::vec3(94.57f, 7.68f, 0.78f)), MathUtils::BlenderToEngineQuat(glm::quat(0.643592f, 0.329059f, 0.634373f, 0.274008f)));
    CreatePad(MathUtils::BlenderToEngine(glm::vec3(53.58f, -51.03f, -59.88f)), MathUtils::BlenderToEngineQuat(glm::quat(0.442553f, 0.665599f, 0.600259f, 0.028527f)));
    CreatePad(MathUtils::BlenderToEngine(glm::vec3(-45.17f, -22.14f, -82.00f)), MathUtils::BlenderToEngineQuat(glm::quat(-0.047015f, 0.697573f, 0.667251f, -0.256822f)));
    CreatePad(MathUtils::BlenderToEngine(glm::vec3(44.71f, 35.77f, -76.99f)), MathUtils::BlenderToEngineQuat(glm::quat(-0.052434f, 0.653777f, 0.696809f, 0.290316f)));
}


void CreateUI()
{
    Actor* fuelBarBG = new Actor("Fuel Bar Background");
    fuelBarBG->AddComponent<RawImage>(FileUtil::AssetDir + "Textures/fuelBarBG.png");
    fuelBarBG->transform->SetScale(glm::vec3(400, 50, 100));
    fuelBarBG->transform->Translate(400, 80, 0);
    
    Actor* fuelBarFG = new Actor("Fuel Bar Background", "fuelBarFG");
    fuelBarFG->AddComponent<RawImage>(FileUtil::AssetDir + "Textures/fuelBarFG.png");
    fuelBarFG->transform->SetScale(glm::vec3(400, 50, 100));
    fuelBarFG->transform->Translate(400, 80, 0);
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

    // Create the main material used for everything
    mainMaterial = new Material(FileUtil::AssetDir + "Materials/main.mat");
    unlitMaterial = new Material(FileUtil::AssetDir + "Materials/unlit.mat");
    unlitMaterial->color = glm::vec3(0.0f, 1.0f, 3.0f);

    CreateSun();
    CreateMoon();
    CreateLander();
    CreateCamera();
    CreateWorld(); // pads, buildings, etc
    CreateUI();

    // Windowed borderless
    WindowManager::GetInstance().SetFullscreen(true);
}

static double lastPrint = 0.0;
void Game::OnUpdate(float deltaTime)
{
    if (Input::GetKeyDown(GLFW_KEY_TAB))
    {
        // Lock and hide cursor
        Input::lockCursor = !Input::lockCursor;
        Input::hideCursor = !Input::hideCursor;
    }

    double now = glfwGetTime();
    if (now - lastPrint >= 1.0)
    {
        std::cout << "Speed: " << glm::length(landerRB->GetLinearVelocity()) << std::endl;
        lastPrint = now;
    }
}

void Game::OnFixedUpdate(float fixedDeltaTime)
{
    // Apply gravity towards 0, 0, 0
    // landerRB->AddForce(-lander->transform->position * 1.62f * 100.0f);
    landerRB->AddForce(-lander->transform->position * 1.62f * 100.0f);
}


void Game::OnShutdown()
{
}