// --------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// --------------------------------------------------
// Przyklad implementacji klasy CGround
//
// Zadanie:
// 1) ustaw poprawna wysokosc drzewa wzgledem podloza
// 2) zaimplementuj poruszanie sie postaci klawiszami
//    WSAD
// --------------------------------------------------
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


// Okno aplikacji
int windowWidth = 800, windowHeight = 800;
const char *windowTitle = "OpenGL (poruszanie postacia po podlozu)";

// Obiekty
enum {
	GROUND = 0,
	TREE,
	LEGO,
	NUMBER_OF_OBJECTS,
};

// ---------------------------------------
GLuint idProgram;
GLuint idVAO[NUMBER_OF_OBJECTS];
GLuint idTexture[NUMBER_OF_OBJECTS];

// ---------------------------------------
glm::mat4x4 matProj;
glm::mat4x4 matView;
glm::mat4x4 matModel = glm::mat4(1.0);


// ---------------------------------------
std::vector<glm::vec3> OBJ_vertices[NUMBER_OF_OBJECTS];
std::vector<glm::vec2> OBJ_uvs[NUMBER_OF_OBJECTS];
std::vector<glm::vec3> OBJ_normals[NUMBER_OF_OBJECTS];


// ---------------------------------------
// TODO: KROK 1: naglowki dwoch nowych klas
// CGround oraz CPlayer
// ---------------------------------------

//#include "ground.hpp"
//CGround myGround;

//#include "player.hpp"
//CPlayer myPlayer;


// ---------------------------------------
void DisplayScene()
{
	// Obliczanie macierzy widoku
	matView = UpdateViewMatrix();
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// potok
	glUseProgram( idProgram );

		// macierze
		glUniformMatrix4fv( glGetUniformLocation( idProgram, "matProj" ), 1, GL_FALSE, glm::value_ptr(matProj) );
		glUniformMatrix4fv( glGetUniformLocation( idProgram, "matView" ), 1, GL_FALSE, glm::value_ptr(matView) );

		// jednostka teksturujaca i uchwyt
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(idProgram, "tex0"), 0);

		// ------------------
		// Scena
		// ------------------
		matModel = glm::mat4(1.0);
		glBindVertexArray( idVAO[GROUND] );
			glUniformMatrix4fv( glGetUniformLocation( idProgram, "matModel" ), 1, GL_FALSE, glm::value_ptr(matModel) );
			glBindTexture(GL_TEXTURE_2D, idTexture[GROUND]);
			glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices[GROUND].size() );
		glBindVertexArray( 0 );

		// ------------------
		// Drzewo
		// ------------------
		// -------------------------------------------
		// TO_DO KROK 5: chcemy wykorzystac klase CGround
		// do poprawnego ustawienia wysokosci y obiektu
		// -------------------------------------------
		float x = 0.0;
		float z = 5.0;

		// TO_DO KROK 5: ustaw wysokosc wzgledem podloza
		float y = 0.0; //myGround.getY(glm::vec2(x, z));

		matModel = glm::translate(glm::mat4(1.0), glm::vec3(x, y, z));

		glBindVertexArray( idVAO[TREE] );
			glUniformMatrix4fv( glGetUniformLocation( idProgram, "matModel" ), 1, GL_FALSE, glm::value_ptr(matModel) );
			glBindTexture(GL_TEXTURE_2D, idTexture[TREE]);
			glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices[TREE].size() );
		glBindVertexArray( 0 );


		// ------------------
		// Lego
		// ------------------

		matModel = glm::mat4(1.0);
		glBindVertexArray( idVAO[LEGO] );
			glUniformMatrix4fv( glGetUniformLocation( idProgram, "matModel" ), 1, GL_FALSE, glm::value_ptr(matModel) );
			glBindTexture(GL_TEXTURE_2D, idTexture[LEGO]);
			glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices[LEGO].size() );
		glBindVertexArray( 0 );

		// ------------------------------------------------
		// TODO KROK 3: renderowanie Lego z poziomu CPlayer
		// ------------------------------------------------

		// myPlayer.Draw();



    glUseProgram(0);
}



