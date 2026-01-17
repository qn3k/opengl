#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct Material {
    float ambient;
    float diffuse;
    float specular;
    float shininess;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

// --- UNIFORMY ---
uniform PointLight lights[4];
uniform int activeLightsCount;
uniform vec3 dirLightDirection;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform bool useLighting;
uniform bool useBlinnPhong;
uniform bool isPointLight;
uniform bool bIsLightSource;

uniform sampler2D uTextureSampler;
uniform bool bUseTexture;
uniform vec3 uColor;
uniform Material material;

// NOWE UNIFORMY DLA ODBIĆ
uniform samplerCube tex_skybox;
uniform float reflectionFactor; // 0.0 = brak odbić, 1.0 = czyste lustro

vec3 CalcPointLight(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 baseColor) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (1.0 + 0.07 * distance + 0.017 * (distance * distance));
    
    float diff = max(dot(norm, lightDir), 0.0);
    
    float spec = 0.0;
    if(useBlinnPhong) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    } else {
        vec3 reflectDir = reflect(-lightDir, norm);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    
    vec3 ambient = material.ambient * light.color * light.intensity * baseColor;
    vec3 diffuse = (material.diffuse * diff) * light.color * light.intensity * baseColor;
    vec3 specular = (material.specular * spec) * light.color * light.intensity;
    
    return (ambient + diffuse + specular) * attenuation;
}

void main()
{
    if(bIsLightSource) {
        FragColor = vec4(lightColor, 1.0);
        return;
    }

    vec4 baseColor = bUseTexture ? texture(uTextureSampler, TexCoords) : vec4(uColor, 1.0);
    if(baseColor.a < 0.1) discard;

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // --- OBLICZANIE ODBICIA (Environment Mapping) ---
    vec3 I = normalize(FragPos - viewPos);
    vec3 R = reflect(I, norm);
    vec3 envColor = texture(tex_skybox, R).rgb;

    if(!useLighting) {
        // Nawet bez świateł możemy chcieć widzieć odbicia (efekt chromu)
        vec3 unlitResult = mix(baseColor.rgb, envColor, reflectionFactor);
        FragColor = vec4(unlitResult, baseColor.a);
        return;
    }

    vec3 result = vec3(0.0);

    if(!isPointLight) {
        // TRYB KIERUNKOWY
        float lightIntensity = 0.8; 
        vec3 effectiveLightColor = lightColor * lightIntensity;
        vec3 lightDir = normalize(-dirLightDirection);

        vec3 ambient = material.ambient * effectiveLightColor * baseColor.rgb;
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = (material.diffuse * diff) * effectiveLightColor * baseColor.rgb;

        float spec = 0.0;
        if(useBlinnPhong) {
            vec3 halfwayDir = normalize(lightDir + viewDir);
            spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
        } else {
            vec3 reflectDir = reflect(-lightDir, norm);
            spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        }
        vec3 specular = (material.specular * spec) * effectiveLightColor;

        result = ambient + diffuse + specular;
    } 
    else {
        // TRYB PUNKTOWY
        for(int i = 0; i < activeLightsCount; i++) {
            result += CalcPointLight(lights[i], norm, FragPos, viewDir, baseColor.rgb);
        }
    }

    // --- FINALNY MIX: Światło + Odbicie ---
    // Mieszamy wynik oświetlenia Phonga z kolorem ze skyboxa
    vec3 finalColor = mix(result, envColor, reflectionFactor);

    FragColor = vec4(finalColor, baseColor.a);
}