#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in mat3 TBN; //normal mapping
in vec3 InstanceColor;
in vec4 FragPosLightSpace;

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

struct Flashlight {
    vec3 position;
    vec3 direction;
    float cutOff;      // Cosinus kąta wewnętrznego 
    float outerCutOff; // Cosinus kąta zewnętrznego (do krawedzi)
    
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};


// --- UNIFORMY ---
uniform Flashlight flashlight;
uniform PointLight lights[4];
uniform int activeLightsCount;
uniform vec3 dirLightDirection;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform bool useLighting;
uniform bool useBlinnPhong;
uniform bool isPointLight;
uniform bool bIsLightSource;    
uniform float uTiling = 1.0; //do heightmap albo po prostu tiling
uniform bool uUseEnvMap; //czy obj korzysta z env mapping
uniform sampler2D uNormalMap;
uniform bool uUseNormalMap; 
uniform float uNormalStrength;

uniform sampler2D uTextureSampler;
uniform bool bIsInstanced;
uniform bool bUseTexture;
uniform vec3 uColor;
uniform Material material;

uniform sampler2D tex_shadowMap;
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
//wersja 2

float ShadowCalculation(vec4 fragPosLightSpace, vec3 norm, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.z > 1.0) return 0.0;

    float bias = max(0.005 * (1.0 - dot(norm, lightDir)), 0.0005);
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(tex_shadowMap, 0); // Pobieramy rozmiar tekstury cienia
    
    for(int x = -2; x <= 2; ++x) {
        for(int y = -2; y <= 2; ++y) {
            float pcfDepth = texture(tex_shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    // Zwracamy srednią z 25 probek 
    return shadow / 25.0;
}
//latarkowe
vec3 CalcFlashlight(Flashlight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 baseColor) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Obliczanie kąta świecenia (theta to cosinus kąta między kierunkiem światła a kierunkiem latarki)
    float theta = dot(lightDir, normalize(-light.direction)); 
    
    // Obliczanie wygaszania na krawędziach (soft edges)
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // Obliczanie osłabienia z odległością (attenuation)
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Specular
    float spec = 0.0;
    if(useBlinnPhong) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    } else {
        vec3 reflectDir = reflect(-lightDir, norm);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }

    vec3 ambient = material.ambient * light.color * baseColor;
    vec3 diffuse = material.diffuse * diff * light.color * baseColor;
    vec3 specular = material.specular * spec * light.color;

    return (ambient + (diffuse + specular) * intensity) * attenuation;
}

void main()
{
    if(bIsLightSource) {
        FragColor = vec4(lightColor, 1.0);
        return;
    }

    // --- 1. LOGIKA KOLORU (ALBEDO) ---
    vec3 albedo;
    float finalAlpha = 1.0;
    vec2 finalUV = TexCoords * uTiling; 

    if(bUseTexture) {
        vec4 texColor = texture(uTextureSampler, finalUV);
        vec3 tint = bIsInstanced ? InstanceColor : vec3(1.0);
        albedo = texColor.rgb * tint;
        finalAlpha = texColor.a;
        if(finalAlpha < 0.1) discard; 
    } else {
        albedo = bIsInstanced ? InstanceColor : uColor;
    }

    // --- 2. NORMAL MAPPING ---
    vec3 norm;
    if(uUseNormalMap) {
        norm = texture(uNormalMap, finalUV).rgb;
        norm = normalize(norm * 2.0 - 1.0);
        norm.xy *= uNormalStrength;
        norm = normalize(TBN * norm);
    } else {
        norm = normalize(Normal);
    }

    vec3 viewDir = normalize(viewPos - FragPos);

    // --- 3. OŚWIETLENIE ---
    vec3 lightingResult = vec3(0.0);

    if(!useLighting) {
        lightingResult = albedo;
    } else {
        // Obliczamy oświetlenie zależnie od trybu
        if(!isPointLight) {
            // SŁOŃCE (Directional)
            vec3 lightDir = normalize(-dirLightDirection);
            float shadow = ShadowCalculation(FragPosLightSpace, norm, lightDir);
            
            float lightIntensity = 0.8; 
            vec3 effColor = lightColor * lightIntensity;
            vec3 ambient = material.ambient * effColor * albedo;
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = (material.diffuse * diff) * effColor * albedo;

            float spec = 0.0;
            if(useBlinnPhong) {
                vec3 halfwayDir = normalize(lightDir + viewDir);
                spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
            } else {
                vec3 reflectDir = reflect(-lightDir, norm);
                spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
            }
            lightingResult = ambient + (1.0 - shadow) * (diffuse + (material.specular * spec) * effColor);
        } else {
            // LATARKA + PUNKTOWE
            lightingResult += CalcFlashlight(flashlight, norm, FragPos, viewDir, albedo);
            for(int i = 0; i < activeLightsCount; i++) {
                lightingResult += CalcPointLight(lights[i], norm, FragPos, viewDir, albedo);
            }
        }
    }

    // --- 4. ENVIRONMENT MAPPING ---
    if(uUseEnvMap) {
        vec3 I = normalize(FragPos - viewPos);
        vec3 R = reflect(I, norm);
        vec3 envColor = texture(tex_skybox, R).rgb;
        
        vec3 reflections = envColor * reflectionFactor;
        lightingResult = mix(lightingResult, reflections, reflectionFactor);
    }

    // --- 5. MGŁA ---
    float dist = length(viewPos - FragPos);
    float fogFactor = clamp((dist - 60.0) / (120.0 - 60.0), 0.0, 1.0);
    vec3 fogColor = vec3(0.7, 0.75, 0.8); 
    
    FragColor = vec4(mix(lightingResult, fogColor, fogFactor), finalAlpha);
}