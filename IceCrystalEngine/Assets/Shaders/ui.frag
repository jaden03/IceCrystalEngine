#version 430 core
in vec2 TexCoord;

uniform sampler2D fragTexture;

out vec4 FragColor;

void main()
{
    FragColor = texture(fragTexture, TexCoord);
//    FragColor = vec4(0.0, 0.0, 1.0, 1.0);
}