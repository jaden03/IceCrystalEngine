#include "Game.h"

#include <iostream>
#include <ostream>

#include <Ice/Core/Actor.h>
#include <Ice/Utils/FileUtil.h>

#include <Ice/Components/Rendering/Renderer.h>
#include <Ice/Components/Rendering/Light.h>

#include <Ice/Components/Freecam.h>

#include "Ice/Components/Physics/BoxCollider.h"
#include "Ice/Components/Physics/MeshCollider.h"
#include "Ice/Components/Physics/RigidBody.h"

#include "Ice/Components/LuaExecutor.h"

#include "CameraController.h"

#include <Ice/Utils/MathUtils.h>

#include <Ice/Components/Rendering/LineRenderer.h>
#include <Ice/Components/UI/RawImage.h>

#include <Ice/Managers/SceneManager.h>

#include "Ice/Components/Audio/AudioSource.h"
#include "Ice/Resources/AudioClip.h"

#ifdef _DEBUG
#include <Ice/IEditor/EditorUI.h>
#endif

void CreateSun()
{
    // Sun
    Actor* sun = new Actor("Sun", "sun");
    Renderer* sunRenderer = new Renderer(FileUtil::EngineAssetDir + "Models/cone.obj");
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

	// Engine Sound
	AudioSource* as = enginePlume->AddComponent<AudioSource>();
	AudioClip* clip = new AudioClip;
	clip->LoadFromFile(FileUtil::AssetDir + "Sounds/engineLoop.wav");
	as->SetClip(clip);
	as->SetVolume(0.0f);
	as->SetMinDistance(5.0f);
	as->SetMaxDistance(100.0f);
	as->SetLooping(true);
	as->Play();
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



void Game::CreateObject(std::string type, glm::vec3 position, glm::quat rotation)
{
	if (type == "pad")
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
	else if (type == "base")
	{
		// Base	
    	Actor* base = new Actor("Pad", "pad");
    	Renderer* baseRenderer = base->AddComponent<Renderer>(FileUtil::AssetDir + "Models/base.obj", mainMaterial);
    	base->transform->SetPosition(position);
    	base->transform->SetRotation(rotation);
    	base->AddComponent<MeshCollider>(baseRenderer->meshHolders[0].vertices, baseRenderer->meshHolders[0].indices, base->transform->scale);
    	base->AddComponent<RigidBody>(0.0f);

		AudioSource* as = base->AddComponent<AudioSource>();
		AudioClip* clip = new AudioClip;
		clip->LoadFromFile(FileUtil::AssetDir + "Sounds/kspSpaceThemeKevinMaclead.wav");
		as->SetClip(clip);
		as->SetVolume(.2f);
		as->SetMinDistance(5.0f);
		as->SetMaxDistance(100.0f);
		as->SetLooping(true);
		as->Play();
	}
}
void Game::CreateWorld()
{
    CreateObject("pad", MathUtils::BlenderToEngine(glm::vec3(44.58f, -34.67f, 68.55f)), MathUtils::BlenderToEngineQuat(glm::quat(0.959001f, 0.149376f, 0.240800f, 0.004277f)));
    CreateObject("pad", MathUtils::BlenderToEngine(glm::vec3(-85.22f, -13.97f, 52.32f)), MathUtils::BlenderToEngineQuat(glm::quat(0.860231f, -0.107791f, -0.419877f, 0.268492f)));
    CreateObject("pad", MathUtils::BlenderToEngine(glm::vec3(3.16f, 88.14f, 20.63f)), MathUtils::BlenderToEngineQuat(glm::quat(0.744106f, -0.631242f, -0.130073f, -0.175845f)));
    CreateObject("pad", MathUtils::BlenderToEngine(glm::vec3(-9.45f, 35.79f, 88.66f)), MathUtils::BlenderToEngineQuat(glm::quat(0.888069f, -0.226797f, 0.129038f, 0.378479f)));
    CreateObject("pad", MathUtils::BlenderToEngine(glm::vec3(94.57f, 7.68f, 0.78f)), MathUtils::BlenderToEngineQuat(glm::quat(0.643592f, 0.329059f, 0.634373f, 0.274008f)));
    CreateObject("pad", MathUtils::BlenderToEngine(glm::vec3(53.58f, -51.03f, -59.88f)), MathUtils::BlenderToEngineQuat(glm::quat(0.442553f, 0.665599f, 0.600259f, 0.028527f)));
    CreateObject("pad", MathUtils::BlenderToEngine(glm::vec3(-45.17f, -22.14f, -82.00f)), MathUtils::BlenderToEngineQuat(glm::quat(-0.047015f, 0.697573f, 0.667251f, -0.256822f)));
    CreateObject("pad", MathUtils::BlenderToEngine(glm::vec3(44.71f, 35.77f, -76.99f)), MathUtils::BlenderToEngineQuat(glm::quat(-0.052434f, 0.653777f, 0.696809f, 0.290316f)));

	CreateObject("base", MathUtils::BlenderToEngine(glm::vec3(47.79f, -15.77f, 70.82f)), MathUtils::BlenderToEngineQuat(glm::quat(0.517686f, 0.301762f, 0.016011f, 0.800428f)));
}


void CreateUI()
{
    Actor* fuelBarBG = new Actor("Fuel Bar Background");
    fuelBarBG->AddComponent<RawImage>(FileUtil::AssetDir + "Textures/fuelBarBG.png");
    fuelBarBG->transform->SetScale(glm::vec3(400, 50, 100));
    fuelBarBG->transform->Translate(80, 80, 0);
    
    Actor* fuelBarFG = new Actor("Fuel Bar Foreground", "fuelBarFG");
    fuelBarFG->AddComponent<RawImage>(FileUtil::AssetDir + "Textures/fuelBarFG.png");
    fuelBarFG->transform->SetScale(glm::vec3(400, 50, 100));
    fuelBarFG->transform->Translate(80, 80, 0);

    Actor* sasToggleBG = new Actor("SAS Toggle Background");
    sasToggleBG->AddComponent<RawImage>(FileUtil::AssetDir + "Textures/sasToggleBG.png");
    sasToggleBG->transform->SetScale(glm::vec3(150, 50, 100));
    sasToggleBG->transform->Translate(80, 150, 0);

    Actor* sasToggleFG = new Actor("SAS Toggle Foreground", "sasToggleFG");
    RawImage* sasToggleFGImage = sasToggleFG->AddComponent<RawImage>(FileUtil::AssetDir + "Textures/sasToggleFG.png");
    sasToggleFG->transform->SetScale(glm::vec3(150, 50, 100));
    sasToggleFG->transform->Translate(80, 150, 0);
    sasToggleFGImage->enabled = false;
}




void Game::OnInit()
{
    std::cout << "========================================" << std::endl;
    std::cout << "[Game] Initializing Game..." << std::endl;
    std::cout << "========================================" << std::endl;

    // Set the gravity to zero, we will apply forces towards the center of world
    // PhysicsManager::GetInstance().GetSystem().SetGravity(JPH::Vec3(0.0f, -1.62f, 0.0f));
    // PhysicsManager::GetInstance().GetSystem().SetGravity(JPH::Vec3(0.0f, 0.0f, 0.0f));
	PhysicsManager::GetInstance().SetGravity(glm::vec3(0,0,0));
	
    // Create the horizontal and vertical input axis'
    Input::CreateAxis("horizontal", GLFW_KEY_D, GLFW_KEY_A);
    Input::CreateAxis("vertical", GLFW_KEY_W, GLFW_KEY_S);
    
    // Create the main material used for everything
    mainMaterial = new Material(FileUtil::AssetDir + "Materials/main.mat");
    unlitMaterial = new Material(FileUtil::EngineAssetDir + "Materials/unlit.mat");
    unlitMaterial->color = glm::vec3(0.0f, 1.0f, 3.0f);

    CreateSun();
    CreateMoon();
    CreateLander();
    CreateCamera();
    CreateWorld(); // pads, buildings, etc
    CreateUI();
    
    std::cout << "========================================" << std::endl;
    std::cout << "[Game] Scene initialization complete!" << std::endl;
    std::cout << "[Game] Total actors in scene: " << SceneManager::GetInstance().GetActorCount() << std::endl;

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