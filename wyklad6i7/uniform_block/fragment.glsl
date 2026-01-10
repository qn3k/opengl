#version 330 core

// Struktura danych odebrana
// z poprzedniego etapu renderingu
// (interface block)
in VS_OUT
{
	vec4 position;
	vec4 color;
	vec4 normal;
	vec2 uv;

} fs_in;

// ------------------------------------
// Zmienne i bloki typu uniform
// ------------------------------------


// 1. Pojedyncza zmienna uniform
uniform float Gray;

// 2. Tablica zmiennych
uniform float Table_of_Numbers[11];


// 3. Struktura
// a) definicja
struct SColor
{
	float r;
	float g;
	float b;
	float a;
};

// b) obiekt struktury typu uniform
uniform SColor greenColor;



// 5. Blok zmiennych jednorodnych polaczony
// z buforem zmiennych jednorodnych
// a) definicja struktury
struct StrColor
{
	vec4 color;
};

// b) Blok zmiennych jednorodnych
// w uzytym standardzie std140 uzupelniamy
// skladowe do vec4
layout (std140) uniform Table_of_Colors
{
	StrColor Block_Color[6];
};



// Ostateczny kolor fragmentu
out vec4 outColor;


void main()
{
	vec4 newColor = fs_in.color;

	newColor.r = Gray;
	newColor.g = Table_of_Numbers[gl_PrimitiveID];

	//newColor.r = Table_of_Numbers[5];
	//newColor.g = Table_of_Numbers[5];
	//newColor.b = Table_of_Numbers[5];

	//newColor = vec4(greenColor.r, greenColor.g, greenColor.b, greenColor.a);

	// blok zmiennych uniform
	newColor = Block_Color[gl_PrimitiveID].color;



    outColor = newColor;
}
