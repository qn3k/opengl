// Vertex shader
#version 330 core

// Dane pobrane z VAO
layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inColor;

out vec3 fragColor;

// Zmienne jednorodne do kontroli obrotu
uniform float AngleX;
uniform float AngleY;


void main()
{
	fragColor = inColor;

	// Macierz translacji o wektor (MoveX, MoveY, MoveZ)
	float MoveX = 0.0, MoveY = 0.0, MoveZ = 0.0;
	mat4x4 MatrixT;
	MatrixT[0] = vec4( 1, 0, 0, 0);
	MatrixT[1] = vec4( 0, 1, 0, 0);
	MatrixT[2] = vec4( 0, 0, 1, 0);
	MatrixT[3] = vec4( MoveX, MoveY, MoveZ, 1);

	// Macierz obrotu o kat AngleX dookola osi X
	mat4 MatrixX;
	MatrixX[0] = vec4( 1, 0, 0, 0);
	MatrixX[1] = vec4( 0, cos(AngleX), -sin(AngleX), 0);
	MatrixX[2] = vec4( 0, sin(AngleX),  cos(AngleX), 0);
	MatrixX[3] = vec4( 0, 0, 0, 1);

	// Macierz obrotu o kat AngleY dookola osi Y
	mat4 MatrixY;
	MatrixY[0] = vec4( cos(AngleY), 0, sin(AngleY), 0);
	MatrixY[1] = vec4( 0, 1, 0, 0);
	MatrixY[2] = vec4( -sin(AngleY),0, cos(AngleY), 0);
	MatrixY[3] = vec4( 0, 0, 0, 1);

	// Nowa pozycja wierzcholka to iloczyn macierzy 
	// obrotow oraz polozenia poczatkowego
	vec4 newPosition = MatrixX * MatrixY * inPosition;
	
	
	gl_Position = newPosition;
}
