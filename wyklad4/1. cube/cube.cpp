// -------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// -------------------------------------------------
// Przyklad programu, ktory renderuje szescian na
// ekranie z mozliwoscia obracania klawiszami
// WSAD.
// -------------------------------------------------
// Szablon aplikacji w OpenGL korzystajacej
// z GLFW zamiast freeGLUT
//
// Kompilacja pod windowsem
// g++ plik.cpp glad.c -lglfw3 -lopengl32 -lgdi32
//
// Wymagana jest bibliteka glad
// https://glad.dav1d.de/
//
// Dolaczony plik glad.c jest skonfigurowany pod
// Opengl 4.5. W razie problemow nalezy podmienic
// skonfigurowac glada pod swoje srodowisko.
// ------------------------------------------------
#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>


// Okno aplikacji
int windowWidth = 600, windowHeight = 600;
const char *windowTitle = "OpenGL w GLFW (obrot klawiszami WSAD)";


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

// ---------------------------------------------------
// funkcja zwrotna do obslugi klawiatury
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch(key)
		{
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;

		case GLFW_KEY_W:
			angleX += 0.1f;
			break;

		case GLFW_KEY_S:
			angleX -= 0.1f;
			break;

		case GLFW_KEY_D:
			angleY += 0.1f;
			break;

		case GLFW_KEY_A:
			angleY -= 0.1f;
			break;

		default:
			printf("Nacisnieto klawisz %d \n", key);
			break;
		}
	}

}


// ---------------------------------------------------
int main( int argc, char *argv[] )
{
	// Kontekst i okno aplikacji
	GLFWwindow* window = nullptr;
	Initialize_GLFW(window);


	// Inicjalizacja naszej sceny
	Initialize();


	// Glowna petla
	while (!glfwWindowShouldClose(window))
	{
		// Obsluga zdarzen
		glfwPollEvents();


		// Render sceny
		DisplayScene();


		glfwSwapBuffers(window);
	}


	// Czyszczenie
	glDeleteProgram( idProgram );
	glDeleteVertexArrays( 1, &idVBO_coord );
	glDeleteVertexArrays( 1, &idVBO_color );
	glDeleteVertexArrays( 1, &idVAO );
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
