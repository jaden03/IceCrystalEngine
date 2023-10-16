#include <Ice/Utils/OBJLoader.h>
#include <Ice/Rendering/Material.h>
#include <Ice/Rendering/Renderer.h>
#include <Ice/Utils/FileUtil.h>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <Ice/Rendering/MeshHolder.h>

std::string ModelPath;
unsigned int VertexArrayObject;

Renderer::Renderer(Actor* owner, std::string modelPath) : Component(owner)
{
	std::cout << FileUtil::AssetDir;

	material = new Material();
	ModelPath = modelPath;
	InitializeRenderer();
}

Renderer::Renderer(Actor* owner, std::string modelPath, Material* material) : Component(owner)
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
	glGenVertexArrays(1, &VertexArrayObject);

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
	}
	else
	{
		std::cout << "Failed to load model: " << ModelPath << std::endl;
	}

}


void Renderer::Render(glm::mat4 view, glm::mat4 projection, glm::mat4 model, int vertexBufferObject, int uvBufferObject, int elementBufferObject)
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
		glBindVertexArray(VertexArrayObject);

		// copy vertices array into a buffer for OpenGL to use
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, meshHolders[i].vertices.size() * sizeof(float), &meshHolders[i].vertices[0], GL_STATIC_DRAW);

		// set our vertex attributes pointers
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		

		// copy uvs array into a buffer for OpenGL to use
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferObject);
		glBufferData(GL_ARRAY_BUFFER, meshHolders[i].uvs.size() * sizeof(float), &meshHolders[i].uvs[0], GL_STATIC_DRAW);

		// set our vertex attributes pointers
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);

		
		// copy indices array into a buffer for OpenGL to use
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshHolders[i].indices.size() * sizeof(unsigned int), &meshHolders[i].indices[0], GL_STATIC_DRAW);

		// unbind the vertex array object to prevent accidental change
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);


		// bind the texture
		material->texture->Bind();
		// use the shader and set the attributes
		material->shader->Use();
		material->shader->setVec3("fragColor", material->color);
		material->shader->setMat4("view", view);
		material->shader->setMat4("projection", projection);
		material->shader->setMat4("model", model);

		// bind the vertex array object
		glBindVertexArray(VertexArrayObject);

		// draw the elements
		glDrawElements(GL_TRIANGLES, meshHolders[i].indices.size() * sizeof(unsigned int), GL_UNSIGNED_INT, 0);
		//glDrawArrays(GL_TRIANGLES, 0, meshHolders[i].vertices.size() * sizeof(float));

		glFlush();

	}
}