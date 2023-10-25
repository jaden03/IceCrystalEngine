#version 330 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D colorBuffer;
uniform sampler2D bloomBuffer;

uniform float exposure;
uniform bool bloom;

void main()
{
    const float gamma = 1.2;
    vec3 hdrColor = texture(colorBuffer, TexCoords).rgb;

    // bloom
    if (bloom)
    {
        vec3 bloomColor = texture(bloomBuffer, TexCoords).rgb;
        hdrColor += bloomColor;
    }

    // this is exposure, basically its how your eyes adjust to the light, the higher the exposure the brighter the image
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

    // also gamma correction is applied to ensure the image is displayed correctly on the screen
    result = pow(result, vec3(1.0 / gamma));

    FragColor = vec4(result, 1.0);
}