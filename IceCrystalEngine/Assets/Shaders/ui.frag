#version 330 core
in vec2 TexCoord;

uniform sampler2D tex;

out vec4 FragColor;

void main()
{
//    FragColor = texture(tex, TexCoord);
    FragColor = vec4(0.0, 0.0, 1.0, 1.0);
}