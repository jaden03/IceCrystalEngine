// fragment.glsl  (exactly your layout, only uses the new color)
#version 430 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;
layout(location = 2) out vec4 PickColor;

in vec3 fColor;
in vec2 fUV;

uniform sampler2D fragTexture;
uniform vec3 fragColor;      // you can still use this if you want
uniform vec3 uniqueColor;

void main()
{
    vec4 tex = texture(fragTexture, fUV);

    // use normal color from geometry shader (very obvious)
    vec3 final = fColor;

    // optional: multiply with your texture or your fragColor uniform
    // final = final * tex.rgb * fragColor;

    FragColor = vec4(final, 1.0);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    BrightColor = brightness > 1.0 ? vec4(FragColor.rgb, 1.0) : vec4(0.0);

    PickColor = vec4(uniqueColor, 1.0);
}