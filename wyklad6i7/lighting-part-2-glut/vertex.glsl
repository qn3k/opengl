#version 330 core

// Atrybuty wierzcholkow z VAO
layout( location = 0 ) in vec4 inPosition;
layout( location = 1 ) in vec2 inUV;
layout( location = 2 ) in vec3 inNormal;


// Struktura parametrow swiatla
struct LightParam
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
	vec3 Attenuation;
	vec3 Position; // Direction dla kierunkowego
};
// Przykladowe swiatlo
uniform LightParam myLight;

// Struktura parametrow materialu
struct MaterialParam
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
	float Shininess;
};
// Przykladowy material
uniform MaterialParam myMaterial;

// Macierze rzutowania i transformacji
uniform mat4 matProj;
uniform mat4 matView;
uniform mat4 matModel;
uniform vec3 cameraPos;

// Dane przesylane do kolejnego etapu
out vec4 Position;
out vec2 UV;
out vec3 Normal;
out vec3 lightCoef;

// ------------------------------------------------------------
// Oswietlenie punktowe
vec3 calculatePointLight(vec4 Position, vec3 Normal, LightParam light, MaterialParam material)
{
	// Ambient
	vec3 ambientPart = light.Ambient * material.Ambient;

	// Diffuse
	vec3 L = normalize(light.Position - Position.xyz);
	float diff = max(dot(L, Normal), 0);
	vec3 diffusePart = diff * light.Diffuse * material.Diffuse;

	// Specular
	vec3 E = normalize(cameraPos - Position.xyz);
	vec3 R = reflect(-E, Normal);
	float spec = pow(max(dot(R,L), 0), material.Shininess);
	vec3 specularPart = spec * light.Specular * material.Specular;

	// Wspolczynnik tlumienia
	float LV = distance(Position.xyz, light.Position);
	float latt = 1.0 / (light.Attenuation.x + light.Attenuation.y * LV + light.Attenuation.z * LV * LV);

	// Glowny wzor
	vec3 lightCoef = ambientPart + latt * (diffusePart + specularPart);
	return lightCoef;
}


// ------------------------------------------------------------
void main()
{

	// Przekazanie danych do shadera fragmentow
	Position = inPosition;
	UV = inUV;
	Normal = inNormal;

	// Wspolczynnik swiatla
	lightCoef = calculatePointLight(Position, Normal, myLight, myMaterial);

	// Ostateczna pozycja wierzcholka
	gl_Position = matProj * matView * matModel * inPosition;
}
