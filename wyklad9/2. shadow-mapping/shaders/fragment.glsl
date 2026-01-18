#version 150 core

// Dane wejsciowe
in vec4 fragPos;
in vec4 fragPosLight;
in vec3 fragNormal;

// Struktura 
struct LightParameters
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
	vec3 Direction;
};

// Dane potrzebne do oswietlenia kierunkowego
uniform vec3 cameraPos;
uniform vec3 lightDirection;
// Dane potrzebne do wyliczenia cienia
uniform sampler2D tex_shadowMap;




// ---------------------------------------------------------------------------
// Zwraca [0-1], gdzie 1 oznacza ze fragment jest calkowicie w cieniu
// ---------------------------------------------------------------------------
float calcDirectionalShadow(vec4 fragPosLight, vec3 fragNormal, vec3 lightDirection)
{

	// Brak cienia
	// return 0;

	// Korekcja perspektywiczna (dla oswietlenia kierunkowego niepotrzebna)
	vec3 projCoords = fragPosLight.xyz / fragPosLight.w;
	// przeksztalcenie wartosci [-1,+1] na [0,1]
	projCoords = projCoords * 0.5 + 0.5;

	// pobranie z tekstury shadowMap odleglosci od zrodla swiatla fragmentu
	// do fragmentu oswietlonego na drodze do aktualnego fragmentu
	float closestDepth = texture(tex_shadowMap, projCoords.xy).r;

	// obliczenie aktualnej odleglosci od zrodla swiatla
	float currentDepth = projCoords.z;

	// Sprawdzenie czy fragment jest w cieniu po odleglosci
	//return (currentDepth > closestDepth) ? 1.0 : 0.0;

	// Shadow acne
	// Parametry i stale ustawiamy eksperymentalnie
	float bias = max(0.004 * (1.0 - dot(fragNormal, lightDirection)), 0.001);
	return (currentDepth - bias > closestDepth) ? 1.0 : 0.0;


}

// ---------------------------------------------------------------------------
// Uproszczone oswietlenie kierunkowe ktore UWAGA: sam diffuse
// ---------------------------------------------------------------------------
vec3 calcDirectionalLight(vec3 fragNormal, vec3 lightDirection)
{
	// Diffuse
	vec3  lightDiffuse_color = vec3(0.8, 0.8, 0.8);
	float diff = max(dot(fragNormal, -lightDirection), 0.0);
	vec3  resultDiffuse = diff * lightDiffuse_color;

	return resultDiffuse;
}

out vec4 outColor;


// ---------------------------------------------------------------------------
void main()
{

	
	// Kolor poczatkowy
	vec3 fragColor = vec3(0.1, 1.0, 0.1);
	vec3 ambientColor = vec3(0.2, 0.2, 0.2);
	
	
	// Oswietlenie kierunkowe (bardzo uproszczone, tylko Diffuse)
	vec3  lightPart = calcDirectionalLight(fragNormal, lightDirection);

	// Cienie swiatla kierunkowego
	float shadowPart = calcDirectionalShadow(fragPosLight, fragNormal, lightDirection);

	// Kolor finalny
	vec3  finalColor = (ambientColor + (1 - shadowPart) * lightPart) * fragColor;

	outColor = vec4(finalColor, 1.0);

}
