#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;

out vec2 fragUV;
out vec3 fragNormal;
out vec3 fragPos;

layout(std140, binding = 0) uniform GlobalData
{
	mat4 view;
	mat4 projection;
	vec3 viewPos;
	float time;
	float nearPlane;
	float farPlane;
	float _padding0;
	float _padding1;
};

uniform mat4 model;
uniform mat3 normalModel;

void main()
{
	fragUV = aUV;
	fragNormal = normalize(normalModel * aNormal);
	fragPos = vec3(model * vec4(aPos, 1.0));
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}