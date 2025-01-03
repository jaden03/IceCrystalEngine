#include <Ice/Components/Renderer.h>

#include <iostream>

#include <Ice/Utils/FileUtil.h>

#include <Ice/Components/Camera.h>
#include <Ice/Components/Light.h>


std::string ModelPath;

Renderer::Renderer() : Component()
{
	material = new Material();
	ModelPath = FileUtil::AssetDir + "Models/cube.obj";
	InitializeRenderer();
}

Renderer::Renderer(std::string modelPath) : Component()
{
	material = new Material();
	ModelPath = modelPath;
	InitializeRenderer();
}

Renderer::Renderer(std::string modelPath, Material* material) : Component()
{
	ModelPath = modelPath;
	this->material = material;
	InitializeRenderer();
}

Renderer::~Renderer()
{
	delete material;

	for (MeshHolder meshHolder : meshHolders)
	{
		glDeleteVertexArrays(1, &meshHolder.vertexArrayObject);
		glDeleteBuffers(1, &meshHolder.vertexBufferObject);
		glDeleteBuffers(1, &meshHolder.uvBufferObject);
		glDeleteBuffers(1, &meshHolder.elementBufferObject);
	}
}


void Renderer::InitializeRenderer()
{
	objl::Loader loader;
	
	bool loadout = loader.LoadFile(ModelPath);

	if (loadout)
	{
		std::vector<objl::Mesh> meshes = loader.LoadedMeshes;
		meshHolders = std::vector<MeshHolder>();

		// loop through the meshes
		for (int i = 0; i < meshes.size(); i++)
		{
			// get the current mesh
			objl::Mesh mesh = meshes[i];

			// get the vertices and uvs
			std::vector<GLfloat> vertices = std::vector<GLfloat>();
			std::vector<GLfloat> uvs = std::vector<GLfloat>();
			std::vector<GLfloat> normals = std::vector<GLfloat>();
			for (int j = 0; j < mesh.Vertices.size(); j++)
			{
				vertices.push_back(mesh.Vertices[j].Position.X);
				vertices.push_back(mesh.Vertices[j].Position.Y);
				vertices.push_back(mesh.Vertices[j].Position.Z);

				uvs.push_back(mesh.Vertices[j].TextureCoordinate.X);
				uvs.push_back(mesh.Vertices[j].TextureCoordinate.Y);

				normals.push_back(mesh.Vertices[j].Normal.X);
				normals.push_back(mesh.Vertices[j].Normal.Y);
				normals.push_back(mesh.Vertices[j].Normal.Z);
			}

			// get the indices
			std::vector<unsigned int> indices = std::vector<unsigned int>();
			for (int j = 0; j < mesh.Indices.size(); j++)
			{
				indices.push_back(mesh.Indices[j]);
			}

			// create a new mesh holder
			MeshHolder meshHolder = MeshHolder(vertices, uvs, normals, indices);

			// add the mesh holder to the vector
			meshHolders.push_back(meshHolder);
		}

		
		for (int i = 0; i < meshHolders.size(); i++)
		{
			glGenVertexArrays(1, &meshHolders[i].vertexArrayObject);

			// bind the vertex array object
			glBindVertexArray(meshHolders[i].vertexArrayObject);

			// create the vertex buffer object and copy the vertices into it
			glGenBuffers(1, &meshHolders[i].vertexBufferObject);
			glBindBuffer(GL_ARRAY_BUFFER, meshHolders[i].vertexBufferObject);
			glBufferData(GL_ARRAY_BUFFER, meshHolders[i].vertices.size() * sizeof(float), &meshHolders[i].vertices[0], GL_STATIC_DRAW);

			// set our vertex attributes pointers
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);


			// create the uv buffer object and copy the uvs into it
			glGenBuffers(1, &meshHolders[i].uvBufferObject);
			glBindBuffer(GL_ARRAY_BUFFER, meshHolders[i].uvBufferObject);
			glBufferData(GL_ARRAY_BUFFER, meshHolders[i].uvs.size() * sizeof(float), &meshHolders[i].uvs[0], GL_STATIC_DRAW);

			// set our vertex attributes pointers
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);


			// create the normal buffer object and copy the normals into it
			glGenBuffers(1, &meshHolders[i].normalBufferObject);
			glBindBuffer(GL_ARRAY_BUFFER, meshHolders[i].normalBufferObject);
			glBufferData(GL_ARRAY_BUFFER, meshHolders[i].normals.size() * sizeof(float), &meshHolders[i].normals[0], GL_STATIC_DRAW);

			// set our vertex attributes pointers
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(2);


			// create the element buffer object and copy the indices into it
			glGenBuffers(1, &meshHolders[i].elementBufferObject);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshHolders[i].elementBufferObject);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshHolders[i].indices.size() * sizeof(unsigned int), &meshHolders[i].indices[0], GL_STATIC_DRAW);

			// unbind the vertex array object to prevent accidental change
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}
	}
	else
	{
		std::cout << "Failed to load model: " << ModelPath << std::endl;
	}


}


