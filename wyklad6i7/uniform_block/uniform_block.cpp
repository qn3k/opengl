// ----------------------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// ----------------------------------------------------------------
// Przyklad przesylania zmiennych jednorodnych oraz tworzenia UBO
// 1. pojedynczej zmiennej
// 2. tablicy zmiennych
// 3. wybranego elementu tablicy
// 4. skladowych struktury
// 5. tablicy struktur za pomoca bloku zmiennych jednorodnych (UBO)
// ----------------------------------------------------------------
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "utilities.hpp"


// ---------------------------------------
// Identyfikatory obiektow

GLuint idProgram;
GLuint idVAO;
GLuint idUBO;

// ---------------------------------------
// Wspolrzedne wierzchokow
GLfloat triangles_coord[2*6] =
{
	-0.5f, -1.0f,
	 0.5f, -1.0f,
	 1.0f,  0.0f,
	 0.5f,  1.0f,
	-0.5f,  1.0f,
	-1.0f,  0.0f
};

// ---------------------------------------
// Kolory wierzcholkow
GLfloat triangles_color[3*6] =
{
	1.f, 0.f, 0.f,
	0.f, 1.f, 0.f,
	0.f, 0.f, 1.f,
	1.f, 1.f, 0.f,
	0.f, 1.f, 1.f,
	1.f, 0.f, 1.f
};

// ---------------------------------------
// Indeksy wierzcholkow
GLuint triangles_indices[3*4] =
{
	0, 1, 2,
	0, 2, 3,
	0, 3, 4,
	0, 4, 5
};


// 1. Pojedyncza zmienna
float Gray = 0.5;

// 2. Tablica zmiennych
float Table_of_Numbers[11] = { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };

// 3. Struktura
struct SColor
{
	float r;
	float g;
	float b;
	float a;
};

struct SColor greenColor = {0.1, 1.0, 0.1, 1.0};


// 5. Tablica struktur
struct SColor Table_of_Colors[6] =
{
	1.f, 0.f, 0.f, 1.f,
	0.f, 1.f, 0.f, 1.f,
	0.f, 0.f, 1.f, 1.f,
	1.f, 1.f, 0.f, 1.f,
	0.f, 1.f, 1.f, 1.f,
	1.f, 0.f, 1.f, 1.f
};






// ---------------------------------------
void DisplayScene()
{

	glClear( GL_COLOR_BUFFER_BIT );

	// Potok
	glUseProgram( idProgram );


		// 1. Przekazywanie pojedynczej wartosci
		glUniform1f(glGetUniformLocation(idProgram, "Gray"), Gray);

		// 2. Przekazywanie tablicy w calosci
		glUniform1fv(glGetUniformLocation(idProgram, "Table_of_Numbers"), 11, Table_of_Numbers);

		// 3. Przekazanie elementu tablicy
		glUniform1f(glGetUniformLocation(idProgram, "Table_of_Numbers[5]"), Table_of_Numbers[5]);

		// 4. Przekazanie skladowych struktury
		glUniform1f(glGetUniformLocation(idProgram, "greenColor.r"), greenColor.r);
		glUniform1f(glGetUniformLocation(idProgram, "greenColor.g"), greenColor.g);
		glUniform1f(glGetUniformLocation(idProgram, "greenColor.b"), greenColor.b);
		glUniform1f(glGetUniformLocation(idProgram, "greenColor.a"), greenColor.a);


		// 5. Przekazanie tablicy struktur za pomoca bufora zmiennych jednorodnych
		glBindBuffer(GL_UNIFORM_BUFFER, idUBO);
			// a) zaktualizowanie danych calej tablicy
			glBufferData(GL_UNIFORM_BUFFER, sizeof(Table_of_Colors), Table_of_Colors, GL_DYNAMIC_DRAW);
			// b) zaktualizowanie czesciowych danych
			//glBufferSubData(GL_UNIFORM_BUFFER,  8, sizeof(Table_of_Colors)-8, Table_of_Colors);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);


		// Renderowanie obiektu
		glBindVertexArray( idVAO );
		glDrawElements( GL_TRIANGLES, 4 * 3, GL_UNSIGNED_INT, NULL );
		glBindVertexArray( 0 );


	glUseProgram( 0 );
	glutSwapBuffers();
	__CHECK_FOR_ERRORS
}


