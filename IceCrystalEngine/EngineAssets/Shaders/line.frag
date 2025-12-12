#version 430 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
layout (location = 2) out vec4 PickColor;

in vec3 fragColor;

void main()
{
    FragColor = vec4(fragColor, 1.0);
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    PickColor = vec4(0.0, 0.0, 0.0, 1.0);
}