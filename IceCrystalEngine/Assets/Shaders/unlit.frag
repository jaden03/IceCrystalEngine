#version 330 core
out vec4 FragColor;

in vec2 fragUV;
in vec3 fragPos;

uniform sampler2D fragTexture;
uniform vec3 fragColor;

void main()
{
	vec4 objectColor = texture(fragTexture, fragUV);
    
    vec4 result = objectColor * vec4(fragColor, 1.0f);
	FragColor = result;
}