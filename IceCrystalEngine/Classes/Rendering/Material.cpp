#include <Ice/Rendering/Material.h>

#include <iostream>
#include <JSON/json.h>
#include <Ice/Utils/FileUtil.h>

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
		shader = new Shader(shaderPath);

		// Get the color
		color = glm::vec3(jsonData["Color"][0], jsonData["Color"][1], jsonData["Color"][2]);
		smoothness = jsonData.value("Smoothness", 0.5f);

		// Parse custom properties
		for (auto& [key, value] : jsonData.items())
		{
			// Skip known properties
			if (key == "Name" || key == "Texture" || key == "Shader" || key == "Color" || key == "Smoothness")
				continue;

			// Detect type and store accordingly
			if (value.is_number_float())
			{
				floatProperties[key] = value.get<float>();
			}
			else if (value.is_number_integer())
			{
				intProperties[key] = value.get<int>();
			}
			else if (value.is_array() && value.size() == 2)
			{
				vec2Properties[key] = glm::vec2(value[0], value[1]);
			}
			else if (value.is_array() && value.size() == 3)
			{
				vec3Properties[key] = glm::vec3(value[0], value[1], value[2]);
			}
			else if (value.is_array() && value.size() == 4)
			{
				vec4Properties[key] = glm::vec4(value[0], value[1], value[2], value[3]);
			}
		}
		
	}
	catch (json::parse_error& e)
	{
		// Handle Errors
		std::cout << "Failed to parse material file: " << materialPath << std::endl;
		std::cout << e.what() << std::endl;
	}
}

void Material::ApplyProperties()
{
	// Apply all stored properties to the shader
	for (const auto& [name, value] : floatProperties)
	{
		shader->setFloat(name, value);
	}
    
	for (const auto& [name, value] : intProperties)
	{
		shader->setInt(name, value);
	}

	for (const auto& [name, value] : vec2Properties)
	{
		shader->setVec2(name, value);
	}
    
	for (const auto& [name, value] : vec3Properties)
	{
		shader->setVec3(name, value);
	}
    
	for (const auto& [name, value] : vec4Properties)
	{
		shader->setVec4(name, value);
	}
}


Material::~Material()
{
	// free the memory
	delete shader;
	delete texture;
}