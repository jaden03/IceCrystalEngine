#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube fragTexture;

void main()
{    
    FragColor = texture(fragTexture, TexCoords);
}