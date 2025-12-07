#pragma once

#include <Ice/Components/Physics/Collider.h>
#include "Jolt/Physics/Collision/Shape/MeshShape.h"

class MeshCollider : public Collider
{
public:
    MeshCollider(const std::vector<float> &vertices, const std::vector<uint32_t> &indices, const glm::vec3 scale)
    {
        JPH::TriangleList triangles;
        
        // Convert indices to triangles
        for (size_t i = 0; i < indices.size(); i += 3)
        {
            // Each index points to a vertex, and each vertex is 3 floats (x, y, z)
            uint32_t idx0 = indices[i] * 3;
            uint32_t idx1 = indices[i + 1] * 3;
            uint32_t idx2 = indices[i + 2] * 3;
            
            JPH::Triangle triangle;
            // Apply scale to vertices
            triangle.mV[0] = JPH::Float3(vertices[idx0] * scale.x, vertices[idx0 + 1] * scale.y, vertices[idx0 + 2] * scale.z);
            triangle.mV[1] = JPH::Float3(vertices[idx1] * scale.x, vertices[idx1 + 1] * scale.y, vertices[idx1 + 2] * scale.z);
            triangle.mV[2] = JPH::Float3(vertices[idx2] * scale.x, vertices[idx2 + 1] * scale.y, vertices[idx2 + 2] * scale.z);
            triangles.push_back(triangle);
        }

        JPH::MeshShapeSettings settings(triangles);
        
        settings.SetEmbedded();
        JPH::ShapeSettings::ShapeResult result = settings.Create();
        shape = result.Get(); // now shape is valid
    }
    
    JPH::ShapeRefC GetShape() const override { return shape; }

private:
    JPH::ShapeRefC shape;
};
