// -------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// -------------------------------------------------
// Przyklad startowy dla tematow
// - skybox
// - environmental mapping
// - rendering instancyjny
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

// Okno aplikacji
int windowWidth = 800, windowHeight = 800;
const char *windowTitle = "OpenGL w GLFW (Szablon)";

// Lokalne pliki naglowkowe
#include "utilities.hpp"
#include "objloader.hpp"

// ---------------------------------------
// NOWE: SkyBox
// ---------------------------------------
#include "skybox.hpp"


// Identyfikatory obiektow
GLuint idProgram;
GLuint idVAO;
GLuint idTexture;

// Macierze PVM
glm::mat4x4 matProj;
glm::mat4x4 matView;
glm::mat4x4 matModel = glm::mat4(1.0);



// Wektory na dane z pliku OBJ
std::vector<glm::vec3> OBJ_vertices;
std::vector<glm::vec2> OBJ_uvs;
std::vector<glm::vec3> OBJ_normals;


// ---------------------------------------
void DisplayScene()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


	// Obliczanie macierzy widoku i pozycji kamery
	matView = UpdateViewMatrix();
	glm::vec3 cameraPos = ExtractCameraPos(matView);


	glUseProgram( idProgram );


		// Przekazujemy zmienne uniform do potoku
		glUniformMatrix4fv( glGetUniformLocation( idProgram, "matProj" ), 1, GL_FALSE, glm::value_ptr(matProj) );
		glUniformMatrix4fv( glGetUniformLocation( idProgram, "matView" ), 1, GL_FALSE, glm::value_ptr(matView) );
		glUniformMatrix4fv( glGetUniformLocation( idProgram, "matModel" ), 1, GL_FALSE, glm::value_ptr(matModel) );
		glUniform3fv( glGetUniformLocation( idProgram, "cameraPos" ), 1, glm::value_ptr(cameraPos) );

		// Tekstury
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, idTexture);
		glUniform1i(glGetUniformLocation(idProgram, "tex0"), 0);

		// Renderowanie obiektu
		glBindVertexArray( idVAO );
		glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices.size() );
		glBindVertexArray( 0 );

	glUseProgram(0);

	// -------------------------------
	// NOWE: Rendering SkyBoxa
	// [odkomentowac]
	// -------------------------------
	//DrawSkyBox(matProj, matView);
}


// ---------------------------------------------------
void Initialize()
{
	// -------------------------------
	// NOWE: Inicjalizacja wszystkiego
	// co potrzebne do SkyBoxa
	// [odkomentowac]
	// -------------------------------
	// CreateSkyBox();

	// Ustawienie domyslnego odsuniecia kamery od polozenia (0,0,0)
	cameraTranslateX = 0.0;
	cameraTranslateY = 0.0;
	cameraTranslateZ = -3.0;

	// Inne ustawienia openGL i sceny
	glEnable( GL_DEPTH_TEST );
	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );

	if (windowHeight != 0)
		matProj = glm::perspective(glm::radians(70.0f), windowWidth/(float)windowHeight, 0.1f, 100.0f );


	// Potok
	idProgram = glCreateProgram();
	glAttachShader( idProgram, LoadShader(GL_VERTEX_SHADER, "shaders/vertex.glsl"));
	glAttachShader( idProgram, LoadShader(GL_FRAGMENT_SHADER, "shaders/fragment.glsl"));
	LinkAndValidateProgram( idProgram );


	// Tekstura
	int tex_width, tex_height, tex_n;
	unsigned char *tex_data;
	stbi_set_flip_vertically_on_load(true); // tylko raz w programie
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // tylko raz w programie
	tex_data = stbi_load("textures/koliber.jpg", &tex_width, &tex_height, &tex_n, 0);
	if (!tex_data) { printf("Texture image file error!\n"); }
	glGenTextures(1, &idTexture);
	glBindTexture(GL_TEXTURE_2D, idTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// OBJ
	if (!loadOBJ("obj/koliber.obj", OBJ_vertices, OBJ_uvs, OBJ_normals))
	{
		printf("Not loaded!\n");
		exit(1);
	}

	// VAO
	glGenVertexArrays( 1, &idVAO );
	glBindVertexArray( idVAO );
		// Wspolrzedne wierzcholkow
		GLuint vBuffer_pos;
		glGenBuffers( 1, &vBuffer_pos );
		glBindBuffer( GL_ARRAY_BUFFER, vBuffer_pos );
		glBufferData( GL_ARRAY_BUFFER, OBJ_vertices.size() * sizeof(glm::vec3), &OBJ_vertices[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		// Wspolrzedne textury UV
		GLuint vBuffer_uv;
		glGenBuffers( 1, &vBuffer_uv );
		glBindBuffer( GL_ARRAY_BUFFER, vBuffer_uv );
		glBufferData( GL_ARRAY_BUFFER, OBJ_uvs.size() * sizeof(glm::vec2), &OBJ_uvs[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
		// Wektory normalne
		GLuint vBuffer_normal;
		glGenBuffers( 1, &vBuffer_normal );
		glBindBuffer( GL_ARRAY_BUFFER, vBuffer_normal );
		glBufferData( GL_ARRAY_BUFFER, OBJ_normals.size() * sizeof(glm::vec3), &OBJ_normals[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 2 );
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
	matProj = glm::perspective(glm::radians(70.0f), width/(float)height, 0.1f, 100.0f );
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

		// Sprawdzanie bledow
		__CHECK_FOR_ERRORS

		// Render Sceny
		DisplayScene();

		glfwSwapBuffers(window);
	}


	// Cleaning
	glDeleteProgram( idProgram );


	return 0;
}
