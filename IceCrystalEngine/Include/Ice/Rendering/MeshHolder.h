#pragma once
#ifndef MESH_HOLDER_H

#define MESH_HOLDER_H

#include <glad/glad.h>
#include <vector>

struct Vertex {
	GLfloat x, y, z;       // position
	GLfloat u, v;          // texture coordinates
	GLfloat nx, ny, nz;    // normal
};

struct MeshHolder {
	std::vector<Vertex> vertices;  // Interleaved vertex data
	std::vector<unsigned int> indices;
    
	GLuint vertexArrayObject;
	GLuint vertexBufferObject;
	GLuint elementBufferObject;
    
	MeshHolder() = default;
	MeshHolder(std::vector<Vertex>&& v, std::vector<unsigned int>&& i)
		: vertices(std::move(v)), indices(std::move(i)) {}
};

#endif