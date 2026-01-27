// ---------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// ---------------------------------------------------
// Przyklad programu wyswietlajacego tekst
// na ekranie za pomoca biblioteki freetype
// Wiecej na
// https://learnopengl.com/In-Practice/Text-Rendering
// ---------------------------------------------------
#include <stdio.h>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Biblioteki GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Lokalne pliki naglowkowe
#include "utilities.hpp"
#include "objloader.hpp"

// ---------------------------------------
// NOWE: funkcje do obslugi tekstu
#include "text-ft.hpp"
// ---------------------------------------

// Okno aplikacji
GLFWwindow* window = nullptr;
int windowWidth = 800, windowHeight = 800;
const char *windowTitle = "OpenGL (tekst na ekranie)";

GLuint idProgram;
GLuint idVAO;

glm::mat4x4 matProj;
glm::mat4x4 matView;
glm::mat4x4 matModel = glm::mat4(1.0);


std::vector<glm::vec3> OBJ_vertices;
std::vector<glm::vec2> OBJ_uvs;
std::vector<glm::vec3> OBJ_normals;

// Zmienna zliczajaca klatki
int frame = 0;

// ---------------------------------------
void DisplayScene()
{
	__CHECK_FOR_ERRORS

	// Obliczanie macierzy widoku
	matView = UpdateViewMatrix();
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


	glUseProgram( idProgram );

		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matProj"), 1, GL_FALSE,  glm::value_ptr(matProj));
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE,  glm::value_ptr(matView));
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE,  glm::value_ptr(matModel));

		// Generowanie obiektow na ekranie
		glBindVertexArray( idVAO );
		glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices.size() );
		glBindVertexArray( 0 );

	glUseProgram( 0 );


	// -----------------------------------
	// NOWE: Generowanie tekstu na ekranie
	// -----------------------------------
	char txt[255];
	sprintf(txt, "Frame: %d", frame++);

	RenderText(txt, 25, 25, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
	RenderText("ESC - Exit", 25, 750, 0.5f, glm::vec3(1.0, 0.0f, 0.0f));

	RenderText("Napis", 25, 70, 0.5f, glm::vec3(0.1, 0.1, 1.0f));
}



// ---------------------------------------------------
void Initialize()
{
	// --------------------------------------------
	// NOWE:
	// Inicjalizacja programu do generowania tekstu
	// --------------------------------------------
    InitText("assets/arial.ttf", 36);


    // Dalej standardowo
	if (!loadOBJ("assets/scene.obj", OBJ_vertices, OBJ_uvs, OBJ_normals))
	{
		printf("Not loaded!\n");
		exit(1);
	}

	// Obliczanie matProj po raz pierwszy
	if (windowHeight != 0)
		matProj = glm::perspective(glm::radians(70.0f), windowWidth/(float)windowHeight, 0.1f, 100.0f );

	idProgram = glCreateProgram();
	glAttachShader( idProgram, LoadShader(GL_VERTEX_SHADER, "shaders/vertex.glsl"));
	glAttachShader( idProgram, LoadShader(GL_FRAGMENT_SHADER, "shaders/fragment.glsl"));
	LinkAndValidateProgram( idProgram );
	glGenVertexArrays( 1, &idVAO );
	glBindVertexArray( idVAO );
	GLuint vBuffer_coord;
	glGenBuffers( 1, &vBuffer_coord );
	glBindBuffer( GL_ARRAY_BUFFER, vBuffer_coord );
	glBufferData( GL_ARRAY_BUFFER, OBJ_vertices.size() * sizeof(glm::vec3), &OBJ_vertices[0], GL_STATIC_DRAW );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
	glEnableVertexAttribArray( 0 );
	glEnable( GL_DEPTH_TEST );
	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
}

// --------------------------------------------------------------
void keyboard_handler()
{
	// tym razem nic tu nie ma
}

// ---------------------------------------------------
int main( int argc, char *argv[] )
{
	// Kontekst i okno aplikacji
	Initialize_GLFW(window);

	// Inicjalizacja sceny
	Initialize();

	// Glowna petla
	while (!glfwWindowShouldClose(window))
	{
		// Obsluga zdarzen
		glfwPollEvents();

		// Plynna obsluga klawiatury
		keyboard_handler();

		// Sprawdzanie bledow
		__CHECK_FOR_ERRORS

		// Render Sceny
		DisplayScene();

		glfwSwapBuffers(window);
	}

	// Cleaning
	glDeleteProgram( idProgram );



	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);

	return 0;
}
