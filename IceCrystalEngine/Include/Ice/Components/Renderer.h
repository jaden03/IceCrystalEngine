#pragma once
#ifndef RENDERER_H

#define RENDERER_H

#include <Ice/Utils/OBJLoader.h>
#include <Ice/Rendering/Material.h>
#include <Ice/Rendering/MeshHolder.h>
#include <glm/gtc/type_ptr.hpp>
#include <Ice/Core/Component.h>

class Renderer : public Component
{

	void InitializeRenderer();

public:
	
	std::vector<objl::Mesh> meshes;
	Material* material;

	std::vector<MeshHolder> meshHolders;

	Renderer();
	Renderer(std::string modelPath);
	Renderer(std::string modelPath, Material* material);

	~Renderer();
	
	void Update() override;
};

#endif