void Renderer::Update()
{
	if (material == nullptr)
	{
		std::cout << "Material not found" << std::endl;
		return;
	}
	
	// Create transformations
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection;


	// only need to recalculate the matrices if position, rotation, or scale has changed
	if (transform->position != lastPosition || transform->eulerAngles != lastEulerAngles || transform->scale != lastScale)
	{
		modelMatrix = glm::mat4(1.0f);
		
		// translation
		modelMatrix = glm::translate(modelMatrix, transform->position);

		// Rotate around the local right axis (pitch)
		modelMatrix = glm::rotate(modelMatrix, glm::radians(-transform->eulerAngles.x), glm::vec3(1, 0, 0));
		// Rotate around the local up axis (yaw)
		modelMatrix = glm::rotate(modelMatrix, glm::radians(-transform->eulerAngles.y), glm::vec3(0, 1, 0));
		// Rotate around the local forward axis (roll)
		modelMatrix = glm::rotate(modelMatrix, glm::radians(-transform->eulerAngles.z), glm::vec3(0, 0, 1));

		// scale
		modelMatrix = glm::scale(modelMatrix, transform->scale * transform->localScale);

		// calculate the normal model
		normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

		
		// set the last variables
		lastPosition = transform->position;
		lastEulerAngles = transform->eulerAngles;
		lastScale = transform->scale;
	}
	

	// camera stuff
	if (sceneManager.mainCamera != nullptr)
	{
		Camera* mainCamera = sceneManager.mainCamera;
		view = mainCamera->view;
		projection = mainCamera->projection;
		//projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, sceneManager.mainCamera->nearClippingPlane, sceneManager.mainCamera->farClippingPlane);
	}
	else
	{
		projection = glm::perspective(glm::radians(90.0f), (float)windowManager.windowWidth / (float)windowManager.windowHeight, 0.1f, 10000.0f);
		//projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 10000.0f);
	}


	
	
	// use the shader and set the attributes
	material->shader->Use();
	
	int usedTextureCount = 0;

	// bind the texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, material->texture->Handle);
	GLint textureLocation = glGetUniformLocation(material->shader->Handle, "fragTexture");
	glUniform1i(textureLocation, 0);
	usedTextureCount++;

		// position stuff
	material->shader->setMat4("view", view);
	material->shader->setMat4("projection", projection);
	material->shader->setMat4("model", modelMatrix);
	material->shader->setMat3("normalModel", normalMatrix);
	
		// material stuff
	material->shader->setVec3("fragColor", material->color);
	
	if (sceneManager.mainCamera != nullptr)
		material->shader->setVec3("viewPos", sceneManager.mainCamera->transform->position);

		// lighting stuff
	material->shader->setFloat("ambientLightStrength", lightingManager.ambientLightingStrength);
	material->shader->setVec3("ambientLightColor", lightingManager.ambientLightingColor);
	
	material->shader->setInt("directionalLightCount", lightingManager.directionalLights.size());
	material->shader->setInt("pointLightCount", lightingManager.pointLights.size());

	int numberOfDirectionalLights = lightingManager.directionalLights.size();
	int maxDirectionalLights = lightingManager.maxDirectionalLights;
	if (numberOfDirectionalLights > maxDirectionalLights)
		numberOfDirectionalLights = maxDirectionalLights;

	for (int i = 0; i < numberOfDirectionalLights; i++)
	{
		material->shader->setVec3("directionalLights[" + std::to_string(i) + "].direction", lightingManager.directionalLights[i]->transform->forward);
		material->shader->setVec3("directionalLights[" + std::to_string(i) + "].color", lightingManager.directionalLights[i]->color);
		material->shader->setFloat("directionalLights[" + std::to_string(i) + "].strength", lightingManager.directionalLights[i]->strength);
		material->shader->setBool("directionalLights[" + std::to_string(i) + "].castShadows", lightingManager.directionalLights[i]->castShadows);
		
		glm::mat4 lightSpaceMatrix = lightingManager.directionalLights[i]->GetLightSpaceMatrix();
		material->shader->setMat4("directionalLights[" + std::to_string(i) + "].lightSpaceMatrix", lightSpaceMatrix);

		glActiveTexture(GL_TEXTURE0 + i + usedTextureCount);
		glBindTexture(GL_TEXTURE_2D, lightingManager.directionalLights[i]->depthMap);
		material->shader->setInt("directionalShadowMap[" + std::to_string(i) + "]", i + usedTextureCount);
		usedTextureCount++;
	}

	int numberOfPointLights = lightingManager.pointLights.size();
	int maxPointLights = lightingManager.maxPointLights;
	if (numberOfPointLights > maxPointLights)
		numberOfPointLights = maxPointLights;

	for (int i = 0; i < numberOfPointLights; i++)
	{
		material->shader->setVec3("pointLights[" + std::to_string(i) + "].position", lightingManager.pointLights[i]->transform->position);
		material->shader->setVec3("pointLights[" + std::to_string(i) + "].color", lightingManager.pointLights[i]->color);
		material->shader->setFloat("pointLights[" + std::to_string(i) + "].strength", lightingManager.pointLights[i]->strength);
		material->shader->setFloat("pointLights[" + std::to_string(i) + "].radius", lightingManager.pointLights[i]->radius);
	}

	int numberOfSpotLights = lightingManager.spotLights.size();
	int maxSpotLights = lightingManager.maxSpotLights;
	if (numberOfSpotLights > maxSpotLights)
		numberOfSpotLights = maxSpotLights;
	
	for (int i = 0; i < numberOfSpotLights; i++)
	{
		material->shader->setVec3("spotLights[" + std::to_string(i) + "].position", lightingManager.spotLights[i]->transform->position);
		material->shader->setVec3("spotLights[" + std::to_string(i) + "].direction", lightingManager.spotLights[i]->transform->forward);
		material->shader->setVec3("spotLights[" + std::to_string(i) + "].color", lightingManager.spotLights[i]->color);
		material->shader->setFloat("spotLights[" + std::to_string(i) + "].strength", lightingManager.spotLights[i]->strength);
		material->shader->setFloat("spotLights[" + std::to_string(i) + "].distance", lightingManager.spotLights[i]->distance);
		material->shader->setFloat("spotLights[" + std::to_string(i) + "].angle", glm::cos(glm::radians(lightingManager.spotLights[i]->angle)));
		material->shader->setFloat("spotLights[" + std::to_string(i) + "].outerAngle", glm::cos(glm::radians(lightingManager.spotLights[i]->angle + 5)));
		material->shader->setBool("spotLights[" + std::to_string(i) + "].castShadows", lightingManager.spotLights[i]->castShadows);

		glm::mat4 lightSpaceMatrix = lightingManager.spotLights[i]->GetLightSpaceMatrix();
		material->shader->setMat4("spotLights[" + std::to_string(i) + "].lightSpaceMatrix", lightSpaceMatrix);

		glActiveTexture(GL_TEXTURE0 + i + usedTextureCount);
		glBindTexture(GL_TEXTURE_2D, lightingManager.spotLights[i]->depthMap);
		material->shader->setInt("spotShadowMap[" + std::to_string(i) + "]", i + usedTextureCount);
		usedTextureCount++;
	}
	

		// extra stuff
	float time = glfwGetTime();
	material->shader->setFloat("time", time);

			// poor mans raycasting
	material->shader->setVec3("uniqueColor", owner->uniqueColor / 255.0f);
	
	// loop through meshHolders and actually draw them to the screen
	for (int i = 0; i < meshHolders.size(); i++)
	{
		// bind the vertex array object
		glBindVertexArray(meshHolders[i].vertexArrayObject);
		// draw the elements
		glDrawElements(GL_TRIANGLES, meshHolders[i].indices.size() * sizeof(unsigned int), GL_UNSIGNED_INT, 0);
	}
}



void Renderer::UpdateShadows()
{
	// loop through meshHolders
	for (int i = 0; i < meshHolders.size(); i++)
	{
		// bind the vertex array object
		glBindVertexArray(meshHolders[i].vertexArrayObject);

		// Create transformations
		glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first

		// translation
		model = glm::translate(model, transform->position);

		// Rotate around the local right axis (pitch)
		model = glm::rotate(model, glm::radians(-transform->eulerAngles.x), glm::vec3(1, 0, 0));
		// Rotate around the local up axis (yaw)
		model = glm::rotate(model, glm::radians(-transform->eulerAngles.y), glm::vec3(0, 1, 0));
		// Rotate around the local forward axis (roll)
		model = glm::rotate(model, glm::radians(-transform->eulerAngles.z), glm::vec3(0, 0, 1));

		// scale
		model = glm::scale(model, transform->scale);

		// position stuff
		lightingManager.shadowShader->setMat4("model", model);

		// draw the elements
		glDrawElements(GL_TRIANGLES, meshHolders[i].indices.size() * sizeof(unsigned int), GL_UNSIGNED_INT, 0);

		glFlush();

	}
}