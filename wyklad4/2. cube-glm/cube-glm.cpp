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
// -------------------------------------------------
#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// ---------------------------------------
// NOWE! Bibliteka GLM
// ---------------------------------------
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
}

// ---------------------------------------
void Initialize()
{

	// Ustawianie koloru, ktorym bedzie czyszczony bufor koloru
	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );

	// Wlczanie testu glebokosci
	glEnable(GL_DEPTH_TEST);


	// -------------------------------------------------------
	// NOWE! Obliczanie macierzy rzutowania perspektywicznego
	// za pierwszym razem (po utworzeniu okna aplikacji)
	// -------------------------------------------------------
	if (windowHeight != 0)
		matProj = glm::perspective(glm::radians(80.0f), windowWidth/(float)windowHeight, 0.1f, 50.0f);


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
// funkcja zwrotna do obslugi zmiany rozmiaru framebuffera
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	windowWidth = width;
	windowHeight = height;


	// Viewport
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);

	// -------------------------------------------------------
	// NOWE! Obliczanie macierzy rzutowania perspektywicznego
	// Dlaczego akurat tutaj?
	// -------------------------------------------------------
	matProj = glm::perspective(glm::radians(70.0f), width/(float)height, 0.1f, 20.0f );
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


	// Inicjalizacja sceny
	Initialize();


	// Glowna petla
	while (!glfwWindowShouldClose(window))
	{
		// Obsluga zdarzen
		glfwPollEvents();

		// Render Sceny
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
