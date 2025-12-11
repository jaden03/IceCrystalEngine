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

#include "Ice/Components/LineRenderer.h"
#include "Ice/Components/RawImage.h"

#include <Ice/Core/SceneManager.h>

#ifdef _DEBUG
#include <Ice/Editor/EditorUI.h>
#endif

void CreateSun()
{
    std::cout << "[Game] Creating Sun..." << std::endl;
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
    sun->transform->RotateLocal(50, 0, 0);
    sun->transform->scale = glm::vec3(0.2f, 0.2f, 0.2f);
    std::cout << "[Game] Sun created successfully" << std::endl;
}

void CreateMoon()
{
    std::cout << "[Game] Creating Moon..." << std::endl;
    // Moon Surface
    Material* moonMaterial = new Material(FileUtil::AssetDir + "Materials/moon.mat");
    Actor* moon = new Actor("Moon", "moon");
    Renderer* moonRenderer = new Renderer(FileUtil::AssetDir + "Models/moon.obj", moonMaterial);
    moon->AddComponent(moonRenderer);
    moon->transform->scale = glm::vec3(4, 4, 4);
    moon->AddComponent<MeshCollider>(moonRenderer->meshHolders[0].vertices, moonRenderer->meshHolders[0].indices, moon->transform->scale);
    RigidBody* rb = moon->AddComponent<RigidBody>(0.0f);
    std::cout << "[Game] Moon created successfully" << std::endl;
}

void Game::CreateLander()
{
    std::cout << "[Game] Creating Lander..." << std::endl;
    // Lander
    lander = new Actor("Lander", "Lander");
    lander->AddComponent<Renderer>(FileUtil::AssetDir + "Models/lander.obj", mainMaterial);
    lander->transform->SetPosition(0, 100, 0);
    std::cout << "[Game] Lander actor created at position (0, 100, 0) with tag 'Lander'" << std::endl;
    lander->AddComponent<BoxCollider>(glm::vec3(1.25, 2.45, 1.25));
    landerRB = lander->AddComponent<RigidBody>(5000.0f);
    lander->AddComponent<LuaExecutor>(FileUtil::AssetDir + "LuaScripts/LanderController.lua");
    landerRB->GetBody()->GetMotionProperties()->SetAngularDamping(0.0f);
    landerRB->GetBody()->GetMotionProperties()->SetLinearDamping(0.0f);
    landerRB->GetBody()->SetFriction(10.0f);

    landerLineRenderer = lander->AddComponent<LineRenderer>(
        // Points
        std::vector<glm::vec3> {},
        // color
        glm::vec3(1, 0, 0),
        glm::vec3(0, 1, 0)
    );
    landerLineRenderer->width = 5.0f;
    
    // // Set up trigger callbacks
    landerRB->OnTriggerEntered = [](RigidBody* other) {
        if (other->owner->tag == "refuelTrigger")
        {
            std::cout << "Entered Fuel Zone" << std::endl;
        }
    };
    
    landerRB->OnTriggerExited = [](RigidBody* other) {
        if (other->owner->tag == "refuelTrigger")
        {
            std::cout << "Exited Fuel Zone" << std::endl;
        }
    };
    
    // Engine Light
    Actor* engineLight = new Actor("Engine Light", "engineLight");
    PointLight* enginePointLight = engineLight->AddComponent<PointLight>();
    enginePointLight->transform->position = lander->transform->position - glm::vec3(0, 1.5, 0);
    enginePointLight->color = glm::vec3(0, .4f, 1);
    enginePointLight->strength = 0;
    enginePointLight->transform->SetParent(lander->transform);

    // Engine Plume
    Actor* enginePlume = new Actor("Engine Plume", "enginePlume");
    enginePlume->AddComponent<Renderer>(FileUtil::AssetDir + "Models/landerPlume.obj", unlitMaterial);
    enginePlume->transform->SetParent(lander->transform);
    enginePlume->transform->SetScale(1, 0, 1);
    enginePlume->transform->localPosition = glm::vec3(0, -1.0, 0);
    std::cout << "[Game] Lander created successfully" << std::endl;
}

void CreateTestCube(Material* material)
{
    std::cout << "[Game] Creating Test Cube near lander for visibility testing..." << std::endl;
    // Create a test cube near the lander to verify rendering works
    Actor* testCube = new Actor("Test Cube", "testCube");
    testCube->AddComponent<Renderer>(FileUtil::AssetDir + "Models/cube.obj", material);
    testCube->transform->SetPosition(10, 100, 0); // Next to the lander
    testCube->transform->SetScale(5, 5, 5); // Make it big and visible
    std::cout << "[Game] Test Cube created at (10, 100, 0) with scale (5, 5, 5)" << std::endl;
}