// ---------------------------------------------------
void Initialize()
{
	// ustawienia openGL
	glEnable( GL_DEPTH_TEST );
	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );

	// Obliczanie matProj po raz pierwszy
	if (windowHeight != 0)
		matProj = glm::perspective(glm::radians(70.0f), windowWidth/(float)windowHeight, 0.1f, 100.0f );


	// Potok
	idProgram = glCreateProgram();
	glAttachShader( idProgram, LoadShader(GL_VERTEX_SHADER, "shaders/vertex.glsl"));
	glAttachShader( idProgram, LoadShader(GL_FRAGMENT_SHADER, "shaders/fragment.glsl"));
	LinkAndValidateProgram( idProgram );

	// Poczatkowe ustawienia kamery
	cameraTranslateX = 0.0f;
	cameraTranslateY = 0.0f;
	cameraTranslateZ = -12.0f;
	cameraRotateX = 0.3f;
	cameraRotateY = 1.0f;

	// Tylko raz w programie (na potrzeby tekstur)
	stbi_set_flip_vertically_on_load(true);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// ---------------------------------------
	// Scena
	{
		// Tekstura
		int tex_width, tex_height, tex_n;
		unsigned char *tex_data;
		tex_data = stbi_load("assets/chess.jpg", &tex_width, &tex_height, &tex_n, 0);

		glGenTextures(1, &idTexture[GROUND]);
		glBindTexture(GL_TEXTURE_2D, idTexture[GROUND]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


		// VAO z pliku OBJ
		if (!loadOBJ("assets/scene-plane.obj", OBJ_vertices[GROUND], OBJ_uvs[GROUND], OBJ_normals[GROUND]))
		{
			printf("Not loaded!\n");
			exit(1);
		}
		glGenVertexArrays( 1, &idVAO[GROUND] );
		glBindVertexArray( idVAO[GROUND] );
		GLuint vBuffer_pos, vBuffer_uv;
		glGenBuffers( 1, &vBuffer_pos );
		glBindBuffer( GL_ARRAY_BUFFER, vBuffer_pos );
		glBufferData( GL_ARRAY_BUFFER, OBJ_vertices[GROUND].size() * sizeof(glm::vec3), &(OBJ_vertices[GROUND])[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		glGenBuffers( 1, &vBuffer_uv );
		glBindBuffer( GL_ARRAY_BUFFER, vBuffer_uv );
		glBufferData( GL_ARRAY_BUFFER, OBJ_uvs[GROUND].size() * sizeof(glm::vec2), &(OBJ_uvs[GROUND])[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
	}

	// ---------------------------------------
	// Drzewo
	{
		// Tekstura
		int tex_width, tex_height, tex_n;
		unsigned char *tex_data;
		tex_data = stbi_load("assets/grass.jpg", &tex_width, &tex_height, &tex_n, 0);

		glGenTextures(1, &idTexture[TREE]);
		glBindTexture(GL_TEXTURE_2D, idTexture[TREE]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


		// VAO z pliku OBJ
		if (!loadOBJ("assets/tree.obj", OBJ_vertices[TREE], OBJ_uvs[TREE], OBJ_normals[TREE]))
		{
			printf("Not loaded!\n");
			exit(1);
		}
		glGenVertexArrays( 1, &idVAO[TREE] );
		glBindVertexArray( idVAO[TREE] );
		GLuint vBuffer_pos, vBuffer_uv;
		glGenBuffers( 1, &vBuffer_pos );
		glBindBuffer( GL_ARRAY_BUFFER, vBuffer_pos );
		glBufferData( GL_ARRAY_BUFFER, OBJ_vertices[TREE].size() * sizeof(glm::vec3), &(OBJ_vertices[TREE])[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		glGenBuffers( 1, &vBuffer_uv );
		glBindBuffer( GL_ARRAY_BUFFER, vBuffer_uv );
		glBufferData( GL_ARRAY_BUFFER, OBJ_uvs[TREE].size() * sizeof(glm::vec2), &(OBJ_uvs[TREE])[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
	}

	// Ludzik LEGO
	{
		// Tekstura
		int tex_width, tex_height, tex_n;
		unsigned char *tex_data;

		tex_data = stbi_load("assets/lego.png", &tex_width, &tex_height, &tex_n, 0);

		glGenTextures(1, &idTexture[LEGO]);
		glBindTexture(GL_TEXTURE_2D, idTexture[LEGO]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		// VAO z pliku OBJ
		if (!loadOBJ("assets/LEGO.obj", OBJ_vertices[LEGO], OBJ_uvs[LEGO], OBJ_normals[LEGO]))
		{
			printf("Not loaded!\n");
			exit(1);
		}

		glGenVertexArrays( 1, &idVAO[LEGO] );
		glBindVertexArray( idVAO[LEGO] );
		GLuint vBuffer_pos, vBuffer_uv;
		glGenBuffers( 1, &vBuffer_pos );
		glBindBuffer( GL_ARRAY_BUFFER, vBuffer_pos );
		glBufferData( GL_ARRAY_BUFFER, OBJ_vertices[LEGO].size() * sizeof(glm::vec3), &(OBJ_vertices[LEGO])[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		glGenBuffers( 1, &vBuffer_uv );
		glBindBuffer( GL_ARRAY_BUFFER, vBuffer_uv );
		glBufferData( GL_ARRAY_BUFFER, OBJ_uvs[LEGO].size() * sizeof(glm::vec2), &(OBJ_uvs[LEGO])[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
		glBindVertexArray( 0 );
	}



	// ----------------------------------------
	// TODO: KROK 2. Inicjalizujemy obiekt
	// klasy CGround podajac liste wierzcholkow
	// siatki wczytanej z pliku OBJ
	// ----------------------------------------

	//myGround.Init(OBJ_vertices[GROUND]);

	// ----------------------------------------
	// NOWE: Inicjalizujemy obiekt postaci
	// klasy CPlayer podajac referencje do
	// obiektu CGround, aby postac wiedziala
	// w jaki sposob wyliczac poprawna wysokosc
	// ----------------------------------------

	//myPlayer.Init(&myGround);

}




// ---------------------------------------------------
// NOWE: Obsluge klawiatury zaimplementujemy przy
// uzyciu tablicy booli, ktore beda ustawiane na
// - true w przypadku wcisniecia danego klawisza oraz
// - false w przypadku zwolnienia klawisza
// ---------------------------------------------------
// NOWE: Tablica przechowujaca boole do plynnej
// obslugi klawiatury
// ---------------------------------------------------
bool __keys[1024];
// funkcja zwrotna do obslugi klawiatury
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		__keys[key] = true;
	}

	if (action == GLFW_RELEASE)
	{
		__keys[key] = false;
	}
}

// ---------------------------------------------------
// funkcja zwrotna do obslugi klawiatury
// ---------------------------------------------------
// TODO KROK 4: Obsluge klawiatury przenosimy do nowej
// funkcji, ktora wywolamy w glownej petli programu.
// ---------------------------------------------------
void keyboard_handler()
{
	float speed = 0.1;	// a moze uzaleznic od FPS?
	float rotate = 0.1f;

//	if (__keys[GLFW_KEY_W])
//		myPlayer.Move(speed);
//
//	if (__keys[GLFW_KEY_S])
//		myPlayer.Move(-speed);
//
//	if (__keys[GLFW_KEY_D])
//		myPlayer.Rotate(-rotate);
//
//	if (__keys[GLFW_KEY_A])
//		myPlayer.Rotate(rotate);

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
	glDeleteVertexArrays( NUMBER_OF_OBJECTS, &idVAO[0] );
	glDeleteTextures(NUMBER_OF_OBJECTS, &idTexture[0]);

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);

	return 0;
}
