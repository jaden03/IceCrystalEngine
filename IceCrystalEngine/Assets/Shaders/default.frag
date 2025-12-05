#version 430 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
layout (location = 2) out vec4 PickColor;

in vec2 fragUV;
in vec3 fragNormal;
in vec3 fragPos;

layout(std140, binding = 0) uniform GlobalData
{
    mat4 view;
    mat4 projection;
    vec3 viewPos;
    float time;
    float nearPlane;
    float farPlane;
    float _padding0;
    float _padding1;
};

// poor mans raycasting
uniform vec3 uniqueColor;

uniform sampler2D fragTexture;
uniform vec3 fragColor;

// lighting
uniform float ambientLightStrength;
uniform vec3 ambientLightColor;

#define MAX_POINT_LIGHTS 64
#define MAX_SPOT_LIGHTS 16
#define MAX_CASCADES 4

uniform bool directionalLightExists;
uniform struct DirectionalLight {
    vec3 direction;
	vec3 color;
	float strength;
    bool castShadows;
    int cascadeCount;
    float cascadeSplits[MAX_CASCADES];
} directionalLight;
layout(std140, binding = 2) uniform DirectionalCascadeData {
    mat4 cascadeMatrices[MAX_CASCADES];
} uboCascade;
uniform sampler2DArray directionalShadowMap;


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
    
    
    // Directional Light
    // -------------------------------------------------
    // 1. Base lighting (diffuse + specular)
    // -------------------------------------------------
    vec3 lightDir = normalize(-directionalLight.direction);
    vec3 color = directionalLight.color;
    float strength = directionalLight.strength;

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 diffuse = diff * color;
    vec3 specular = spec * color;

    // =================================================
    // SHADOW CALCULATION
    // =================================================
    float shadow = 0.0;

    if (directionalLight.castShadows)
    {
        // ---------------------------------------------
        // 2. Choose cascade based on view-space depth
        // ---------------------------------------------
        vec4 fragPosViewSpace = view * vec4(fragPos, 1.0);
        float depthValue = abs(fragPosViewSpace.z);
        
        int cascade = -1;
        for (int c = 0; c < directionalLight.cascadeCount; c++)
        {
            if (depthValue < directionalLight.cascadeSplits[c])
            {
                cascade = c;
                break;
            }
        }
        if (cascade == -1)
        {
            cascade = directionalLight.cascadeCount;
        }

        // ---------------------------------------------
        // SET COLOR AND RETURN BASED ON CASCADE INDEX
        // ---------------------------------------------
//        if (cascade == 0)
//        {
//            FragColor = vec4(1, 0, 0, 1);   // red
//            return;
//        }
//        else if (cascade == 1)
//        {
//            FragColor = vec4(0, 1, 0, 1);   // green
//            return;
//        }
//        else if (cascade == 2)
//        {
//            FragColor = vec4(0, 0, 1, 1);   // blue
//            return;
//        }
//        else if (cascade == 3)
//        {
//            FragColor = vec4(1, 1, 0, 1);   // yellow
//            return;
//        }
//
//        // fallback in case extra cascades exist
//        FragColor = vec4(1, 0, 1, 1);       // magenta
//        return;

        // ---------------------------------------------
        // 3. Transform fragment into light space
        // ---------------------------------------------
        vec4 fragPosLightSpace = uboCascade.cascadeMatrices[cascade] * vec4(fragPos, 1.0);
        vec3 proj = fragPosLightSpace.xyz / fragPosLightSpace.w;
        proj = proj * 0.5 + 0.5;
        
        // -----------------------------------------
        // 4. Check if inside shadow map bounds
        // -----------------------------------------
        if (proj.z < 1.0)
        {
            // -------------------------------------
            // 5. Bias (reduces shadow acne)
            // -------------------------------------
//            vec3 normal = normalize(fragNormal);
//            float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
//            const float biasModifier = 0.5f;
//            if (cascade == directionalLight.cascadeCount)
//            {
//                bias *= 1 / (farPlane * biasModifier);
//            }
//            else
//            {
//                bias *= 1 / (directionalLight.cascadeSplits[cascade] * biasModifier);
//            }


            // -------------------------------------
            // 6. PCF Sampling
            // -------------------------------------
            ivec3 size = textureSize(directionalShadowMap, 0);
            vec2 texelSize = 1.0 / vec2(size.xy);

            float shadowSum = 0.0;
            for (int x = -2; x <= 2; x++)
            {
                for (int y = -2; y <= 2; y++)
                {
                    float sampleDepth = texture(directionalShadowMap, vec3(proj.xy + vec2(x, y) * texelSize, cascade)).r;
//                    shadowSum += (proj.z - bias) > sampleDepth ? 1.0 : 0.0;
                    shadowSum += proj.z > sampleDepth ? 1.0 : 0.0;
                }
            }
            shadow = shadowSum / 25.0;
        }
    }
    // =================================================
    // Final contribution from the directional light
    // =================================================
    lighting += (diffuse + specular) * strength * (1.0 - shadow);
    

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