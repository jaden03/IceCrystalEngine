#include <Ice/Components/Renderer.h>

#include <iostream>

#include <Ice/Utils/FileUtil.h>

#include <Ice/Components/Camera.h>
#include <Ice/Components/Light.h>

#include "Ice/Core/RendererManager.h"

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



std::string Renderer::GetCachePath()
{
	return ModelPath + ".cache";
}

bool Renderer::IsCacheValid()
{
	namespace fs = std::filesystem;

	if (!fs::exists(GetCachePath())) return false;

	auto cacheTime = fs::last_write_time(GetCachePath());
	auto modelTime = fs::last_write_time(ModelPath);

	return cacheTime >= modelTime;
}

bool Renderer::LoadFromCache() {
	std::string cachePath = GetCachePath();
    std::ifstream file(cachePath, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Failed to open cache file: " << cachePath << std::endl;
        return false;
    }
    
    // Read and verify version
    uint32_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (version != CACHE_VERSION) {
        std::cout << "Cache version mismatch. Rebuilding cache." << std::endl;
        return false;
    }
    
    // Read number of meshes
    size_t numMeshes;
    file.read(reinterpret_cast<char*>(&numMeshes), sizeof(numMeshes));
    
    meshHolders.clear();
    meshHolders.reserve(numMeshes);
    
    // Read each mesh
    for (size_t i = 0; i < numMeshes; i++) {
        // Read vertices
        size_t vertexCount;
        file.read(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));
        std::vector<GLfloat> vertices(vertexCount);
        file.read(reinterpret_cast<char*>(vertices.data()), vertexCount * sizeof(GLfloat));
        
        // Read UVs
        size_t uvCount;
        file.read(reinterpret_cast<char*>(&uvCount), sizeof(uvCount));
        std::vector<GLfloat> uvs(uvCount);
        file.read(reinterpret_cast<char*>(uvs.data()), uvCount * sizeof(GLfloat));
        
        // Read normals
        size_t normalCount;
        file.read(reinterpret_cast<char*>(&normalCount), sizeof(normalCount));
        std::vector<GLfloat> normals(normalCount);
        file.read(reinterpret_cast<char*>(normals.data()), normalCount * sizeof(GLfloat));
        
        // Read indices
        size_t indexCount;
        file.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
        std::vector<unsigned int> indices(indexCount);
        file.read(reinterpret_cast<char*>(indices.data()), indexCount * sizeof(unsigned int));
        
        // Create mesh holder
        meshHolders.emplace_back(vertices, uvs, normals, indices);
    }
    
    file.close();
    
    if (file.fail() && !file.eof()) {
        std::cout << "Error reading cache file. Cache may be corrupted." << std::endl;
        meshHolders.clear();
        return false;
    }
    
    std::cout << "Loaded " << numMeshes << " meshes from cache: " << cachePath << std::endl;
    return true;
}

void Renderer::SaveToCache() {
	std::string cachePath = GetCachePath();
	std::ofstream file(cachePath, std::ios::binary);
	if (!file.is_open()) {
		std::cout << "Failed to create cache file: " << cachePath << std::endl;
		return;
	}
    
	// Write version
	file.write(reinterpret_cast<const char*>(&CACHE_VERSION), sizeof(CACHE_VERSION));
    
	// Write number of meshes
	size_t numMeshes = meshHolders.size();
	file.write(reinterpret_cast<const char*>(&numMeshes), sizeof(numMeshes));
    
	// Write each mesh
	for (const auto& mesh : meshHolders) {
		// Write vertices
		size_t vertexCount = mesh.vertices.size();
		file.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
		file.write(reinterpret_cast<const char*>(mesh.vertices.data()), vertexCount * sizeof(GLfloat));
        
		// Write UVs
		size_t uvCount = mesh.uvs.size();
		file.write(reinterpret_cast<const char*>(&uvCount), sizeof(uvCount));
		file.write(reinterpret_cast<const char*>(mesh.uvs.data()), uvCount * sizeof(GLfloat));
        
		// Write normals
		size_t normalCount = mesh.normals.size();
		file.write(reinterpret_cast<const char*>(&normalCount), sizeof(normalCount));
		file.write(reinterpret_cast<const char*>(mesh.normals.data()), normalCount * sizeof(GLfloat));
        
		// Write indices
		size_t indexCount = mesh.indices.size();
		file.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));
		file.write(reinterpret_cast<const char*>(mesh.indices.data()), indexCount * sizeof(unsigned int));
	}
    
	file.close();
	std::cout << "Saved " << numMeshes << " meshes to cache: " << cachePath << std::endl;
}

bool Renderer::LoadFromOBJ() {
	objl::Loader loader;
	bool loadout = loader.LoadFile(ModelPath);
    
	if (!loadout) {
		std::cout << "Failed to load model: " << ModelPath << std::endl;
		return false;
	}
    
	std::vector<objl::Mesh> meshes = loader.LoadedMeshes;
	meshHolders.clear();
	meshHolders.reserve(meshes.size());
    
	// Loop through the meshes
	for (const auto& mesh : meshes) {
		// Get the vertices, uvs, and normals
		std::vector<GLfloat> vertices;
		std::vector<GLfloat> uvs;
		std::vector<GLfloat> normals;
        
		vertices.reserve(mesh.Vertices.size() * 3);
		uvs.reserve(mesh.Vertices.size() * 2);
		normals.reserve(mesh.Vertices.size() * 3);
        
		for (const auto& vertex : mesh.Vertices) {
			vertices.push_back(vertex.Position.X);
			vertices.push_back(vertex.Position.Y);
			vertices.push_back(vertex.Position.Z);
            
			uvs.push_back(vertex.TextureCoordinate.X);
			uvs.push_back(vertex.TextureCoordinate.Y);
            
			normals.push_back(vertex.Normal.X);
			normals.push_back(vertex.Normal.Y);
			normals.push_back(vertex.Normal.Z);
		}
        
		// Get the indices
		std::vector<unsigned int> indices(mesh.Indices.begin(), mesh.Indices.end());
        
		// Create a new mesh holder
		meshHolders.emplace_back(vertices, uvs, normals, indices);
	}
    
	std::cout << "Loaded " << meshes.size() << " meshes from OBJ: " << ModelPath << std::endl;
	return true;
}


