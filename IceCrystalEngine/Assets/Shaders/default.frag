#version 330 core
out vec4 FragColor;

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

        vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fragPos, 1.0);
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        projCoords.xy = fragPosLightSpace.xy * 0.5 + 0.5;
        float closestDepth = texture(directionalShadowMap[i], projCoords.xy).r;
        float currentDepth = projCoords.z;
        float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

        if (projCoords.z > 1.0)
            shadow = 0.0;
        
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

    vec4 result = vec4((ambientLighting + lighting), 1.0) * objectColor * vec4(fragColor, 1.0);
	FragColor = result;
}