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
uniform float uTiling = 1.0; //do heightmap
uniform bool uUseEnvMap; //czy obj korzysta z env mapping

uniform sampler2D uTextureSampler;
uniform bool bUseTexture;
uniform vec3 uColor;
uniform Material material;

in vec4 FragPosLightSpace;
uniform sampler2D tex_shadowMap;

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

float ShadowCalculation(vec4 fragPosLightSpace, vec3 norm, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.z > 1.0) return 0.0;

    float closestDepth = texture(tex_shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;

    // BIAS
    float bias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005);
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

void main()
{
    if(bIsLightSource) {
        FragColor = vec4(lightColor, 1.0);
        return;
    }

    //heightmap
    vec2 finalUV = TexCoords * uTiling; 
    
    vec4 baseColor = bUseTexture ? texture(uTextureSampler, finalUV) : vec4(uColor, 1.0);
    
    if(baseColor.a < 0.1) discard;

    vec4 texColor = texture(uTextureSampler, TexCoords);
    
    if(texColor.a < 0.5) {
        discard;
    }
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
        // TRYB KIERUNKOWY (Z CIENIAMI)
        vec3 lightDir = normalize(-dirLightDirection);
        
        // 1. Obliczamy cień
        float shadow = ShadowCalculation(FragPosLightSpace, norm, lightDir);  

        float lightIntensity = 0.8; 
        vec3 effectiveLightColor = lightColor * lightIntensity;

        // 2. Ambient - obliczany zawsze, niezależnie od cienia
        vec3 ambient = material.ambient * effectiveLightColor * baseColor.rgb;
        
        // 3. Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = (material.diffuse * diff) * effectiveLightColor * baseColor.rgb;

        // 4. Specular
        float spec = 0.0;
        if(useBlinnPhong) {
            vec3 halfwayDir = normalize(lightDir + viewDir);
            spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
        } else {
            vec3 reflectDir = reflect(-lightDir, norm);
            spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        }
        vec3 specular = (material.specular * spec) * effectiveLightColor;

        // KLUCZOWA ZMIANA: Ambient jest poza nawiasem cienia!
        result = ambient + (1.0 - shadow) * (diffuse + specular);
    }
    
    else {
        // TRYB PUNKTOWY
        for(int i = 0; i < activeLightsCount; i++) {
            result += CalcPointLight(lights[i], norm, FragPos, viewDir, baseColor.rgb);
        }
    }

    // --- FINALNY MIX: Światło + Odbicie ---
    vec3 mixedColor = result;

    // Tylko jeśli flaga jest włączona ORAZ mamy jakiś współczynnik odbicia
    if(uUseEnvMap && reflectionFactor > 0.0) {
        mixedColor = mix(result, envColor, reflectionFactor);
    }

    // --- DODAWANIE MGŁY ---
    float dist = length(viewPos - FragPos); // Odległość od kamery
    
    // Parametry mgły: 
    float fogStart = 60.0;
    float fogEnd = 120.0;
    
    // Obliczamy współczynnik mgły 
    float fogFactor = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
    
    // Kolor mgły 
    vec3 fogColor = vec3(0.7, 0.75, 0.8); 
    
    vec3 colorWithFog = mix(mixedColor, fogColor, fogFactor);

    FragColor = vec4(colorWithFog, baseColor.a);
}