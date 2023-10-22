#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrColorBuffer;
uniform float exposure;

void main()
{
    const float gamma = 1.2;
    vec3 hdrColor = texture(hdrColorBuffer, TexCoords).rgb;

    // this is reinhard tone mapping, it essentially scales the image's color to the range [0, 1]
    // vec3 result = hdrColor / (hdrColor + vec3(1.0));

    // this is exposure, basically its how your eyes adjust to the light, the higher the exposure the brighter the image
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

    // also gamma correction is applied to ensure the image is displayed correctly on the screen
    result = pow(result, vec3(1.0 / gamma));

    FragColor = vec4(result, 1.0);
}