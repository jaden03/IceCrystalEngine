#version 430 core

in vec2 TexCoord;

uniform sampler2DArray fragTexture;

out vec4 FragColor;

void main()
{
    // sample from layer 0
    FragColor = texture(fragTexture, vec3(TexCoord, 0.0));
}
