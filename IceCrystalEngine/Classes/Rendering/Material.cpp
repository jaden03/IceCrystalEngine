#include <Ice/Rendering/Material.h>
#include <JSON/json.h>
#include <Ice/Utils/FileUtil.h>
#include <iostream>
#include <Ice/Rendering/Texture.h>

using json = nlohmann::json;

Material::Material()
{
	Material::InitializeMaterial();
}

Material::Material(std::string path)
{
	materialPath = path;
	Material::InitializeMaterial();
}

void Material::InitializeMaterial()
{
	std::string jsonString = FileUtil::ReadFile(materialPath);

	try
	{
		// Parse the JSON string
		json jsonData = json::parse(jsonString);

		// Get the name of the material
		name = jsonData["Name"];
		
		// Get the texture path
		if (jsonData["Texture"] != "")
		{
			std::string texturePath = jsonData["Texture"];
			texture = new Texture(texturePath);
		}
		else
		{
			texture = new Texture();
		}

		// Get the shader path
		std::string shaderPath = jsonData["Shader"];
		shader = new Shader(shaderPath + ".vert", shaderPath + ".frag");

		// Get the color
		color = glm::vec3(jsonData["Color"][0], jsonData["Color"][1], jsonData["Color"][2]);
	}
	catch (json::parse_error& e)
	{
		// Handle Errors
		std::cout << "Failed to parse material file: " << materialPath << std::endl;
		std::cout << e.what() << std::endl;
	}
}


Material::~Material()
{
	// free the memory
	delete shader;
	delete texture;
}