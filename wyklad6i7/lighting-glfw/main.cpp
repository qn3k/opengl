// -------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// -------------------------------------------------
// Przyklad obslugi myszy w glfw do obslugi
// polozenia i orientacji kamery (macierzy matView)
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
// Opengl 4.5 i nalezy go wlasnym plikiem glad.c
// pobranym z powyzszej strony.
// -------------------------------------------------
#include <stdio.h>
#include <vector>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// -----------------------------------------
// NOWE: plik do obslugi plikow graficznych
// -----------------------------------------
# define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Macierze PVM
glm::mat4 matProj;
glm::mat4 matView;
glm::mat4 matModel = glm::mat4(1.0);

// Lokalne pliki naglowkowe
#include "utilities.hpp"
#include "objloader.hpp"




// Identyfikatory obiektow
GLuint idProgram;
GLuint idVAO;

// Wektory na dane z pliku OBJ
std::vector<glm::vec3> OBJ_vertices;
std::vector<glm::vec2> OBJ_uvs;
std::vector<glm::vec3> OBJ_normals;



// ---------------------------------------------------
void DisplayScene()
{
	// Czyszczenie bufora koloru i glebokosci
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// -----------------------------------------------
	// NOWE : makro do wykrywania prostych bledow
	// -----------------------------------------------
	__CHECK_FOR_ERRORS

	// ------------------------------------------------------------
	// Wyliczanie macierz widoku
	// ------------------------------------------------------------
	matView = UpdateViewMatrix();

	// -----------------------------------------------
	// NOWE : wyliczanie pozycji kamery z matView
	// -----------------------------------------------
	glm::vec3 cameraPos = ExtractCameraPos(matView);



	// Wlaczenie programu
	glUseProgram( idProgram );


		// Przekazanie macierzy rzutowania i widoku
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(matProj));
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(matView));

		// Przekazanie pozycji kamery
		glUniform3fv( glGetUniformLocation( idProgram, "cameraPos" ), 1, &cameraPos[0] );



		// Przekazanie macierzy modelu obiektu
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModel));

		// Rendering
		glBindVertexArray( idVAO );
		glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices.size() );
		glBindVertexArray( 0 );


	// Wylaczanie
	glUseProgram( 0 );

}

// ---------------------------------------------------
void Initialize()
{
	// Ustawienia globalne
	glEnable(GL_DEPTH_TEST);
	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );

	// ------------------------------------------------------------
	// Ustawienie domyslnego odsuniecia kamery od polozenia (0,0,0)
	// ------------------------------------------------------------
	CameraTranslate_x = 0.0;
	CameraTranslate_y = -1.0;
	CameraTranslate_z = -12.0;

	 if (windowHeight != 0)
        matProj = glm::perspective(glm::radians(80.0f), windowWidth/(float)windowHeight, 0.1f, 50.0f);



	// Wczytanie pliku obj
	if (!loadOBJ("scene.obj", OBJ_vertices, OBJ_uvs, OBJ_normals))
	{
		printf("Not loaded!\n");
		exit(1);
	}

	// Potok
	idProgram = glCreateProgram();
	glAttachShader( idProgram, LoadShader(GL_VERTEX_SHADER, "vertex.glsl"));
	glAttachShader( idProgram, LoadShader(GL_FRAGMENT_SHADER, "fragment.glsl"));
	LinkAndValidateProgram( idProgram );


	// VAO
	glGenVertexArrays( 1, &idVAO );
	glBindVertexArray( idVAO );
		// Bufor na wspolrzedne wierzcholkow
		GLuint idVBO_coord;
		glGenBuffers( 1, &idVBO_coord );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_coord );
		glBufferData( GL_ARRAY_BUFFER, OBJ_vertices.size() * sizeof(glm::vec3), &OBJ_vertices[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		// Bufor na wspolrzedne UV
		GLuint idVBO_uv;
		glGenBuffers( 1, &idVBO_uv );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_uv );
		glBufferData( GL_ARRAY_BUFFER, OBJ_uvs.size() * sizeof(glm::vec2), &OBJ_uvs[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
		// -----------------------------------------------
		// NOWE : Bufor na wektory normalne
		// -----------------------------------------------
		GLuint idVBO_normal;
		glGenBuffers( 1, &idVBO_normal );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_normal );
		glBufferData( GL_ARRAY_BUFFER, OBJ_normals.size() * sizeof(glm::vec3), &OBJ_normals[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 2 );
	glBindVertexArray( 0 );

}

// ---------------------------------------------------
int main( int argc, char *argv[] )
{
	// Funkcja zwrotna do bledow
	glfwSetErrorCallback(error_callback);
	// Inicjalizacja glfw
	if (!glfwInit()) exit(EXIT_FAILURE);
	// Ustalenie wersji OpenGL na 4.5
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif


	// Okno aplikacji i kontekst
    GLFWwindow* window = nullptr;
	window = glfwCreateWindow((int)(windowWidth), (int)(windowHeight), "Szablon OpenGL3+glfw", nullptr, nullptr);
	if (window == nullptr) return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // vsync
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("Failed to initialize GLAD\n");
		return -1;
	}

	// Funkcje zwrotne
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);


	// Inicjalizacja
	Initialize();


	// Glowna petla
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// Render Sceny
		DisplayScene();

		glfwSwapBuffers(window);
	}


	// Cleaning
	glDeleteProgram( idProgram );


	return 0;
}
