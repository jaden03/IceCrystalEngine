#include <Ice/Core/SceneInitializer.h>

#include <Ice/Core/Actor.h>
#include <Ice/Utils/FileUtil.h>

#include <Ice/Components/Camera.h>
#include <Ice/Components/Renderer.h>
#include <Ice/Components/Freecam.h>

#include <iostream>

SceneInitializer::SceneInitializer()
{
	InitializeScene();
}


void SceneInitializer::InitializeScene()
{
    Actor* cameraActor = new Actor("Main Camera");
    Camera* cameraComponent = cameraActor->AddComponent<Camera>();
    cameraActor->AddComponent<Freecam>();

    Actor* testActor = new Actor("Test Actor", "Test");
    Material* material = new Material(FileUtil::AssetDir + "Materials/object.mat");
    Renderer* renderer = new Renderer(FileUtil::AssetDir + "Models/finch.obj", material);
    testActor->AddComponent(renderer);

    testActor->transform->Translate(0, -4, 5);


    // This takes a few seconds to load because spock49.obj is not an optimized model at all
	/*Actor* testActor2 = new Actor("Test Actor 2", "Test");
	Material* material2 = new Material(FileUtil::AssetDir + "Materials/spock.mat");
	Renderer* renderer2 = new Renderer(FileUtil::AssetDir + "Models/spock49.obj", material2);
	testActor2->AddComponent(renderer2);*/

	
}