#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <iostream>

class MathUtils
{
public:
    // Blender to Engine Coordinate System Conversion
    // Blender: X right, Y forward, Z up
    static glm::vec3 BlenderToEngine(const glm::vec3& blenderPos)
    {
        return glm::vec3(
            blenderPos.x,
            blenderPos.z,
            -blenderPos.y
        );
    }
    
static glm::quat BlenderToEngineQuat(const glm::quat& b)
    {
        return glm::quat(
            b.w,
            b.x,
            b.z,
            -b.y
        );
    }
};
