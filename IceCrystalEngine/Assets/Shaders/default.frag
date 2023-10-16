#version 330 core
out vec4 FragColor;

in vec2 fragUV;

uniform sampler2D fragTexture;
uniform vec3 fragColor;

void main()
{
	FragColor = texture(fragTexture, fragUV) * vec4(fragColor, 1.0);
}