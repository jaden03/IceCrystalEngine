#include <Ice/Core/SceneInitializer.h>

#include <Ice/Components/Camera.h>
#include <Ice/Components/Renderer.h>
#include <Ice/Components/Freecam.h>
#include <Ice/Components/Light.h>

#include <iostream>

#include "Ice/Core/LuaManager.h"

SceneInitializer::SceneInitializer()
{
	InitializeScene();
}


void SceneInitializer::InitializeScene()
{
	Material* unlitMaterial = new Material(FileUtil::AssetDir + "Materials/unlit.mat");
	
	Material* unlitMaterialBlue = new Material(FileUtil::AssetDir + "Materials/unlit.mat");
	unlitMaterialBlue->color = glm::vec3(0.0f, 0.0f, 15.0f); // making the color higher than 1.0 will make it glow with bloom

	Material* unlitMaterialRed = new Material(FileUtil::AssetDir + "Materials/unlit.mat");
	unlitMaterialRed->color = glm::vec3(10.0f, 0.0f, 0.0f);

	Material* unlitMaterialGreen = new Material(FileUtil::AssetDir + "Materials/unlit.mat");
	unlitMaterialGreen->color = glm::vec3(0.0f, 10.0f, 0.0f);
	
    Actor* cameraActor = new Actor("Main Camera");
    Camera* cameraComponent = cameraActor->AddComponent<Camera>();
    cameraActor->AddComponent<Freecam>();

	//This takes a few seconds to load because spock49.obj is a very high poly model, but mostly its just because my fbx loader is just slow
	// Actor* testActor2 = new Actor("Test Actor 2", "Test");
	// Material* material2 = new Material(FileUtil::AssetDir + "Materials/spock.mat");
	// Renderer* renderer2 = new Renderer(FileUtil::AssetDir + "Models/spock49.obj", material2);
	// testActor2->AddComponent(renderer2);
	// testActor2->transform->Rotate(0, 90, 0);
	// testActor2->transform->Translate(-.05f, -1.5, -.15f);
	// testActor2->transform->SetParent(cameraActor->transform);
	


	// Airship
    Actor* testActor = new Actor("Airship", "Test");
    Material* material = new Material(FileUtil::AssetDir + "Materials/object.mat");
    Renderer* testRenderer = new Renderer(FileUtil::AssetDir + "Models/finch.obj", material);
    testActor->AddComponent(testRenderer);

    testActor->transform->Translate(0, 0, 5);



	// Floor
	Actor* floorActor = new Actor("Floor", "Floor");
	Renderer* floorRenderer = new Renderer(FileUtil::AssetDir + "Models/cube.obj");
	floorActor->AddComponent(floorRenderer);
	floorActor->transform->scale = glm::vec3(15, 1, 15);
	floorActor->transform->Translate(0, -5, 0);
	
	
	// Cube
	Actor* icosphereActor = new Actor("Icosphere");
	Renderer* icosphereRenderer = new Renderer(FileUtil::AssetDir + "Models/icosphere.obj");
	icosphereActor->AddComponent(icosphereRenderer);
	icosphereActor->transform->scale = glm::vec3(0.25f, 0.25f, 0.25f);
	icosphereActor->transform->Translate(0, -3, 0);




	Actor* sun = new Actor("Sun", "sun");
    Renderer* sunRenderer = new Renderer(FileUtil::AssetDir + "Models/cone.obj");
	sun->AddComponent(sunRenderer);
    DirectionalLight* sunLight = sun->AddComponent<DirectionalLight>();
	sunRenderer->castShadows = false;

	//// set the rotation quat to be at an angle and angled down
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




	Actor* pointLight = new Actor("Blue Light", "PointLight1");
	PointLight* pointLightComponent = pointLight->AddComponent<PointLight>();
	pointLight->transform->Translate(0, -3, 0);
	Renderer* pointLightRenderer = new Renderer(FileUtil::AssetDir + "Models/icosphere.obj", unlitMaterialBlue);
	pointLight->AddComponent(pointLightRenderer);
	pointLight->transform->scale = glm::vec3(0.05f, 0.05f, 0.05f);
	pointLightRenderer->castShadows = false;
	pointLightComponent->color = glm::vec3(0.0f, 0.0f, 1.0f);
	pointLightComponent->strength = 5;

	Actor* pointLight2 = new Actor("Green Light", "PointLight2");
	PointLight* pointLightComponent2 = pointLight2->AddComponent<PointLight>();
	pointLight2->transform->Translate(0, -3, 0);
	Renderer* pointLightRenderer2 = new Renderer(FileUtil::AssetDir + "Models/icosphere.obj", unlitMaterialGreen);
	pointLight2->AddComponent(pointLightRenderer2);
	pointLight2->transform->scale = glm::vec3(0.05f, 0.05f, 0.05f);
	pointLightRenderer2->castShadows = false;
	pointLightComponent2->color = glm::vec3(0.0f, 1.0f, 0.0f);
	pointLightComponent2->strength = 5;

	Actor* pointLight3 = new Actor("Red Light", "PointLight3");
	PointLight* pointLightComponent3 = pointLight3->AddComponent<PointLight>();
	pointLight3->transform->Translate(0, -3, 0);
	Renderer* pointLightRenderer3 = new Renderer(FileUtil::AssetDir + "Models/icosphere.obj", unlitMaterialRed);
	pointLight3->AddComponent(pointLightRenderer3);
	pointLight3->transform->scale = glm::vec3(0.05f, 0.05f, 0.05f);
	pointLightRenderer3->castShadows = false;
	pointLightComponent3->color = glm::vec3(1.0f, 0.0f, 0.0f);
	pointLightComponent3->strength = 5;

	Actor* pointLight4 = new Actor("White Light", "PointLight4");
	PointLight* pointLightComponent4 = pointLight4->AddComponent<PointLight>();
	pointLight4->transform->Translate(0, -1, 0);
	Renderer* pointLightRenderer4 = new Renderer(FileUtil::AssetDir + "Models/icosphere.obj", unlitMaterial);
	pointLight4->AddComponent(pointLightRenderer4);
	pointLight4->transform->scale = glm::vec3(0.025f, 0.025f, 0.025f);
	pointLightRenderer4->castShadows = false;




	// Actor* spotLight = new Actor("Spot Light", "SpotLight1");
	// SpotLight* spotLightComponent = spotLight->AddComponent<SpotLight>();
	// spotLight->transform->Translate(0, 0, -5);
	// spotLight->transform->Rotate(-25, 0, 0);
	// Renderer* spotLightRenderer = new Renderer(FileUtil::AssetDir + "Models/cone.obj", unlitMaterial);
	// spotLight->AddComponent(spotLightRenderer);
	// spotLight->transform->scale = glm::vec3(0.05f, 0.05f, 0.05f);
	// spotLightRenderer->castShadows = false;
	// spotLightComponent->strength = 5;

	
}
