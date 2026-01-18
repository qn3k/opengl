// -------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// -------------------------------------------------
// Przyklad startowy dla tematow
// - cienie metoda shadow mapping
// -------------------------------------------------
#include <stdio.h>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Obsluga plikow graficznych
# define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Biblioteki GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Lokalne pliki naglowkowe
#include "utilities.hpp"
#include "objloader.hpp"

// ---------------------------------------
// NOWE: Plik naglowkowy z funkcjami do
// generowania cieni swiatla kierunkowego
// ---------------------------------------
#include "shadow-dir.hpp"

// Okno aplikacji
int windowWidth = 800, windowHeight = 800;
const char *windowTitle = "OpenGL (shadow mapping - nacisnij SPACE)";


// Identyfikatory
GLuint idProgram;
GLuint idVAO;
GLuint idVBO_pos;
GLuint idVBO_normal;


// Macierze
glm::mat4x4 matPVM;
glm::mat4x4 matProj;
glm::mat4x4 matView;
glm::mat4x4 matModel = glm::mat4(1.0);


// Pliki OBJ
std::vector<glm::vec3> OBJ_vertices;
std::vector<glm::vec3> OBJ_normals;
std::vector<glm::vec2> OBJ_uvs;

bool isShadowMapping = false;

// -------------------------------------------------------
// NOWE: Rendering do shadow mapy
// -------------------------------------------------------
void RenderScene_to_ShadowMap()
{
	// 1. Renderowanie z pozycji swiatla do textury DepthMap
	glViewport(0, 0, DepthMap_Width, DepthMap_Height);
	glBindFramebuffer(GL_FRAMEBUFFER, DepthMap_idFrameBuffer);
	glClear(GL_DEPTH_BUFFER_BIT);


	// AKTYWUJEMY program
	glUseProgram( DepthMap_idProgram );

		// Przesylamy macierze rzutowania i kamery pozaekranowej
		glUniformMatrix4fv( glGetUniformLocation( DepthMap_idProgram, "matProj" ), 1, GL_FALSE, glm::value_ptr(lightProj) );
		glUniformMatrix4fv( glGetUniformLocation( DepthMap_idProgram, "matView" ), 1, GL_FALSE, glm::value_ptr(lightView) );

		// Teraz renderujemy wszystkie obiekty
		// ktore maja rzucac cien (generowac cien w shadow mapie)

		glUniformMatrix4fv( glGetUniformLocation( DepthMap_idProgram, "matModel" ), 1, GL_FALSE, glm::value_ptr(matModel) );
		glBindVertexArray( idVAO );
		glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices.size() );
		glBindVertexArray( 0 );


	// WYLACZAMY program
	glUseProgram(0);
}

// -------------------------------------------------------
void RenderScene_on_Screen()
{
	// 2. Renderowanie z pozycji kamery na ekran
	glViewport(0, 0, windowWidth, windowHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Obliczanie macierzy widoku
	matView = UpdateViewMatrix();

	// AKTYWUJEMY program
	glUseProgram( idProgram );

		// Obliczenie macierzy PV do potoku
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(matProj));
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(matView));

		// ----------------------------------------
		// NOWE: Przeslanie macierzy oswietlenia
		// do wyliczenia wspolczynnika cienia
		// ----------------------------------------
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "lightProj"), 1, GL_FALSE, glm::value_ptr(lightProj));
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "lightView"), 1, GL_FALSE, glm::value_ptr(lightView));

		// Przeslanie danych do swiatla kierunkowego
		glUniform3fv( glGetUniformLocation( idProgram, "lightDirection" ), 1, glm::value_ptr(lightDirection) );

		// -----------------------------------------------
		// NOWE: ustawiamy teksture shadow mapy na slot 2
		// -----------------------------------------------
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, DepthMap_idTexture);
		glUniform1i(glGetUniformLocation( idProgram, "tex_shadowMap" ), 2);


		// RENDERING: Scena
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModel));
		glBindVertexArray( idVAO);
		glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices.size() );
		glBindVertexArray( 0 );


	// WYLACZAMY program
	glUseProgram(0);
}

// -------------------------------------------------------
void DisplayScene()
{
	// -------------------------------------------
	// NOWE: 1. Rendering pozaekranowy do mapy
	// cienia z polozenia swiatla
	// -------------------------------------------
	if (isShadowMapping)
		RenderScene_to_ShadowMap();

	// 2. Rendering na ekran z polozenia kamery
	RenderScene_on_Screen();
}

// ---------------------------------------------------
void Initialize()
{
	// Ustawienia OpenGL i sceny
	glEnable( GL_DEPTH_TEST );
	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );

	// Obliczanie matProj po raz pierwszy
	if (windowHeight != 0)
		matProj = glm::perspective(glm::radians(70.0f), windowWidth/(float)windowHeight, 0.1f, 100.0f );


	// Ladowanie pliku OBJ
	if (!loadOBJ("assets/scene.obj", OBJ_vertices, OBJ_uvs, OBJ_normals))
	{
		printf("Not loaded!\n");
		exit(1);
	}

	// Potok OpenGL
	idProgram = glCreateProgram();
	glAttachShader( idProgram, LoadShader(GL_VERTEX_SHADER, "shaders/vertex.glsl"));
	glAttachShader( idProgram, LoadShader(GL_FRAGMENT_SHADER, "shaders/fragment.glsl"));
	LinkAndValidateProgram( idProgram );

	// SCENA
	glGenVertexArrays( 1, &idVAO );
	glBindVertexArray( idVAO );
		glGenBuffers( 1, &idVBO_pos );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_pos );
		glBufferData( GL_ARRAY_BUFFER, OBJ_vertices.size() * sizeof(glm::vec3), &(OBJ_vertices)[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		glGenBuffers( 1, &idVBO_normal );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_normal );
		glBufferData( GL_ARRAY_BUFFER, OBJ_normals.size() * sizeof(glm::vec3), &(OBJ_normals)[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
	glBindVertexArray( 0 );


	// ------------------------------------------------------------
	// NOWE: Utworzenie wszystkiego co potrzebne jest do mapy cieni
	// ------------------------------------------------------------
	ShadowMapDir_Init();
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

		// Sprawdzanie bledow
		__CHECK_FOR_ERRORS

		// Render Sceny
		DisplayScene();

		glfwSwapBuffers(window);
	}

	// Cleaning
	glDeleteProgram( idProgram );
	glDeleteVertexArrays( 1, &idVAO );
	glDeleteBuffers(1, &idVBO_pos);
	glDeleteBuffers(1, &idVBO_normal);

	// -----------------------------------------
	// NOWE: Czyszczenie tego co stworzylismy
	// w ramach shadow mappingu
	// -----------------------------------------
	ShadowMapDir_Clean();

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);

	return 0;
}
