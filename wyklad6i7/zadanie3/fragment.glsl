#version 330 core

out vec4 FragColor;

// Dane z Vertex Shadera
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Struktura materiału obiektu
struct Material {
    float ambient;
    float diffuse;
    float specular;
    float shininess;
};

// Uniformy oświetlenia
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor; 
uniform bool useLighting;
uniform bool useBlinnPhong;
uniform bool bIsLightSource; 

// Uniformy tekstur i koloru obiektu
uniform sampler2D uTextureSampler;
uniform bool bUseTexture;
uniform vec3 uColor;

uniform Material material;

void main()
{
    if(bIsLightSource) {
        FragColor = vec4(lightColor, 1.0);
        return;
    }

    // 1. USTALENIE BAZOWEGO KOLORU OBIEKTU
    vec4 baseColor;
    if(bUseTexture) {
        baseColor = texture(uTextureSampler, TexCoords);
        if(baseColor.a < 0.1) discard; 
    } else {
        baseColor = vec4(uColor, 1.0);
    }

    // Jeśli oświetlenie wyłączone (F1), zwróć czysty kolor
    if(!useLighting) {
        FragColor = baseColor;
        return;
    }

    // --- PARAMETR MOCY ŚWIATŁA ---
    float lightIntensity = 1.8; // Zwiększone dla lepszego efektu
    vec3 effectiveLightColor = lightColor * lightIntensity;

    // 2. OBLICZENIA OŚWIETLENIA
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Obliczanie odległości dla tłumienia (Attenuation)
    float distance = length(lightPos - FragPos);
    // Wzór na tłumienie (stała, liniowa, kwadratowa)
    // Dopasowane do promienia ok. 20-30 jednostek
    float attenuation = 1.0 / (1.0 + 0.07 * distance + 0.017 * (distance * distance));

    // AMBIENT (Światło otoczenia - lekko tłumione, by zachować głębię)
    vec3 ambient = material.ambient * effectiveLightColor * baseColor.rgb;

    // DIFFUSE (Lambert)
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = (material.diffuse * diff) * effectiveLightColor * baseColor.rgb;

    // SPECULAR (Błysk)
    float spec = 0.0;
    if(useBlinnPhong) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    } else {
        vec3 reflectDir = reflect(-lightDir, norm);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    vec3 specular = (material.specular * spec) * effectiveLightColor;

    // 3. SKŁADANIE FINALNEGO PIKSELA
    // Tłumienie nakładamy na Diffuse i Specular (Ambient zazwyczaj jest stały)
    vec3 result = ambient + (diffuse + specular) * attenuation;

    FragColor = vec4(result, baseColor.a);
}