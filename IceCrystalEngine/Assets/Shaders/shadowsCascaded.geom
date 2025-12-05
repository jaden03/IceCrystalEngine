#version 430 core

#define MAX_CASCADES 4

layout(triangles, invocations = MAX_CASCADES) in;
layout(triangle_strip, max_vertices = 3) out;

layout(std140, binding = 0) uniform DirectionalCascadeData {
    mat4 cascadeMatrices[MAX_CASCADES];
};

void main() {
    for (int i = 0; i < 3; ++i) {
        gl_Position = cascadeMatrices[gl_InvocationID] * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}