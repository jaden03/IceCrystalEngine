#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 fragUV;
in vec3 fragPos;

uniform sampler2D fragTexture;
uniform vec3 fragColor;

void main()
{
	vec4 objectColor = texture(fragTexture, fragUV);
    
    vec4 result = objectColor * vec4(fragColor, 1.0f);
	FragColor = result;

	// this calculates bloom
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}