void Renderer::CreateGLBuffers() {
    for (auto& mesh : meshHolders) {
        glGenVertexArrays(1, &mesh.vertexArrayObject);
        glBindVertexArray(mesh.vertexArrayObject);
        
        // Create the vertex buffer object and copy the vertices into it
        glGenBuffers(1, &mesh.vertexBufferObject);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferObject);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), 
                     mesh.vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Create the UV buffer object and copy the UVs into it
        glGenBuffers(1, &mesh.uvBufferObject);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.uvBufferObject);
        glBufferData(GL_ARRAY_BUFFER, mesh.uvs.size() * sizeof(float), 
                     mesh.uvs.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        
        // Create the normal buffer object and copy the normals into it
        glGenBuffers(1, &mesh.normalBufferObject);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.normalBufferObject);
        glBufferData(GL_ARRAY_BUFFER, mesh.normals.size() * sizeof(float), 
                     mesh.normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(2);
        
        // Create the element buffer object and copy the indices into it
        glGenBuffers(1, &mesh.elementBufferObject);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.elementBufferObject);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), 
                     mesh.indices.data(), GL_STATIC_DRAW);
        
        // Unbind the vertex array object to prevent accidental change
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
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
	std::string cachePath = GetCachePath();
	bool loaded = false;
    
	// Try to load from cache first
	if (IsCacheValid()) {
		loaded = LoadFromCache();
	}
    
	// If cache failed or doesn't exist, load from OBJ
	if (!loaded) {
		if (LoadFromOBJ()) {
			// Save to cache for next time
			SaveToCache();
		} else {
			return; // Failed to load model
		}
	}
    
	// Create OpenGL buffers
	CreateGLBuffers();
}


void Renderer::Update()
{
	if (material == nullptr)
	{
		std::cout << "Material not found" << std::endl;
		return;
	}

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

	// use the shader and set the attributes
	material->shader->Use();
	
	// bind the texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, material->texture->Handle);
	GLint textureLocation = glGetUniformLocation(material->shader->Handle, "fragTexture");
	glUniform1i(textureLocation, 0);

		// position stuff
	material->shader->setMat4("model", modelMatrix);
	material->shader->setMat3("normalModel", normalMatrix);
	
		// material stuff
	material->shader->setVec3("fragColor", material->color);

	// spot lights
	int numberOfSpotLights = lightingManager.spotLights.size();
	int maxSpotLights = lightingManager.maxSpotLights;
	if (numberOfSpotLights > maxSpotLights)
		numberOfSpotLights = maxSpotLights;
	
	for (int i = 0; i < numberOfSpotLights; i++)
	{
		std::string prefix = "spotLights[" + std::to_string(i) + "].";
		material->shader->setVec3(prefix + "position", lightingManager.spotLights[i]->transform->position);
		material->shader->setVec3(prefix + "direction", lightingManager.spotLights[i]->transform->forward);
		material->shader->setVec3(prefix + "color", lightingManager.spotLights[i]->color);
		material->shader->setFloat(prefix + "strength", lightingManager.spotLights[i]->strength);
		material->shader->setFloat(prefix + "distance", lightingManager.spotLights[i]->distance);
		material->shader->setFloat(prefix + "angle", glm::cos(glm::radians(lightingManager.spotLights[i]->angle)));
		material->shader->setFloat(prefix + "outerAngle", glm::cos(glm::radians(lightingManager.spotLights[i]->angle + 5)));
		material->shader->setBool(prefix + "castShadows", lightingManager.spotLights[i]->castShadows);

		glm::mat4 lightSpaceMatrix = lightingManager.spotLights[i]->GetLightSpaceMatrix();
		material->shader->setMat4(prefix + "lightSpaceMatrix", lightSpaceMatrix);

		glActiveTexture(GL_TEXTURE0 + sceneManager.usedTextureCount);
		glBindTexture(GL_TEXTURE_2D, lightingManager.spotLights[i]->depthMap);
		material->shader->setInt("spotShadowMap[" + std::to_string(i) + "]", sceneManager.usedTextureCount);
		sceneManager.usedTextureCount++;
	}

		// poor mans raycasting
	material->shader->setVec3("uniqueColor", owner->uniqueColor / 255.0f);
	
	// loop through meshHolders and actually draw them to the screen
	for (int i = 0; i < meshHolders.size(); i++)
	{
		// bind the vertex array object
		glBindVertexArray(meshHolders[i].vertexArrayObject);
		// draw the elements
		glDrawElements(GL_TRIANGLES, meshHolders[i].indices.size(), GL_UNSIGNED_INT, 0);
	}
}



void Renderer::UpdateShadows()
{
	// position stuff
	lightingManager.shadowShader->setMat4("model", modelMatrix);
	lightingManager.shadowsCascadedShader->setMat4("model", modelMatrix);
	
	// loop through meshHolders
	for (int i = 0; i < meshHolders.size(); i++)
	{
		// bind the vertex array object
		glBindVertexArray(meshHolders[i].vertexArrayObject);
		// draw the elements
		glDrawElements(GL_TRIANGLES, meshHolders[i].indices.size(), GL_UNSIGNED_INT, 0);
	}
}