void CreateCamera()
{
    std::cout << "[Game] Creating Camera..." << std::endl;
    // Camera
    Actor* cameraActor = new Actor("Main Camera");
    Camera* cam = cameraActor->AddComponent<Camera>();
    cam->farClippingPlane = 200.0f;
    // cameraActor->AddComponent<LuaExecutor>(FileUtil::AssetDir + "LuaScripts/CameraRotation.lua");
    cameraActor->AddComponent<CameraController>();
    std::cout << "[Game] Camera created with CameraController component" << std::endl;
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
    // cant only set local position because the actual position would be calculated and set on the next frame
    // but the physics need the position to already be set when initializing the rigidbody, so we have to calculate out the actual world pos and set it
    padRefuelTrigger->transform->position = pad->transform->position + (pad->transform->Up() * 0.4f); // use the Up() function because up would just be world up (hasnt had the first update yet)
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
    
    Actor* fuelBarFG = new Actor("Fuel Bar Foreground", "fuelBarFG");
    fuelBarFG->AddComponent<RawImage>(FileUtil::AssetDir + "Textures/fuelBarFG.png");
    fuelBarFG->transform->SetScale(glm::vec3(400, 50, 100));
    fuelBarFG->transform->Translate(400, 80, 0);

    Actor* sasToggleBG = new Actor("SAS Toggle Background");
    sasToggleBG->AddComponent<RawImage>(FileUtil::AssetDir + "Textures/sasToggleBG.png");
    sasToggleBG->transform->SetScale(glm::vec3(150, 50, 100));
    sasToggleBG->transform->Translate(400, 150, 0);

    Actor* sasToggleFG = new Actor("SAS Toggle Foreground", "sasToggleFG");
    RawImage* sasToggleFGImage = sasToggleFG->AddComponent<RawImage>(FileUtil::AssetDir + "Textures/sasToggleFG.png");
    sasToggleFG->transform->SetScale(glm::vec3(150, 50, 100));
    sasToggleFG->transform->Translate(400, 150, 0);
    sasToggleFGImage->enabled = false;
}




