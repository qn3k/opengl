#version 330 core
// Dane z VAO
layout( location = 0 ) in vec4 inPosition;
layout( location = 1 ) in vec2 inUV;
layout( location = 2 ) in vec3 inNormal;

// -----------------------------------
// NOWE: uporzadkujemy dane przesylane
// do kolejnego shadera w postaci
// bloku interfejsu
// -----------------------------------
out VS_OUT
{
	vec3 fragPos;
	vec2 fragUV;
	vec3 fragNormal;
} vs;


uniform mat4 matProj;
uniform mat4 matView;
uniform mat4 matModel;

void main()
{
	// ---------------------------------------
	// NOWE: do zmiennych out odwolujemy sie
	// jak do skladowych struktury
	// ---------------------------------------
	vs.fragPos = inPosition.xyz;
	vs.fragUV = inUV;
	vs.fragNormal = inNormal;

	gl_Position = matProj * matView * matModel * inPosition;
}
