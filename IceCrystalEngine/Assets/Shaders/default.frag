#version 430 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
layout (location = 2) out vec4 PickColor;

in vec2 fragUV;
in vec3 fragNormal;
in vec3 fragPos;

// poor mans raycasting
uniform vec3 uniqueColor;

uniform sampler2D fragTexture;
uniform vec3 fragColor;
uniform vec3 viewPos;
uniform mat4 view;

// lighting
uniform float ambientLightStrength;
uniform vec3 ambientLightColor;

#define MAX_POINT_LIGHTS 64
#define MAX_SPOT_LIGHTS 16
#define MAX_DIRECTIONAL_LIGHTS 1
#define MAX_CASCADES 4

uniform int directionalLightCount;
uniform struct DirectionalLight {
    vec3 direction;
	vec3 color;
	float strength;
    bool castShadows;
    int cascadeCount;
    float cascadeSplits[MAX_CASCADES];
} directionalLights[MAX_DIRECTIONAL_LIGHTS];
layout(std140, binding = 0) uniform DirectionalCascadeData {
    mat4 cascadeMatrices[MAX_CASCADES];
} uboCascade;
uniform sampler2DArray directionalShadowMaps[MAX_DIRECTIONAL_LIGHTS];


uniform int pointLightCount;
uniform struct PointLight {
    vec3 position;
	vec3 color;
	float strength;
    float radius;
} pointLights[MAX_POINT_LIGHTS];;


uniform int spotLightCount;
uniform struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float strength;
    float distance;
    float angle;
    float outerAngle;
    mat4 lightSpaceMatrix;
    bool castShadows;
} spotLights[MAX_SPOT_LIGHTS];;
uniform sampler2D spotShadowMap[MAX_SPOT_LIGHTS];


void main()
{
	vec3 ambientLighting = ambientLightStrength * ambientLightColor;

	vec4 objectColor = texture(fragTexture, fragUV);
    
	vec3 lighting = vec3(0.0);

    vec3 normal = normalize(fragNormal);

    vec3 viewDir = normalize(viewPos - fragPos);
    
    // Directional Lights
    for (int i = 0; i < directionalLightCount; i++)
    {
        DirectionalLight light = directionalLights[i];

        // -------------------------------------------------
        // 1. Base lighting (diffuse + specular)
        // -------------------------------------------------
        vec3 lightDir = normalize(-light.direction);
        vec3 color = light.color;
        float strength = light.strength;

        float diff = max(dot(normal, lightDir), 0.0);

        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

        vec3 diffuse = diff * color;
        vec3 specular = spec * color;

        // =================================================
        // SHADOW CALCULATION
        // =================================================
        float shadow = 0.0;

        if (light.castShadows)
        {
            // ---------------------------------------------
            // 2. Choose cascade based on view-space depth
            // ---------------------------------------------
            float depthVS = length(viewPos - fragPos);

            int cascade = light.cascadeCount - 1;
            for (int c = 0; c < light.cascadeCount; c++)
            {
                if (depthVS < light.cascadeSplits[c])
                {
                    cascade = c;
                    break;
                }
            }

            // ---------------------------------------------
            // 3. Transform fragment into light space
            // ---------------------------------------------
            vec4 fragLS = uboCascade.cascadeMatrices[cascade] * vec4(fragPos, 1.0);
            vec3 proj = fragLS.xyz / fragLS.w;
            proj = proj * 0.5 + 0.5;

            // -----------------------------------------
            // 4. Check if inside shadow map bounds
            // -----------------------------------------
            if (proj.x >= 0.0 && proj.x <= 1.0 &&
                proj.y >= 0.0 && proj.y <= 1.0 &&
                proj.z >= 0.0 && proj.z <= 1.0)
            {
                // -------------------------------------
                // 5. Bias (reduces shadow acne)
                // -------------------------------------
                float ndotl = max(dot(normal, lightDir), 0.0);
                float bias = max(0.0005 * (1.0 - ndotl), 0.00005);

                // -------------------------------------
                // 6. PCF Sampling
                // -------------------------------------
                vec2 texelSize = 1.0 / vec2(textureSize(directionalShadowMaps[i], 0));
                float shadowSum = 0.0;

                for (int x = -1; x <= 1; x++)
                {
                    for (int y = -1; y <= 1; y++)
                    {
                        float sampleDepth = texture(directionalShadowMaps[i], vec3(proj.xy + vec2(x, y) * texelSize, cascade)).r;

                        shadowSum += (proj.z - bias > sampleDepth) ? 1.0 : 0.0;
                    }
                }

                shadow = shadowSum / 9.0;
            }

        }

        // =================================================
        // Final contribution from this directional light
        // =================================================
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

    // Spot Lights
    for (int i = 0; i < spotLights.length(); i++)
    {
        SpotLight light = spotLights[i];

        vec3 lightDir = normalize(light.position - fragPos);
        vec3 color = light.color;
        float strength = light.strength;
        float distance = light.distance;
        float angle = light.angle;
        float outerAngle = light.outerAngle;

        float diff = max(dot(normal, lightDir), 0.0);

        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

        float theta = dot(lightDir, normalize(-light.direction));

        float epsilon = angle - outerAngle;
        float intensity = clamp((theta - outerAngle) / (epsilon), 0.0, 1.0);

        float distanceFromLight = length(light.position - fragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distanceFromLight + 0.032 * (distanceFromLight * distanceFromLight));

        vec3 diffuse = diff * color;
        vec3 specular = spec * color;

        // Shadow
        float shadow = 0.0;
        
        if (light.castShadows)
        {
            vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fragPos, 1.0);

            vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
            projCoords = projCoords * 0.5 + 0.5;

            float closestDepth = texture(spotShadowMap[i], projCoords.xy).r;
            float currentDepth = projCoords.z;

            float bias = 0.0003;

            vec2 texelSize = 1.0 / textureSize(spotShadowMap[i], 0);
            float shadowSum = 0.0;

            for (int x = -2; x <= 2; ++x)
            {
                for (int y = -2; y <= 2; ++y)
                {
                    float pcfDepth = texture(spotShadowMap[i], projCoords.xy + vec2(x, y) * texelSize).r;
                    shadowSum += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
                }
            }

            shadow = shadowSum / 25.0;

            if (projCoords.z > 1.0)
                shadow = 0.0;
        }


        lighting += (diffuse + specular) * strength * intensity * attenuation * (1.0 - shadow);
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

    // poor mans raycasting
    // put uniqueColor into a range of 0-1
    PickColor = vec4(uniqueColor, 1.0);
}