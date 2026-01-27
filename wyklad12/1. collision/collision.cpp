// ------------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// ------------------------------------------------------
// Przyklad implementacji klasy CSceneObject
// obslugujacej obiekty CMesh wraz z koliderem CCollider
// ------------------------------------------------------
#include <stdio.h>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Obsluga plikow graficznych
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Biblioteki GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Lokalne pliki naglowkowe
#include "utilities.hpp"
#include "objloader.hpp"


// Klasa CColider i pochodne
#include "collider.hpp"

// Klasa CMesh i CSceneObject
// dziedziczaca po CMesh
#include "mesh.hpp"
#include "scene-object.hpp"

// Klasa CPlayer
#include "player.hpp"

// Okno aplikacji
GLFWwindow* window = nullptr;
int windowWidth = 800, windowHeight = 800;
const char *windowTitle = "OpenGL (test kolizji)";

glm::mat4x4 matProj;
glm::mat4x4 matView;
glm::mat4x4 matModel = glm::mat4(1.0);


// Identyfikatory obiektow
GLuint idProgram;

// ---------------------------------------
// NOWE: Obecnie mamy 3 rodzaje obiektow:
// ---------------------------------------

// A. Obiekty do renderowania - zwykle meshe
CMesh m_ground;
CMesh m_stone;
CMesh m_ufo;


// B. Obiekty sceny z polozeniem/orientacja
// ktore moga miec collidery lub nie
CSceneObject ground; 	// ten nie ma collidera
CSceneObject stone;		// a ten bedzie mial

// C. Obiekt postaci, ktory udostepnia metody
// poruszania sie po scenie wraz z testem kolizji
CPlayer myPlayer;


// ---------------------------------------
void DisplayScene()
{
	__CHECK_FOR_ERRORS

	// Obliczanie macierzy widoku
	matView = UpdateViewMatrix();
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glUseProgram( idProgram );

		// Wyslanie macierzy rzutowania
		glUniformMatrix4fv( glGetUniformLocation( idProgram, "matProj" ), 1, GL_FALSE, glm::value_ptr(matProj) );
		glUniformMatrix4fv( glGetUniformLocation( idProgram, "matView" ), 1, GL_FALSE, glm::value_ptr(matView) );

		// GROUND
		ground.Draw();

		// STONE
		stone.Draw();

		// Rendering postaci, ktora sie poruszamy
		myPlayer.Draw();

	glUseProgram( 0 );
}



// ---------------------------------------------------
void Initialize()
{
	// Tylko raz w programie (na potrzeby tekstur)
	stbi_set_flip_vertically_on_load(true);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// ustawienia openGL
	glEnable( GL_DEPTH_TEST );
	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );

	// Obliczanie matProj po raz pierwszy
	if (windowHeight != 0)
		matProj = glm::perspective(glm::radians(70.0f), windowWidth/(float)windowHeight, 0.1f, 100.0f );


	// potok
	idProgram = glCreateProgram();
	glAttachShader( idProgram, LoadShader(GL_VERTEX_SHADER, "shaders/vertex.glsl"));
	glAttachShader( idProgram, LoadShader(GL_FRAGMENT_SHADER, "shaders/fragment.glsl"));
	LinkAndValidateProgram( idProgram );


	// A. Inicjalizacja meshy
	m_ground.Init("assets/chess.obj", "assets/chess.jpg");
	m_stone.Init("assets/sphere.obj", "assets/grass.jpg");
	m_ufo.Init("assets/ufo.obj", "assets/ufo.jpg");


	// B. Inicjalizacja obiektow sceny
	ground.Init(&m_ground);

	// z ewentualna pozycja i colliderem
	stone.Init(&m_stone);
	stone.setPosition(glm::vec3(3.0, 0.0, 1.0));
	stone.setCollider(new CSphereCollider(stone.getPosition(), 1.0f));

	// C. Inicjalizacja playera
	myPlayer.Init(&m_ufo);
	myPlayer.setPosition(glm::vec3(-1.0, 0.0, 0.0));
	myPlayer.setCollider(new CSphereCollider(myPlayer.getPosition(), 0.7f));
}


// --------------------------------------------------------------
void keyboard_handler()
{
	// Predkosc poruszania sie i obrotu
	float speed  = 0.1f;	// a moze uzaleznic od FPS?
	float rotate = 0.1f;

	// ----------------------------------------------
	// NOWE: poruszanie sie postacia za pomoca
	// nowej metody Move(), ktora dodatkowo sprawdza
	// test kolizji
	// ----------------------------------------------
	// TODO: obecnie obslugujemy tylko jeden kolider
	// a bedziemy potrzebowali wiecej colliderow
	// ----------------------------------------------

	if (__keys[GLFW_KEY_W])
		myPlayer.Move(speed, stone);

	if (__keys[GLFW_KEY_S])
		myPlayer.Move(-speed, stone);

	if (__keys[GLFW_KEY_D])
		myPlayer.Rotate(-rotate);

	if (__keys[GLFW_KEY_A])
		myPlayer.Rotate(rotate);
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

		// -------------------------------
		// NOWE: Plynna obsluga klawiatury
		// -------------------------------
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