void Game::OnInit()
{
    std::cout << "========================================" << std::endl;
    std::cout << "[Game] Initializing Game..." << std::endl;
    std::cout << "========================================" << std::endl;

    // Set the gravity to zero, we will apply forces towards the center of world
    // PhysicsManager::GetInstance().GetSystem().SetGravity(JPH::Vec3(0.0f, -1.62f, 0.0f));
    PhysicsManager::GetInstance().GetSystem().SetGravity(JPH::Vec3(0.0f, 0.0f, 0.0f));
    
    // Create the horizontal and vertical input axis'
    Input::CreateAxis("horizontal", GLFW_KEY_D, GLFW_KEY_A);
    Input::CreateAxis("vertical", GLFW_KEY_W, GLFW_KEY_S);
    
    // Create the main material used for everything
    mainMaterial = new Material(FileUtil::AssetDir + "Materials/main.mat");
    unlitMaterial = new Material(FileUtil::AssetDir + "Materials/unlit.mat");
    unlitMaterial->color = glm::vec3(0.0f, 1.0f, 3.0f);

    CreateSun();
    CreateMoon();
    CreateLander();
    CreateCamera();
    CreateTestCube(mainMaterial); // Add test cube for visibility testing
    CreateWorld(); // pads, buildings, etc
    CreateUI();
    
    std::cout << "========================================" << std::endl;
    std::cout << "[Game] Scene initialization complete!" << std::endl;
    std::cout << "[Game] Total actors in scene: " << SceneManager::GetInstance().GetActorCount() << std::endl;
    
    // Verify lander exists
    Actor* landerCheck = SceneManager::GetInstance().GetActorByTag("Lander");
    if (landerCheck)
    {
        std::cout << "[Game] Lander verification: FOUND (position: " 
                  << landerCheck->transform->position.x << ", "
                  << landerCheck->transform->position.y << ", "
                  << landerCheck->transform->position.z << ")" << std::endl;
        
        // Check lander components
        Renderer* landerRenderer = landerCheck->GetComponent<Renderer>();
        if (landerRenderer)
        {
            std::cout << "[Game] Lander has Renderer component with " 
                      << landerRenderer->meshHolders.size() << " meshes" << std::endl;
            if (landerRenderer->material)
            {
                std::cout << "[Game] Lander material is valid" << std::endl;
            }
            else
            {
                std::cout << "[Game] WARNING: Lander material is NULL!" << std::endl;
            }
        }
        else
        {
            std::cout << "[Game] WARNING: Lander has no Renderer component!" << std::endl;
        }
    }
    else
    {
        std::cout << "[Game] Lander verification: NOT FOUND - THIS IS A PROBLEM!" << std::endl;
    }
    
    // Verify camera exists
    Camera* cameraCheck = SceneManager::GetInstance().GetComponentOfType<Camera>();
    if (cameraCheck)
    {
        std::cout << "[Game] Camera verification: FOUND" << std::endl;
        std::cout << "[Game] Camera position: (" 
                  << cameraCheck->transform->position.x << ", "
                  << cameraCheck->transform->position.y << ", "
                  << cameraCheck->transform->position.z << ")" << std::endl;
        std::cout << "[Game] Camera FOV: " << cameraCheck->fieldOfView 
                  << ", Near: " << cameraCheck->nearClippingPlane 
                  << ", Far: " << cameraCheck->farClippingPlane << std::endl;
    }
    else
    {
        std::cout << "[Game] Camera verification: NOT FOUND - THIS IS A PROBLEM!" << std::endl;
    }
    
    // Verify moon exists
    Actor* moonCheck = SceneManager::GetInstance().GetActorByTag("moon");
    if (moonCheck)
    {
        std::cout << "[Game] Moon verification: FOUND" << std::endl;
        Renderer* moonRenderer = moonCheck->GetComponent<Renderer>();
        if (moonRenderer && !moonRenderer->meshHolders.empty())
        {
            std::cout << "[Game] Moon has Renderer with " << moonRenderer->meshHolders.size() << " meshes" << std::endl;
        }
    }
    else
    {
        std::cout << "[Game] WARNING: Moon not found!" << std::endl;
    }
    
    std::cout << "========================================" << std::endl;
    std::cout << "[Game] CONTROLS:" << std::endl;
    std::cout << "[Game] - Hold RIGHT MOUSE BUTTON and move mouse to rotate camera" << std::endl;
    std::cout << "[Game] - Use SCROLL WHEEL to zoom in/out" << std::endl;
    std::cout << "[Game] - Press Ctrl+P to toggle Edit/Play mode (Debug builds)" << std::endl;
    std::cout << "========================================" << std::endl;

#ifdef _DEBUG
    // Automatically start in Play Mode for immediate gameplay
    EditorUI::GetInstance().SetPlayMode(PlayMode::PLAY);
    std::cout << "[Game] Auto-starting in Play Mode for immediate gameplay" << std::endl;
    std::cout << "[Game] Press Ctrl+P to return to Edit Mode if needed" << std::endl;
#endif

    // Windowed borderless (using proper glfwSetWindowMonitor method)
    WindowManager::GetInstance().SetFullscreen(true);
}



void PredictTrajectory(glm::vec3 startPos, glm::vec3 startVel, glm::vec3 moonCenter, float gravityStrength, std::vector<glm::vec3>& outPoints, float dt = 0.02f, int maxSteps = 10000) {
    glm::vec3 pos = startPos;
    glm::vec3 vel = startVel;
    
    outPoints.clear();
    outPoints.reserve(maxSteps);
    
    glm::vec3 startDir = glm::normalize(startPos - moonCenter);
    bool passedOpposite = false;
    
    for (int i = 0; i < maxSteps; i++) {
        outPoints.push_back(pos);
        
        // Check if we've completed an orbit
        if (i > 10) {
            glm::vec3 currentDir = glm::normalize(pos - moonCenter);
            float dotStart = glm::dot(currentDir, startDir);
            
            if (dotStart < -0.5f) passedOpposite = true;
            if (passedOpposite && dotStart > 0.95f) break;
        }
        
        glm::vec3 dir = glm::normalize(moonCenter - pos);
        glm::vec3 acceleration = dir * gravityStrength;
        
        vel += acceleration * dt;
        pos += vel * dt;
    }
}


void Game::OnUpdate(float deltaTime)
{
    
}

void Game::OnFixedUpdate(float fixedDeltaTime)
{
    const float gravity = 1.62f;
    
    landerRB->AddForce(glm::normalize(-lander->transform->position) * gravity * landerRB->mass);

    // Predict trajectory and set the line renderer points
    PredictTrajectory(
        lander->transform->position,
        landerRB->GetLinearVelocity(),
        glm::vec3(0, 0, 0),
        gravity,
        landerLineRenderer->points,
        fixedDeltaTime
    );
}

void Game::OnShutdown()
{
}