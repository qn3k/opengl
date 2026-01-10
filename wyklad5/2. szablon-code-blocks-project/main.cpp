// -------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// ------------------------------------------------
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
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Biblioteki GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Obsluga plikow graficznych
# define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Okno aplikacji
int windowWidth = 600, windowHeight = 600;
const char *windowTitle = "OpenGL w GLFW (Szablon)";

// Lokalne pliki naglowkowe
#include "utilities.hpp"
#include "objloader.hpp"

// Macierze transformacji i rzutowania
glm::mat4 matProj;
glm::mat4 matView;
glm::mat4 matModel;

// Zmienna czas do animacji
float Time = 0.0;


// Wektory na dane z pliku OBJ
std::vector<glm::vec3> OBJ_vertices;
std::vector<glm::vec2> OBJ_uvs;
std::vector<glm::vec3> OBJ_normals;


// Identyfikatory obiektow OpenGLa
GLuint idProgram;	// programu
GLuint idVBO;		// bufora wierzcholkow
GLuint idVAO;		// tablic wierzcholkow
GLuint idVBO_coord; // bufory
GLuint idVBO_color; // bufory


// ---------------------------------------
void DisplayScene()
{
	Time += 0.01;

	// Czyszczenie bufora koloru i glebokosci
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Aktualizacja macierzy widoku (obsluga myszy)
	matView = UpdateViewMatrix();


	// Aktywowanie potoku
	glUseProgram( idProgram );


		// Przekazanie macierzy
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(matProj));
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(matView));


		// Obiekt z pliku OBJ
		matModel = glm::mat4(1.0);
		matModel = glm::rotate(matModel, Time, glm::vec3(0,1,0));
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModel));

		// Rendering
		glBindVertexArray( idVAO );
		glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices.size() );
		glBindVertexArray( 0 );


	// Wylaczanie
	glUseProgram( 0 );
}

// ---------------------------------------
void Initialize()
{
	// Pierwsze ustalanie macierzy rzytowania
	if (windowHeight != 0)
		matProj = glm::perspective(glm::radians(80.0f), windowWidth/(float)windowHeight, 0.1f, 50.0f);


	// Wczytanie pliku obj
	if (!loadOBJ("assets/scene.obj", OBJ_vertices, OBJ_uvs, OBJ_normals))
	{
		printf("Not loaded!\n");
		exit(1);
	}

	// Potok
	idProgram = glCreateProgram();
	glAttachShader( idProgram, LoadShader(GL_VERTEX_SHADER, "shaders/vertex.glsl"));
	glAttachShader( idProgram, LoadShader(GL_FRAGMENT_SHADER, "shaders/fragment.glsl"));
	LinkAndValidateProgram( idProgram );

	// VAO
	glGenVertexArrays( 1, &idVAO );
	glBindVertexArray( idVAO );
		// Bufor na wspolrzedne wierzcholkow
		glGenBuffers( 1, &idVBO_coord );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_coord );
		glBufferData( GL_ARRAY_BUFFER, OBJ_vertices.size() * sizeof(glm::vec3), &OBJ_vertices[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		// Bufor na kolory
		glGenBuffers( 1, &idVBO_color );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_color );
		glBufferData( GL_ARRAY_BUFFER, OBJ_normals.size() * sizeof(glm::vec3), &OBJ_normals[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
	glBindVertexArray( 0 );

	// Test glebokosci
	glEnable(GL_DEPTH_TEST);

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

		default:
			printf("Nacisnieto klawisz %d \n", key);
			break;
		}
	}
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

	// Macierz rzutowania perspektywicznego
	matProj = glm::perspective(glm::radians(70.0f), width/(float)height, 0.1f, 20.0f );
}

// ---------------------------------------------------
int main( int argc, char *argv[] )
{
	// Kontekst i okno aplikacji
    GLFWwindow* window = nullptr;
	Initialize_GLFW(window);


	// Inicjalizacja
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
	glDeleteVertexArrays(1, &idVAO);
	glDeleteBuffers(1, &idVBO_coord);
	glDeleteBuffers(1, &idVBO_color);
	glDeleteProgram(idProgram);
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
