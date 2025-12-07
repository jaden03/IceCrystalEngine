#pragma once

#include <Ice/Components/Physics/Collider.h>

#include "Ice/Rendering/MeshHolder.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"

class MeshCollider : public Collider
{
public:
    MeshCollider(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const glm::vec3 scale)
    {
        JPH::TriangleList triangles;
        triangles.reserve(indices.size() / 3);
        
        // Convert indices to triangles
        for (size_t i = 0; i < indices.size(); i += 3)
        {
            // Each index points directly to a Vertex struct
            uint32_t idx0 = indices[i];
            uint32_t idx1 = indices[i + 1];
            uint32_t idx2 = indices[i + 2];
            
            JPH::Triangle triangle;
            // Apply scale to vertex positions (x, y, z members of Vertex)
            triangle.mV[0] = JPH::Float3(
                vertices[idx0].x * scale.x, 
                vertices[idx0].y * scale.y, 
                vertices[idx0].z * scale.z
            );
            triangle.mV[1] = JPH::Float3(
                vertices[idx1].x * scale.x, 
                vertices[idx1].y * scale.y, 
                vertices[idx1].z * scale.z
            );
            triangle.mV[2] = JPH::Float3(
                vertices[idx2].x * scale.x, 
                vertices[idx2].y * scale.y, 
                vertices[idx2].z * scale.z
            );
            triangles.push_back(triangle);
        }

        JPH::MeshShapeSettings settings(triangles);
        
        settings.SetEmbedded();
        JPH::ShapeSettings::ShapeResult result = settings.Create();
        shape = result.Get();
    }
    
    JPH::ShapeRefC GetShape() const override { return shape; }

private:
    JPH::ShapeRefC shape;
};
