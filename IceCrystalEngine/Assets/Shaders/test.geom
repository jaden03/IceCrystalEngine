// geometry.glsl
#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 model;      // needed again here for correct MVP calculation
uniform mat4 view;
uniform mat4 projection;
uniform float time;

in vec3 vWorldPos[];
in vec2 vUV[];

out vec3 fColor;
out vec2 fUV;

void main()
{
    // face normal in world space
    vec3 a = vWorldPos[1] - vWorldPos[0];
    vec3 b = vWorldPos[2] - vWorldPos[0];
    vec3 faceNormal = normalize(cross(a, b));

    // classic normal visualization color
    vec3 normalColor = abs(faceNormal);

    // smooth breathe 0 → 1 → 0
    float breathe = (sin(time * 2.8) + 1.0) * 0.5;   // 0..1
    float offset  = breathe * 0.7;                   // max 0.7 units out

    for (int i = 0; i < 3; ++i)
    {
        // move vertex along face normal in world space
        vec3 newWorldPos = vWorldPos[i] + faceNormal * offset;

        // re-project the moved vertex so it stays correct from any camera angle
        gl_Position = projection * view * vec4(newWorldPos, 1.0);

        fColor = normalColor;
        fUV    = vUV[i];

        EmitVertex();
    }
    EndPrimitive();
}