// -------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// -------------------------------------------------
// Przyklad programu, ktory renderuje szescian na
// ekranie z mozliwoscia jego obracania klawiszami
// WSAD.
// 
// Uwaga: przyklad najprostszy - bez uzycia 
// biblioteki glm do obslugi typow macierzowych
// po stronie CPU.
// -------------------------------------------------
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

// Funkcje pomocnicze
#include "utilities.hpp"

// Identyfikatory obiektow
GLuint idProgram;	// programu
GLuint idVAO;		// tablic wierzcholkow
GLuint idVBO_coord;	// bufor wspolrzednych
GLuint idVBO_color; // bufor na kolory

// Dane szescianu
#include "cube.h"

// Zmienne do obracania szescianem
float angleX;
float angleY;


// ---------------------------------------
void DisplayScene()
{
	// Czyszczenie bufora koloru
	glClear( GL_COLOR_BUFFER_BIT );


	// Wlaczenie programu
	glUseProgram( idProgram );

		// Przekazujemy katy obrotu do potoku
		glUniform1f( glGetUniformLocation(idProgram, "AngleX"), angleX );
		glUniform1f( glGetUniformLocation(idProgram, "AngleY"), angleY );


		// Aktywacja VAO i uruchomienie renderingu
		glBindVertexArray( idVAO );
		glDrawArrays( GL_TRIANGLES, 0, NUMBER_OF_TRIANGLES*3 );
		glBindVertexArray( 0 );


	// Wylaczanie
	glUseProgram( 0 );


	glutSwapBuffers();
}

// ---------------------------------------
void Initialize()
{
	// Ustawienie maszyny stanów OpenGL
	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );


	// Potok
	idProgram = glCreateProgram();
		glAttachShader( idProgram, LoadShader(GL_VERTEX_SHADER, "vertex.glsl"));
		glAttachShader( idProgram, LoadShader(GL_FRAGMENT_SHADER, "fragment.glsl"));
	LinkAndValidateProgram( idProgram );


	// VAO
	glGenVertexArrays( 1, &idVAO );
	glBindVertexArray( idVAO );
		// Bufor na wspolrzedne wierzcholkow
		glGenBuffers( 1, &idVBO_coord );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_coord );
		glBufferData( GL_ARRAY_BUFFER, sizeof( coord ), coord, GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		// Bufor na kolory wierzcholkow
		glGenBuffers( 1, &idVBO_color );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_color );
		glBufferData( GL_ARRAY_BUFFER, sizeof( color ), color, GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
	glBindVertexArray( 0 );

}

// ---------------------------------------
void Reshape( int width, int height )
{
	glViewport( 0, 0, width, height );
}

// ---------------------------------------------------
void Keyboard( unsigned char key, int x, int y )
{
    switch(key)
    {
		case 27:	// ESC key
			exit(0);
			break;

		case 'w':
			angleX += 0.1f;
			break;

		case 's':
			angleX -= 0.1f;
			break;

		case 'd':
			angleY += 0.1f;
			break;

		case 'a':
			angleY -= 0.1f;
			break;

    }

    glutPostRedisplay();
}


// ---------------------------------------------------
int main( int argc, char *argv[] )
{
	// GLUT
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB );
	glutInitContextVersion( 3, 3 );
	glutInitContextProfile( GLUT_CORE_PROFILE );
	glutInitWindowSize( 500, 500 );
	glutCreateWindow( "Obracanie klawiszami WSAD" );
	glutDisplayFunc( DisplayScene );
	glutReshapeFunc( Reshape );

	// Keyboard
	glutKeyboardFunc( Keyboard );


	// GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if( GLEW_OK != err )
	{
		printf("GLEW Error\n");
		exit(1);
	}

	// OpenGL
	if( !GLEW_VERSION_3_3 )
	{
		printf("Brak OpenGL 3.3!\n");
		exit(1);
	}


	Initialize();
	glutMainLoop();


	// Cleaning
	glDeleteProgram( idProgram );
	glDeleteBuffers( 1, &idVBO_coord );
	glDeleteBuffers( 1, &idVBO_color );
	glDeleteVertexArrays( 1, &idVAO );

	return 0;
}
