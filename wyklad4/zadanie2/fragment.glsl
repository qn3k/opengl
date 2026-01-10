#version 330 core

in vec3 vLocalPos;
in vec3 vNormal;

out vec4 FragColor;

uniform vec3 uColor1; // Kolor dolny
uniform vec3 uColor2; // Kolor górny

void main()
{
    // 1. Obliczanie Gradientu
    // Stożki i małpka zazwyczaj mają współrzędne Y od około -1.0 do 1.0.
    // Musimy zmapować ten zakres na 0.0 - 1.0, żeby użyć funkcji mix.
    
    float gradientFactor = (vLocalPos.y + 1.0) / 2.0;

    gradientFactor = clamp(gradientFactor, 0.0, 1.0);

    // Mieszamy dwa kolory
    vec3 objectColor = mix(uColor1, uColor2, gradientFactor);

    // 2. Proste oświetlenie (opcjonalne, dodaje trójwymiarowości)
    vec3 lightDir = normalize(vec3(5.0, 10.0, 5.0)); // Światło z góry-boku
    vec3 norm = normalize(vNormal);
    float diff = max(dot(norm, lightDir), 0.3); 
    
    vec3 finalColor = objectColor * diff;

    FragColor = vec4(finalColor, 1.0);
}