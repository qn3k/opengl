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

// Struktura dla wielu świateł punktowych
struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

// --- UNIFORMY ---
uniform PointLight lights[4];    // Tablica świateł punktowych (N)
uniform int activeLightsCount;   // Liczba aktywnych świateł (1-4)
uniform vec3 dirLightDirection;  // Kierunek słońca (H)
uniform vec3 lightColor;         // Kolor bazowy (używany dla słońca i źródeł)
uniform vec3 viewPos;
uniform bool useLighting;
uniform bool useBlinnPhong;
uniform bool isPointLight;       // true = punktowe (jedno lub wiele), false = kierunkowe
uniform bool bIsLightSource;

uniform sampler2D uTextureSampler;
uniform bool bUseTexture;
uniform vec3 uColor;
uniform Material material;

// FUNKCJA OBLICZAJĄCA JEDNO ŚWIATŁO PUNKTOWE
vec3 CalcPointLight(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 baseColor) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Attenuation
    // Attenuation - coraz mniej swieci z odlegloscia, w kierunkowym swieci tak samo mocno wszedzie 
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (1.0 + 0.07 * distance + 0.017 * (distance * distance));
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Specular
    float spec = 0.0;
    if(useBlinnPhong) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    } else { //tutaj jest Phong
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
    // Jeśli to wizualizacja żarówki (G)
    if(bIsLightSource) {
        FragColor = vec4(lightColor, 1.0);
        return;
    }

    // Kolor bazowy
    vec4 baseColor = bUseTexture ? texture(uTextureSampler, TexCoords) : vec4(uColor, 1.0);
    if(baseColor.a < 0.1) discard;

    // Rendering bez oświetlenia (F1)
    if(!useLighting) {
        FragColor = baseColor;
        return;
    }

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0);

    // --- LOGIKA WYBORU TRYBU ---
    
    if(!isPointLight) {
        // --- TRYB KIERUNKOWY (H) ---
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
        // --- TRYB WIELU ŚWIATEŁ PUNKTOWYCH (N) ---
        for(int i = 0; i < activeLightsCount; i++) {
            result += CalcPointLight(lights[i], norm, FragPos, viewDir, baseColor.rgb);
        }
    }

    FragColor = vec4(result, baseColor.a);
}