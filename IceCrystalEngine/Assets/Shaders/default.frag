#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 fragUV;
in vec3 fragNormal;
in vec3 fragPos;

uniform sampler2D fragTexture;
uniform vec3 fragColor;
uniform vec3 viewPos;

// lighting
uniform float ambientLightStrength;
uniform vec3 ambientLightColor;

#define MAX_POINT_LIGHTS 128
#define MAX_DIRECTIONAL_LIGHTS 5

uniform int directionalLightCount;
uniform struct DirectionalLight {
    vec3 direction;
	vec3 color;
	float strength;
    mat4 lightSpaceMatrix;
    bool castShadows;
} directionalLights[MAX_DIRECTIONAL_LIGHTS];
uniform sampler2D directionalShadowMap[MAX_DIRECTIONAL_LIGHTS];


uniform int pointLightCount;
uniform struct PointLight {
    vec3 position;
	vec3 color;
	float strength;
    float radius;
} pointLights[MAX_POINT_LIGHTS];;


void main()
{
	vec3 ambientLighting = ambientLightStrength * ambientLightColor;

	vec4 objectColor = texture(fragTexture, fragUV);
    
	vec3 lighting = vec3(0.0);

    vec3 normal = normalize(fragNormal);

    vec3 viewDir = normalize(viewPos - fragPos);

	 // Directional Lights
    for (int i = 0; i < directionalLights.length(); i++)
    {
        DirectionalLight light = directionalLights[i];
		
		vec3 lightDir = normalize(-light.direction);
        vec3 color = light.color;
        float strength = light.strength;

        float diff = max(dot(normal, lightDir), 0.0);

        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

        vec3 diffuse = diff * color;
        vec3 specular = spec * color;


        // Shadow
        float shadow = 0.0;

        if (light.castShadows)
        {
            vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fragPos, 1.0);

            vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
            projCoords = projCoords * 0.5 + 0.5;

            float closestDepth = texture(directionalShadowMap[i], projCoords.xy).r;
            float currentDepth = projCoords.z;

            float bias = 0.0001;
           
            vec2 texelSize = 1.0 / textureSize(directionalShadowMap[i], 0);
            float shadowSum = 0.0;

            for (int x = -2; x <= 2; ++x)
            {
                for (int y = -2; y <= 2; ++y)
                {
                    float pcfDepth = texture(directionalShadowMap[i], projCoords.xy + vec2(x, y) * texelSize).r;
                    shadowSum += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
                }
            }

            shadow = shadowSum / 25.0;

            if (projCoords.z > 1.0)
                shadow = 0.0;
        }
        
        lighting += (diffuse + specular) * strength * (1.0 - shadow);
    }

    // Point Lights
    for (int i = 0; i < pointLights.length(); i++)
    {
        PointLight light = pointLights[i];

        vec3 lightDir = normalize(light.position - fragPos);
        vec3 color = light.color;
        float strength = light.strength;
        float radius = light.radius;

        float diff = max(dot(normal, lightDir), 0.0);

        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

        float distance = length(light.position - fragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

        // Apply the attenuation based on radius
        attenuation *= clamp(1.0 - (distance / radius), 0.0, 1.0);

        vec3 diffuse = diff * color;
        vec3 specular = spec * color;

        lighting += (diffuse + specular) * strength * attenuation;
    }

    // this is a hack to make sure the light will always be visible (even if it should realistically be absorbed by the object)
    vec3 endColor = fragColor;
    if (endColor.r == 0.0)
        endColor.r = 0.1;
    if (endColor.g == 0.0)
        endColor.g = 0.1;
    if (endColor.b == 0.0)
        endColor.b = 0.1;
    if (objectColor.r == 0.0)
        objectColor.r = 0.1;
    if (objectColor.g == 0.0)
        objectColor.g = 0.1;
    if (objectColor.b == 0.0)
        objectColor.b = 0.1;

    vec4 result = vec4((ambientLighting + lighting), 1.0) * objectColor * vec4(endColor, 1.0);
	FragColor = result;

    // this calculates bloom
    float brightness = length(FragColor.rgb);
    if (brightness > 3)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}