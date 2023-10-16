#pragma once
#ifndef MESH_HOLDER_H

#define MESH_HOLDER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

class MeshHolder
{
	
public:
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> uvs;
	std::vector<unsigned int> indices;

	MeshHolder(std::vector<GLfloat> vertices, std::vector<GLfloat> uvs, std::vector<unsigned int> indices)
	{
		this->vertices = vertices;
		this->uvs = uvs;
		this->indices = indices;
	}
	
};

#endif