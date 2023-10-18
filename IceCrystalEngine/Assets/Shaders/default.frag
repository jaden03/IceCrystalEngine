#version 330 core
out vec4 FragColor;

in vec2 fragUV;

uniform sampler2D fragTexture;
uniform vec3 fragColor;

// lighting
uniform float ambientLightStrength;
uniform vec3 ambientLightColor;

void main()
{
	vec3 ambientLighting = ambientLightStrength * ambientLightColor;

	vec4 texel = texture(fragTexture, fragUV) * vec4(fragColor, 1.0);
	vec3 lighting = ambientLighting * texel.rgb;

	FragColor = vec4(lighting, texel.a);
}