// ---------------------------------------
void Reshape( int width, int height )
{
	glViewport( 0, 0, width, height );
}


// ---------------------------------------
void Initialize()
{
	// Ustawienia maszyny stanow OpenGL
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );


	// Potok i shadery
	idProgram = glCreateProgram();
	glAttachShader( idProgram, LoadShader(GL_VERTEX_SHADER, "vertex.glsl"));
	glAttachShader( idProgram, LoadShader(GL_FRAGMENT_SHADER, "fragment.glsl"));
	LinkAndValidateProgram( idProgram );


	// Vertex array
	glGenVertexArrays( 1, &idVAO );
	glBindVertexArray( idVAO );

		// BUFOR: Wspolrzedne wierzcholkow
		GLuint idBuffer_coord;
		glGenBuffers( 1, &idBuffer_coord );
		glBindBuffer( GL_ARRAY_BUFFER, idBuffer_coord );
		glBufferData( GL_ARRAY_BUFFER, sizeof( triangles_coord ), triangles_coord, GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );

		// BUFOR: Kolor wierzcholkow
		GLuint idBuffer_color;
		glGenBuffers( 1, &idBuffer_color );
		glBindBuffer( GL_ARRAY_BUFFER, idBuffer_color  );
		glBufferData( GL_ARRAY_BUFFER, sizeof( triangles_color ), triangles_color, GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );

		// BUFOR: Indeksy wierzcholkow
		GLuint idBuffer_idx;
		glGenBuffers( 1, &idBuffer_idx );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, idBuffer_idx );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( triangles_indices ), triangles_indices, GL_STATIC_DRAW );

	glBindVertexArray( 0 );


	// -------------------------------------
	// NOWE: UBO - Uniform Block Objects
	// -------------------------------------

	// 1. Tworzymy UBO - bufor na dane jednorodne
	glGenBuffers(1, &idUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, idUBO);

	// 2a) Alokacja pamieci bufora bez przekazania danych
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Table_of_Colors), NULL, GL_STATIC_DRAW);
	// 2b) Alokacja i skopiowanie danych
	// glBufferData(GL_UNIFORM_BUFFER, sizeof(Table_of_Colors), Table_of_Colors, GL_DYNAMIC_DRAW);


	// 3. Pobranie lokalizacji bloku jednorodnego
	GLuint locationUBO = glGetUniformBlockIndex(idProgram, "Table_of_Colors");

	// 4. Ustalamy punkt bindowania (w wersji 3.3 sami ustalamy)
	GLuint bindingPoint = 1;
	glUniformBlockBinding(idProgram, locationUBO, bindingPoint);

	// 5. Polaczenie punktu bindowania i UBO
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, idUBO);



}




// ---------------------------------------------------
int main( int argc, char *argv[] )
{
	// GLUT
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB );
	glutInitContextVersion( 3, 2 );
	glutInitContextProfile( GLUT_CORE_PROFILE );
	glutInitWindowSize( 500, 500 );
	glutCreateWindow( "Tablice wierzcholkow" );
	glutDisplayFunc( DisplayScene );
	glutReshapeFunc( Reshape );


	// GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if( GLEW_OK != err )
	{
		printf("GLEW Error\n");
		exit(1);
	}

	// OpenGL
	if( !GLEW_VERSION_3_2 )
	{
		printf("Brak OpenGL 3.2!\n");
		exit(1);
	}


	Initialize();


	glutMainLoop();

	// Cleaning
	glDeleteProgram( idProgram );
	glDeleteVertexArrays( 1, &idVAO );

	return 0;
}
