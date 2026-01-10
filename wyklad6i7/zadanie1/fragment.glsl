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
uniform vec3 lightColor;  // Kolor światła (zmieniany klawiszami 1-4)
uniform bool useLighting;
uniform bool useBlinnPhong;

// Uniformy tekstur i koloru obiektu
uniform sampler2D uTextureSampler;
uniform bool bUseTexture;
uniform vec3 uColor;

uniform Material material;

void main()
{
    // 1. USTALENIE BAZOWEGO KOLORU OBIEKTU (Tekstura lub kolor stały)
    vec4 baseColor;
    if(bUseTexture) {
        baseColor = texture(uTextureSampler, TexCoords);
        if(baseColor.a < 0.1) discard; // Alpha test dla kwiatów
    } else {
        baseColor = vec4(uColor, 1.0);
    }

    // Jeśli oświetlenie wyłączone (L), zwróć czysty kolor
    if(!useLighting) {
        FragColor = baseColor;
        return;
    }

    // --- PARAMETR MOCY ŚWIATŁA ---
    // Możesz zwiększyć tę wartość (np. na 2.0), aby światło było jeszcze silniejsze
    float lightIntensity = 1.5; 
    vec3 effectiveLightColor = lightColor * lightIntensity;

    // 2. OBLICZENIA OŚWIETLENIA (Model Phonga / Blinna-Phonga)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // AMBIENT (Światło otoczenia - słabe, by cienie nie były całkiem czarne)
    vec3 ambient = material.ambient * effectiveLightColor * baseColor.rgb;

    // DIFFUSE (Światło rozproszone - główny kolor oświetlonej powierzchni)
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = (material.diffuse * diff) * effectiveLightColor * baseColor.rgb;

    // SPECULAR (Błysk światła na powierzchni)
    float spec = 0.0;
    if(useBlinnPhong) {
        // Model Blinn-Phong (Wektor połowiczny)
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    } else {
        // Model Phong (Wektor odbicia)
        vec3 reflectDir = reflect(-lightDir, norm);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    
    // Specular zazwyczaj odbija kolor światła, nie obiektu
    vec3 specular = (material.specular * spec) * effectiveLightColor;

    // 3. SKŁADANIE FINALNEGO PIKSELA
    vec3 result = ambient + diffuse + specular;
    
    // Nałożenie ewentualnego osłabienia (opcjonalne, tutaj zakomentowane dla pełnej mocy)
    // float distance = length(lightPos - FragPos);
    // result /= (1.0 + 0.02 * distance * distance); 

    FragColor = vec4(result, baseColor.a);
}