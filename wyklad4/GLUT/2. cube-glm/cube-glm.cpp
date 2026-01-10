// -------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// -------------------------------------------------
// Przyklad programu, ktory renderuje szescian na
// ekranie z mozliwoscia jego obracania klawiszami
// WSAD. 
// 
// Uwaga: tym razem generowanie macierzy 
// transformacji wykonamy za pomoca biblioteki glm 
// po stronie CPU i przesylamy do potoku za pomoca
// zmiennej jednorodnej.
// -------------------------------------------------
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

// ---------------------------------------
// NOWE! Bibliteka GLM
// ---------------------------------------
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// Funkcje pomocnicze
#include "utilities.hpp"



// Identyfikatory obiektow
GLuint idProgram;	// programu
GLuint idVAO;		// tablic wierzcholkow
GLuint idVBO_coord;	// bufor wspolrzednych
GLuint idVBO_color; // bufor na kolory

// Dane szescianu
#include "cube.h"

// ------------------------------------------
// NOWE! Macierze transformacji i rzutowania
// ------------------------------------------
glm::mat4 matProj;
glm::mat4 matView;
glm::mat4 matModel;


// Zmienne do kontroli obrotem obiektu
float angleY;
float angleX;

// ---------------------------------------
void DisplayScene()
{

	// Czyszczenie bufora koloru i glebokosci
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


	// --------------------------------------------------
	// NOWE! Obliczanie macierzy widoku (pozycja kamery)
	// --------------------------------------------------
	matView = glm::mat4(1.0);
	matView = glm::translate(matView, glm::vec3(0.0, 0.0, -2.0));

	// -------------------------------------------------------
	// NOWE! Obliczanie macierzy modelu obiektu
	// -------------------------------------------------------
	matModel = glm::mat4(1.0);
	matModel = glm::rotate(matModel, angleY, glm::vec3(0.0, 1.0, 0.0));
	matModel = glm::rotate(matModel, angleX, glm::vec3(1.0, 0.0, 0.0));



	// Wlaczenie programu
	glUseProgram( idProgram );


		// ------------------------------------------------------------------
		// NOWE! Przekazujemy macierze matProj, matView i matModel do potoku
		// ------------------------------------------------------------------
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(matProj) );
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(matView) );
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModel) );


		// Uruchamiamy rendering szescianu
		glBindVertexArray( idVAO );
		glDrawArrays( GL_TRIANGLES, 0, NUMBER_OF_TRIANGLES*3 );
		glBindVertexArray( 0 );


	// Wylaczanie potoku
	glUseProgram( 0 );


	glutSwapBuffers();
}

// ---------------------------------------
void Initialize()
{

	// Ustawianie koloru, ktorym bedzie czyszczony bufor koloru
	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );

	// Wlczanie testu glebokosci
	glEnable(GL_DEPTH_TEST);



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

	// -------------------------------------------------------
	// NOWE! Obliczanie macierzy rzutowania perspektywicznego
	// Dlaczego akurat tutaj?
	// -------------------------------------------------------
	matProj = glm::perspective(glm::radians(70.0f), width/(float)height, 0.1f, 20.0f );
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
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
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
