#version 330 core
in vec3 vNormal;
in vec3 vFragPos;
in vec2 vTexCoord;

out vec4 FragColor;

uniform vec3 uColor;
uniform sampler2D uTextureSampler; 
uniform int bUseTexture; // 1: Użyj tekstury (może być alpha/discard), 0: Użyj uColor

void main()
{
    // 1. Oświetlenie (Prosty Ambient + Diffuse)
    vec3 lightPos = vec3(5.0, 5.0, 5.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    
    // Ambient
    vec3 ambient = 0.2 * lightColor;
    
    // Diffuse
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(lightPos - vFragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 2. Kolor Obiektu (Kolor czy Tekstura + Logika Kwiatu/Alpha)
    vec4 finalObjectColor;
    
    if (bUseTexture == 1)
    {
        // Mamy teksturę - pobieramy z niej kolor i kanał alpha
        vec4 texColor = texture(uTextureSampler, vTexCoord);
        
        // discard jesli jest za maly piksel 
        if (texColor.a < 0.1) {
            discard;
        }
        
        // Używamy koloru z tekstury
        finalObjectColor = texColor;
    }
    else
    {
        // bez alpha discard
        finalObjectColor = vec4(uColor, 1.0);
    }
    
    // Finalne Obliczenie Koloru
    
    // Wynikowe natężenie światła (Ambient + Diffuse) jest mnożone przez kolor obiektu (RGB)
    vec3 resultRGB = (ambient + diffuse) * finalObjectColor.rgb;
    
    FragColor = vec4(resultRGB, finalObjectColor.a); 
}