#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;

out vec2 fragUV;
out vec3 fragPos;

layout(std140, binding = 0) uniform GlobalData
{
	mat4 view;
	mat4 projection;
	vec3 viewPos;
	float time;
	float nearPlane;
	float farPlane;
	float gd_padding0;
	float gd_padding1;
};
uniform mat4 model;

void main()
{
	fragUV = aUV;
	fragPos = vec3(model * vec4(aPos, 1.0));
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}