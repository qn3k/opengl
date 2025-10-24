// -------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// -------------------------------------------------
// Prosty szablon aplikacji w OpenGLu
// Rozbudowany o przekazywanie dwoch atrybutow:
// - wspolrzednych wierzcholkow
// - kolorow wierzcholkow
// -------------------------------------------------
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

// Funkcje pomocnicze
#include "utilities.hpp"


// ---------------------------------------
// Identyfikatory obiektow OpenGL

GLuint idProgram;	// poroku/programu
GLuint idVAO;		// tablicy wierzcholkow
GLuint idVBO_coord; // bufora na wspolrzedne wierzcholkow
GLuint idVBO_color; // bufora na kolory wierzcholkow


// ---------------------------------------
// Dane geometrii

// Wspolrzedne wierzchokow
// po dwa floaty na wierzcholek
// wspolrzedne w zakresie -1.0 do 1.0
GLfloat triangles[6*2] =
{
	-0.8, -0.8,	// wierzcholek 0
	 0.0, -0.8, // wierzcholek 1
	-0.4,  0.0, // itd.

	 0.0,  0.0,
	 0.8,  0.0,
	 0.4,  0.8
};

// Kolory wierzcholkow
// po trzy floaty na wierzcholek
// kolory w palecie RGB w zakresie 0.0 - 1.0
GLfloat colors[6*3] =
{
	1.0, 0.0, 0.0, // wierzcholek 0
	1.0, 0.0, 0.0, // wierzcholek 1
	1.0, 0.0, 0.0, // itd.

	0.0, 0.0, 1.0,
	0.0, 0.0, 1.0,
	0.0, 0.0, 1.0
};


// ---------------------------------------
void DisplayScene()
{
	// ---------------------------
	// Etap (5) rendering
	// ---------------------------
	glClear( GL_COLOR_BUFFER_BIT );

	// Wlaczenie potoku
	glUseProgram( idProgram );

		// Aktywacja tablicy wierzcholkow (VAO)
		glBindVertexArray( idVAO );
		// Uruchomienie aktualnego potoku na aktualnej tablicy wierzcholkow
		// Renderowanie za pomoca GL_TRIANGLES wierzhcholkow 6 poczawszy od 0
		glDrawArrays( GL_TRIANGLES, 0, 6 );
		// Dezaktywujemy VAO
		glBindVertexArray( 0 );

	// Dezaktywujemy potok
	glUseProgram( 0 );


	glutSwapBuffers();
}

// ---------------------------------------
void Initialize()
{
	// -------------------------------------------------
	// Etap (2) przeslanie danych wierzcholków do OpenGL
	// -------------------------------------------------

	// Tworzymy tablice wierzcholkow (VAO)
	glGenVertexArrays( 1, &idVAO );
	glBindVertexArray( idVAO );
		// Tworzymy bufor na wspolrzedne wierzcholkow
		glGenBuffers( 1, &idVBO_coord );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_coord );
		glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * 2*6, triangles, GL_STATIC_DRAW );
		// Ustawienie danych bufora pod atrybut o numerze 0
		// ten sam ustawiony w vertex shaderze
		glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );

		// Tworzymy bufor na kolory wierzcholkow
		glGenBuffers( 1, &idVBO_color );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_color );
		glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * 3*6, colors, GL_STATIC_DRAW );
		// Ustawienie danych bufora pod atrybut o numerze 1
		// ten sam ustawiony w vertex shaderze
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );

	glBindVertexArray( 0 );


	// ---------------------------------------
	// Etap (3) stworzenie potoku graficznego
	// ---------------------------------------
	idProgram = glCreateProgram();
		glAttachShader( idProgram, LoadShader(GL_VERTEX_SHADER, "vertex.glsl"));
		glAttachShader( idProgram, LoadShader(GL_FRAGMENT_SHADER, "fragment.glsl"));
	LinkAndValidateProgram( idProgram );



	// -----------------------------------------
	// Etap (4) ustawienie maszyny stanow OpenGL
	// -----------------------------------------

	// ustawienie koloru czyszczenia ramki koloru
	glClearColor( 0.9f, 0.9f, 0.9f, 0.9f );

}

// ---------------------------------------------------
// Funkcja wywolywana podczas tworzenia okna aplikacji
// oraz zmiany jego rozdzielczosci
void Reshape( int width, int height )
{
	glViewport( 0, 0, width, height );
}



// ---------------------------------------------------
// Funkcja wywolywana podczas wcisniecia klawisza ASCII
void Keyboard( unsigned char key, int x, int y )
{
    switch(key)
    {
		case 27:	// ESC key
			// opuszczamy glowna petle
			glutLeaveMainLoop();
			break;

		case ' ':	// Space key
			printf("SPACE!\n");
			// ponowne renderowanie
			glutPostRedisplay();
			break;

		case 'x':
			// opuszczamy glowna petle
			glutLeaveMainLoop();
			break;
    }
}


// ---------------------------------------------------
int main( int argc, char *argv[] )
{
	// -----------------------------------------------
	// Etap (1) utworzynie kontektu OpenGLa i okna
	// aplikacji, rejestracja funkcji zwrotnych
	// -----------------------------------------------

	// GLUT
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB );
	glutInitContextVersion( 3, 3 );
	glutInitContextProfile( GLUT_CORE_PROFILE );
	glutInitWindowSize( 500, 500 );
	glutCreateWindow( "Szablon programu w OpenGL" );
	glutDisplayFunc( DisplayScene );
	glutReshapeFunc( Reshape );
	glutKeyboardFunc( Keyboard );


	// GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if( GLEW_OK != err ) {
		printf("GLEW Error\n");
		exit(1);
	}

	// OpenGL
	if( !GLEW_VERSION_3_3 ) {
		printf("Brak OpenGL 3.3!\n");
		exit(1);
	}


	// Inicjalizacja
	Initialize();

	// Rendering
	glutMainLoop();


	// Cleaning
	glDeleteProgram( idProgram );
	glDeleteVertexArrays( 1, &idVBO_coord );
	glDeleteVertexArrays( 1, &idVAO );

	return 0;
}
