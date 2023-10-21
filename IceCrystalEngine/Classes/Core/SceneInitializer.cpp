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
	Material* unlitMaterial = new Material(FileUtil::AssetDir + "Materials/unlit.mat");
	//unlitMaterial->color = glm::vec3(1.0f, 1.0f, 1.0f);


    Actor* cameraActor = new Actor("Main Camera");
    Camera* cameraComponent = cameraActor->AddComponent<Camera>();
    cameraActor->AddComponent<Freecam>();


	// Airship
    Actor* testActor = new Actor("Test Actor", "Test");
    Material* material = new Material(FileUtil::AssetDir + "Materials/object.mat");
    Renderer* testRenderer = new Renderer(FileUtil::AssetDir + "Models/finch.obj", material);
    testActor->AddComponent(testRenderer);

    testActor->transform->Translate(0, 0, 5);



	// Floor
	Actor* floorActor = new Actor("Floor");
	Renderer* floorRenderer = new Renderer(FileUtil::AssetDir + "Models/cube.obj");
	floorActor->AddComponent(floorRenderer);
	floorActor->transform->scale = glm::vec3(8, 1, 8);
	floorActor->transform->Translate(0, -5, 0);
	
	
	// Cube
	Actor* cubeActor = new Actor("Cube");
	Renderer* cubeRenderer = new Renderer(FileUtil::AssetDir + "Models/cube.obj");
	cubeActor->AddComponent(cubeRenderer);
	cubeActor->transform->scale = glm::vec3(0.5f, 0.5f, 0.5f);
	cubeActor->transform->Translate(0, -3, 3);




	Actor* sun = new Actor("Sun", "sun");
    Renderer* sunRenderer = new Renderer(FileUtil::AssetDir + "Models/cone.obj", unlitMaterial);
	sun->AddComponent(sunRenderer);
    DirectionalLight* sunLight = sun->AddComponent<DirectionalLight>();
	sunRenderer->castShadows = false;
	//sunLight->castShadows = false;

	// set the rotation quat to be at an angle and angled down
	//sun->transform->Rotate(230, 0, 0);
	sun->transform->Translate(0, 5, -12.5f);
	sun->transform->Rotate(-30, 0, 0);
	sun->transform->scale = glm::vec3(0.2f, 0.2f, 0.2f);


	
	//Actor* sun2 = new Actor("Sun2");
	//Renderer* sunRenderer2 = new Renderer(FileUtil::AssetDir + "Models/cone.obj");
	//sun2->AddComponent(sunRenderer2);
	//sun2->AddComponent<DirectionalLight>();

	//// set the rotation quat to be at an angle and angled down (different from the first sun)
	//sun2->transform->Rotate(210, 0, 0);
	//sun2->transform->Translate(0, 5, 20);
	//sun2->transform->scale = glm::vec3(0.2f, 0.2f, 0.2f);




	Actor* pointLight = new Actor("Point Light", "PointLight1");
	PointLight* pointLightComponent = pointLight->AddComponent<PointLight>();
	//pointLightComponent->color = glm::vec3(1.0f, 0.0f, 0.0f);
	pointLight->transform->Translate(0, -3, 10);
	Renderer* pointLightRenderer = new Renderer(FileUtil::AssetDir + "Models/cube.obj", unlitMaterial);
	pointLight->AddComponent(pointLightRenderer);
	pointLight->transform->scale = glm::vec3(0.1f, 0.1f, 0.1f);
	pointLightRenderer->castShadows = false;
	pointLight->transform->SetParent(testActor->transform);

	/*Actor* pointLight2 = new Actor("Point Light", "PointLight2");
	PointLight* pointLightComponent2 = pointLight2->AddComponent<PointLight>();
	pointLightComponent2->color = glm::vec3(0.0f, 0.0f, 1.0f);
	pointLight2->transform->Translate(0, -3, 4);*/

	/*Actor* pointLight3 = new Actor("Point Light", "PointLight3");
	PointLight* pointLightComponent3 = pointLight3->AddComponent<PointLight>();
	pointLightComponent3->color = glm::vec3(0.0f, 0.0f, 1.0f);*/

     //This takes a few seconds to load because spock49.obj is not an optimized model at all
	/*Actor* testActor2 = new Actor("Test Actor 2", "Test");
	Material* material2 = new Material(FileUtil::AssetDir + "Materials/spock.mat");
	Renderer* renderer2 = new Renderer(FileUtil::AssetDir + "Models/spock49.obj", material2);
	testActor2->AddComponent(renderer2);*/

	
}