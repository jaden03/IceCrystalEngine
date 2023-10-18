#include <Ice/Core/SceneInitializer.h>

#include <Ice/Components/Camera.h>
#include <Ice/Components/Renderer.h>
#include <Ice/Components/Freecam.h>
#include <Ice/Components/Light.h>

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
    Renderer* testRenderer = new Renderer(FileUtil::AssetDir + "Models/finch.obj", material);
    testActor->AddComponent(testRenderer);

    testActor->transform->Translate(0, -4, 5);


	Actor* sun = new Actor("Sun");
    Renderer* sunRenderer = new Renderer(FileUtil::AssetDir + "Models/cube.obj");
	sun->AddComponent(sunRenderer);
    sun->AddComponent<DirectionalLight>();

    sun->transform->Rotate(-45, 45, 0);
	sun->transform->scale = glm::vec3(0.2f, 0.2f, 0.2f);


	Actor* pointLight = new Actor("Point Light", "PointLight1");
	PointLight* pointLightComponent = pointLight->AddComponent<PointLight>();
	pointLightComponent->color = glm::vec3(1.0f, 0.0f, 0.0f);

	Actor* pointLight2 = new Actor("Point Light", "PointLight2");
	PointLight* pointLightComponent2 = pointLight2->AddComponent<PointLight>();
	pointLightComponent2->color = glm::vec3(0.0f, 1.0f, 0.0f);

	Actor* pointLight3 = new Actor("Point Light", "PointLight3");
	PointLight* pointLightComponent3 = pointLight3->AddComponent<PointLight>();
	pointLightComponent3->color = glm::vec3(0.0f, 0.0f, 1.0f);

    // This takes a few seconds to load because spock49.obj is not an optimized model at all
	/*Actor* testActor2 = new Actor("Test Actor 2", "Test");
	Material* material2 = new Material(FileUtil::AssetDir + "Materials/spock.mat");
	Renderer* renderer2 = new Renderer(FileUtil::AssetDir + "Models/spock49.obj", material2);
	testActor2->AddComponent(renderer2);*/

	
}