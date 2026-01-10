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

uniform vec3 lightPos;
uniform vec3 dirLightDirection; // Kierunek światła słonecznego
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform bool useLighting;
uniform bool useBlinnPhong;
uniform bool isPointLight;      // Flaga wyboru typu światła
uniform bool bIsLightSource;

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

    vec4 baseColor = bUseTexture ? texture(uTextureSampler, TexCoords) : vec4(uColor, 1.0);
    if(baseColor.a < 0.1) discard;

    if(!useLighting) {
        FragColor = baseColor;
        return;
    }

    float lightIntensity;
    if(isPointLight) {
        lightIntensity = 1.5; // Punktowe zostaje mocne
    } else {
        lightIntensity = 1; // Kierunkowe osłabiamy 
    }
    vec3 effectiveLightColor = lightColor * lightIntensity;
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // --- OBLICZANIE KIERUNKU ŚWIATŁA ---
    vec3 lightDir;
    float attenuation = 1.0;

    if(isPointLight) {
        // Światło punktowe
        lightDir = normalize(lightPos - FragPos);
        float distance = length(lightPos - FragPos);
        attenuation = 1.0 / (1.0 + 0.07 * distance + 0.017 * (distance * distance));
    } else {
        // Światło kierunkowe (odwracamy kierunek, bo potrzebujemy wektora DO światła)
        lightDir = normalize(-dirLightDirection);
        attenuation = 1.0; // Światło kierunkowe nie słabnie wraz z odległością
    }

    // AMBIENT
    vec3 ambient = material.ambient * effectiveLightColor * baseColor.rgb;

    // DIFFUSE
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = (material.diffuse * diff) * effectiveLightColor * baseColor.rgb;

    // SPECULAR
    float spec = 0.0;
    if(useBlinnPhong) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    } else {
        vec3 reflectDir = reflect(-lightDir, norm);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    vec3 specular = (material.specular * spec) * effectiveLightColor;

    // SKŁADANIE (Tłumienie tylko dla punktowego)
    vec3 result = ambient + (diffuse + specular) * attenuation;

    FragColor = vec4(result, baseColor.a);
}