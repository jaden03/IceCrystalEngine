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


struct DirectionalLight {
    vec3 direction;
	vec3 color;
	float strength;
};

uniform int directionalLightCount;
uniform DirectionalLight directionalLights[5];


struct PointLight {
    vec3 position;
	vec3 color;
	float strength;
    float radius;
};

uniform int pointLightCount;
uniform PointLight pointLights[128];


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

        lighting += (diffuse + specular) * strength;
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