#version 330 core
in vec2 TexCoord;

uniform sampler2D fragTexture;
uniform float time;

out vec4 FragColor;

void main()
{
    FragColor = texture(fragTexture, TexCoord);
    
//    // Base texture sampling
//    vec2 uv = TexCoord;
//
//    // Add wavy distortion
//    uv.x += sin(uv.y * 10.0 + time) * 0.05;
//    uv.y += cos(uv.x * 10.0 + time) * 0.05;
//
//    // Add swirling motion
//    vec2 centeredUV = uv - 0.5; // Center UVs around (0.5, 0.5)
//    float angle = atan(centeredUV.y, centeredUV.x) + sin(time) * 2.0;
//    float radius = length(centeredUV);
//    centeredUV.x = cos(angle) * radius;
//    centeredUV.y = sin(angle) * radius;
//    uv = centeredUV + 0.5; // Restore to original UV space
//
//    // Apply color shifting
//    vec4 color = texture(fragTexture, uv);
//    color.rgb = color.rgb * 0.5 + 0.5 * vec3(
//    sin(time + 0.0),
//    sin(time + 2.0),
//    sin(time + 4.0)
//    );
//
//    // Add pulsating brightness modulation
//    float brightness = 0.5 + 0.5 * sin(time * 5.0);
//    color.rgb *= brightness;
//
//    // Add radial pulse
//    vec2 radialCenter = TexCoord - 0.5;
//    float radialDist = length(radialCenter) * 8.0;
//    float radialEffect = 0.5 + 0.5 * sin(radialDist - time * 4.0);
//    color.rgb *= radialEffect;
//
//    // Add glitchy chromatic aberration
//    float offset = 0.01 * sin(time * 10.0);
//    vec4 r = texture(fragTexture, uv + vec2(offset, 0.0));
//    vec4 g = texture(fragTexture, uv);
//    vec4 b = texture(fragTexture, uv - vec2(offset, 0.0));
//    vec4 chromatic = vec4(r.r, g.g, b.b, 1.0);
//    color.rgb = mix(color.rgb, chromatic.rgb, 0.5);
//
//    // Add a transparent checkerboard pattern
//    vec2 checkerUV = TexCoord * 20.0; // Scale up the UVs for the checkerboard
//    float checker = mod(floor(checkerUV.x) + floor(checkerUV.y) + floor(time * 2.0), 2.0);
//    float checkerAlpha = 0.3; // Transparency for the checkerboard
//    vec3 checkerColor = mix(vec3(1.0), vec3(1.0, 0.0, 0.0), checker); // Checker color (red)
//
//    // Blend checkerboard overlay with the current color
//    color.rgb = mix(color.rgb, checkerColor, checker * checkerAlpha);
//
//    // Add moving transparent stripes
//    float stripeWidth = 0.1; // Width of each stripe
//    float stripeSpeed = 2.0; // Speed of stripe movement
//    float stripe = smoothstep(0.0, stripeWidth, sin((TexCoord.y + time * stripeSpeed) * 10.0));
//    float stripeTransparency = 1.0 - stripe; // Inverse stripe for transparency effect
//
//    // Apply stripe transparency to the entire quad
//    color.a = stripeTransparency;

    // Final output color
//    FragColor = color;
}
