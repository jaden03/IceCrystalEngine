#pragma once // this prevents multiple definitions of the class

// Header Guard
#ifndef MATERIAL_H // this is a "header guard", it is used to prevent the same header file from being included more than once, basically if "#define MATERIAL_H" is already somewhere else
// it will prevent the code between "#ifndef MATERIAL_H" and "#endif" from being included again

// Definition of the class
#define MATERIAL_H

#include <string> // this just includes the string library from the standard library
#include "Shader.h"
#include "Texture.h"
#include <glad/glad.h>

class Material // this is the class definition
{
	
	void InitializeMaterial();

public:
	
	std::string materialPath = "{PROJECT_ROOT}Assets/Materials/default.mat";
	std::string name = "Default Material"; // the name of the material

	Texture* texture; // the texture of the material
	Shader* shader; // the shader the material uses

	glm::vec3 color;

	Material(); // constructs a default material
	Material(std::string path); // constructs a material using a .mat file

	~Material(); // deconstructor
	
};

#endif // this line ends the header guard at the top