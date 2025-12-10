#version 430 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 TexCoords;

uniform samplerCube fragTexture;

void main()
{    
    FragColor = texture(fragTexture, TexCoords);
    BrightColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}