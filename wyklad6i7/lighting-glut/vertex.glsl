#version 330 core

// Atrybuty wierzcholkow z VAO
layout( location = 0 ) in vec4 inPosition;
layout( location = 1 ) in vec2 inUV;
layout( location = 2 ) in vec3 inNormal;

// Macierz rzutowania i transformacji
uniform mat4 matProj;
uniform mat4 matView;
uniform mat4 matModel;

// -------------------------------------------
// NOWE: Pozycja kamery przekazana z aplikacji
// -------------------------------------------
uniform vec3 cameraPos;


// Dane wyjsciowe do shadera fragmentow
out vec3 fragPos;
out vec2 fragUV;
out vec3 fragNormal;

// -------------------------------------------
// NOWE: Wspolczynnik oswietlenia
// -------------------------------------------
out vec3 fragLightCoef;



// -------------------------------------------
// NOWE: Stuktury potrzebne do oswietlenia
// -------------------------------------------

// Struktura parametrow swiatla
struct LightParam
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
	vec3 Attenuation;
	vec3 Position; // Direction dla kierunkowego
};

// Struktura parametrow materialu
struct MaterialParam
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
	float Shininess;
};


// -------------------------------------------
// NOWE: Funkcja wyliczajaca wspolczynnik 
// oswietlenia swiatla punktowego
// -------------------------------------------
vec3 calculatePointLight(vec3 Position, vec3 Normal, vec3 CameraPos, LightParam light, MaterialParam material)
{
	// 1. Czesc ambientowa
    vec3 ambientPart = light.Ambient * material.Ambient;
	
	// 2. Czesc rozproszeniowa
    vec3 L = normalize(light.Position - Position);
    float diff = max(dot(L, Normal), 0);
    vec3 diffusePart = diff * light.Diffuse * material.Diffuse;

	// 3. Czesc odblaskowa
    vec3 E = normalize(CameraPos - Position);
    vec3 R = reflect(-E, Normal);
    float spec = pow(max(dot(R,L), 0), material.Shininess);
    vec3 specularPart = spec * light.Specular * material.Specular;
	
	// Wspolczynnik tlumienia
    float LV = distance(Position, light.Position);
    float latt = 1.0 / (light.Attenuation.x + light.Attenuation.y * LV + light.Attenuation.z * LV * LV);

	// Ostateczny wzor na wspolczynnik oswietlenia
    return ambientPart + latt * (diffusePart + specularPart);
}





void main()
{
	// Przekazanie danych do shadera fragmentow
	fragPos = inPosition.xyz;
	fragUV = inUV;
	fragNormal = inNormal;
	
	
	// Przykladowe swiatlo
	LightParam myLight = LightParam
	(
		vec3 (0.1 , 0.1 , 0.1), // ambient
		vec3 (1.0 , 1.0 , 1.0), // diffuse
		vec3 (1.0 , 1.0 , 1.0), // specular
		vec3 (1.0 , 0.0 , 0.01), // attenuation
		vec3 (2.0 , 3.0 , 1.0)  // position
	);

	// Przykladowy material
	MaterialParam myMaterial = MaterialParam
	(
		vec3 (0.2, 0.2, 0.2), // ambient
		vec3 (1.0, 1.0, 1.0), // diffuse
		vec3 (0.5, 0.5, 0.5), // specular
		32.0 // shininess
	);


	// Wyliczanie wspolczynnika oswietlenia
	fragLightCoef = calculatePointLight(inPosition.xyz, inNormal, cameraPos, myLight, myMaterial);


	// Ostateczna pozycja wierzcholka
	gl_Position = matProj * matView * matModel * inPosition;
}
