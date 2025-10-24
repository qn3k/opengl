// Fragment shader w wersji 3.3
#version 330 core


// Predefiniowane dane wejsciowe
/*
in  vec4 gl_FragCoord;
in  int  gl_PrimitiveID;
*/


// Dane wyjsciowe, w tym wypadku tylko jedna
// ktora zostanie umieszczona w buforze koloru
out vec4 outColor;


void main()
{

	// W zaleznosci od parzystosci identyfikatora
	// prymitywu ustalany jest kolor wyjsciowy
	if (gl_PrimitiveID%2 == 0)
		outColor = vec4(1.0, 0.0, 0.0, 1.0);
	else
		outColor = vec4(0.0, 1.0, 0.0, 1.0);

	// ------------------------------
	// Zadanie:
	// ------------------------------
	// W zmiennej gl_FragCoord znajduja sie informacje
	// o polozeniu piksela we wspolrzednych obszaru
	// renderingu. Wykorzystaj zatem dwie pierwsze skladowe:
	// gl_FragCoord.x oraz gl_FragCoord.y, aby nadac
	// przejscia gradientowe renderowanym trojkatom.


}
