#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Ice/Core/WindowManager.h>
#include <Ice/Core/SceneManager.h>
#include <Ice/Utils/OBJLoader.h>
#include <Ice/Utils/FileUtil.h>
#include <Ice/Core/Transform.h>

#include <Ice/Components/Renderer.h>
#include <Ice/Rendering/MeshHolder.h>
#include <Ice/Rendering/Material.h>
#include <Ice/Components/Camera.h>

WindowManager& windowManager = WindowManager::GetInstance();
SceneManager& sceneManager = SceneManager::GetInstance();
std::string ModelPath;

Renderer::Renderer() : Component()
{
	material = new Material();
	ModelPath = FileUtil::AssetDir + "Models/triangulatedCube.obj";
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
}


void Renderer::InitializeRenderer()
{
	objl::Loader loader;
	
	bool loadout = loader.LoadFile(ModelPath);

	if (loadout)
	{
		meshes = loader.LoadedMeshes;
		meshHolders = std::vector<MeshHolder>();

		// loop through the meshes
		for (int i = 0; i < meshes.size(); i++)
		{
			// get the current mesh
			objl::Mesh mesh = meshes[i];

			// get the vertices and uvs
			std::vector<GLfloat> vertices = std::vector<GLfloat>();
			std::vector<GLfloat> uvs = std::vector<GLfloat>();
			for (int j = 0; j < mesh.Vertices.size(); j++)
			{
				vertices.push_back(mesh.Vertices[j].Position.X);
				vertices.push_back(mesh.Vertices[j].Position.Y);
				vertices.push_back(mesh.Vertices[j].Position.Z);

				uvs.push_back(mesh.Vertices[j].TextureCoordinate.X);
				uvs.push_back(mesh.Vertices[j].TextureCoordinate.Y);
			}

			// get the indices
			std::vector<unsigned int> indices = std::vector<unsigned int>();
			for (int j = 0; j < mesh.Indices.size(); j++)
			{
				indices.push_back(mesh.Indices[j]);
			}

			// create a new mesh holder
			MeshHolder meshHolder = MeshHolder(vertices, uvs, indices);

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

	// loop through meshHolders
	for (int i = 0; i < meshHolders.size(); i++)
	{
		// bind the vertex array object
		glBindVertexArray(meshHolders[i].vertexArrayObject);

		


		// temporary
		// Create transformations
		glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);

		float time = glfwGetTime();

		// translation
		model = glm::translate(model, transform->position);
		
		// rotation
		model = glm::rotate(model, transform->rotation.x, glm::vec3(1, 0, 0));
		model = glm::rotate(model, transform->rotation.y, glm::vec3(0, 1, 0));
		model = glm::rotate(model, transform->rotation.z, glm::vec3(0, 0, 1));

		// scale
		model = glm::scale(model, transform->scale);
		

		// camera stuff
		if (sceneManager.mainCamera != nullptr)
		{
			Camera* mainCamera = sceneManager.mainCamera;
			view = glm::lookAt(mainCamera->transform->position, mainCamera->transform->position + mainCamera->transform->forward, mainCamera->transform->up);
			
			projection = glm::perspective(glm::radians(sceneManager.mainCamera->fieldOfView), (float)windowManager.windowWidth / (float)windowManager.windowHeight, sceneManager.mainCamera->nearClippingPlane, sceneManager.mainCamera->farClippingPlane);
		}
		else
		{
			projection = glm::perspective(glm::radians(90.0f), (float)windowManager.windowWidth / (float)windowManager.windowHeight, 0.1f, 10000.0f);
		}


		// bind the texture
		material->texture->Bind();
		// use the shader and set the attributes
		material->shader->Use();
		material->shader->setVec3("fragColor", material->color);
		material->shader->setMat4("view", view);
		material->shader->setMat4("projection", projection);
		material->shader->setMat4("model", model);
		material->shader->setFloat("time", time);

		// draw the elements
		glDrawElements(GL_TRIANGLES, meshHolders[i].indices.size() * sizeof(unsigned int), GL_UNSIGNED_INT, 0);
		//glDrawArrays(GL_TRIANGLES, 0, meshHolders[i].vertices.size() * sizeof(float));

		glFlush();

	}
}