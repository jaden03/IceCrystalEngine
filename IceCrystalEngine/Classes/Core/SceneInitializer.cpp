#include <Ice/Core/SceneInitializer.h>

#include <Ice/Components/Camera.h>
#include <Ice/Components/Renderer.h>
#include <Ice/Components/Freecam.h>
#include <Ice/Components/Light.h>

#include <iostream>

#include "Ice/Components/RawImage.h"
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

	

	Actor* sun = new Actor("Sun", "sun");
	Renderer* sunRenderer = new Renderer(FileUtil::AssetDir + "Models/cone.obj");
	sun->AddComponent(sunRenderer);
	DirectionalLight* sunLight = sun->AddComponent<DirectionalLight>();
	sunRenderer->castShadows = false;
	// Angle the sun with a parent Actor
	Actor* sunTilt = new Actor("Sun Tilt");
	sun->transform->SetParent(sunTilt->transform);
	sunTilt->transform->Rotate(glm::vec3(0.0f, 45.0f, 0.0f));
	//// set the rotation quat to be at an angle and angled down
	sun->transform->TranslateLocal(0, 5, -12.5f);
	sun->transform->RotateLocal(-30, 0, 0);
	sun->transform->scale = glm::vec3(0.2f, 0.2f, 0.2f);

	Actor* testImage = new Actor("Test Image", "testImage");
	RawImage* rawImage = new RawImage(FileUtil::AssetDir + "Textures/ObjectGrid.png");
	testImage->AddComponent(rawImage);
	testImage->transform->SetScale(glm::vec3(150, 150, 100));
	testImage->transform->Translate(400, 50, 0);

	Actor* testImage2 = new Actor();
	RawImage* rawImage2 = new RawImage(FileUtil::AssetDir + "Textures/crate.png");
	testImage2->AddComponent(rawImage2);
	testImage2->transform->SetScale(glm::vec3(100, 100, 100));
	testImage2->transform->Translate(600, 50, 0);
	

	
	Material* crateMaterial = new Material(FileUtil::AssetDir + "Materials/crate.mat");
	
	int rows = 5;  // Number of rows in the grid
	int cols = 5;  // Number of columns in the grid
	float spacing = 4.0f;  // Space between crates

	// Calculate the offset to center the grid
	float offsetX = (cols - 1) * spacing / 2.0f;
	float offsetZ = (rows - 1) * spacing / 2.0f;

	// Loop through rows and columns to create a grid of crates
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			// Create a new crate actor for each grid position
			Actor* crate = new Actor();
        
			// Position the crate based on its row and column, adjusting by the offset
			crate->transform->Translate(j * spacing - offsetX, -5, i * spacing - offsetZ);  // Centering the grid

			// Create the crate renderer and assign the material
			Renderer* crateRenderer = new Renderer(FileUtil::AssetDir + "Models/cube.obj", crateMaterial);
        
			// Add the renderer to the crate actor
			crate->AddComponent(crateRenderer);
		}
	}

	

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

	

	// Floor
	Actor* floorActor = new Actor("Floor", "Floor");
	Renderer* floorRenderer = new Renderer(FileUtil::AssetDir + "Models/cube.obj");
	floorActor->AddComponent(floorRenderer);
	floorActor->transform->scale = glm::vec3(15, 1, 15);
	floorActor->transform->Translate(0, -7, 0);
	
}
