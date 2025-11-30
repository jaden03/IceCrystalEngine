#version 430 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
layout (location = 2) out vec4 PickColor;

in vec2 fragUV;
in vec3 fragPos;

// poor mans raycasting
uniform vec3 uniqueColor;

uniform sampler2D fragTexture;
uniform vec3 fragColor;

void main()
{
	vec4 objectColor = texture(fragTexture, fragUV);
    
    vec4 result = objectColor * vec4(fragColor, 1.0f);
	FragColor = result;

	// this calculates bloom
    float brightness = length(FragColor.rgb);
    if (brightness > 3)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

    // poor mans raycasting
    PickColor = vec4(uniqueColor, 1.0);
}