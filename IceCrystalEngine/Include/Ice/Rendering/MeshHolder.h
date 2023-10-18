#pragma once
#ifndef MESH_HOLDER_H

#define MESH_HOLDER_H

#include <glad/glad.h>
#include <vector>

class MeshHolder
{
	
public:
	
	unsigned int vertexArrayObject;

	unsigned int vertexBufferObject;
	unsigned int uvBufferObject;
	unsigned int elementBufferObject;

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