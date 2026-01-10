// -------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// -------------------------------------------------
// Przyklad uzycia tekstury z kanalem alpha
// do usuwania geometrii (przezroczysty kwiatek)
// mechanizmem discard w shaderze fragmentow.
// -------------------------------------------------
#include <stdio.h>
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

// Okno aplikacji
int windowWidth = 600, windowHeight = 600;
const char *windowTitle = "OpenGL (Tekstura przezroczysta)";

// Lokalne pliki naglowkowe
#include "utilities.hpp"


// Macierze transformacji i rzutowania
glm::mat4 matProj = glm::mat4(1.0);
glm::mat4 matView = glm::mat4(1.0);
glm::mat4 matModel = glm::mat4(1.0);

// Identyfikatory obiektow
GLuint idProgram;
GLuint idVAO;
GLuint idVBO_coord;
GLuint idVBO_color;
GLuint idVBO_uv;

// Identyfikator obiektu tekstury
GLuint idTexture;

// Lokalizacje zmiennych jednorodnych
GLint loc_matProj = -1;
GLint loc_matView = -1;
GLint loc_matModel = -1;
GLint loc_tex = -1;


// Wspolrzedne dwoch kwadratow
GLfloat vertices[12*3] =
{
	-1.0, -1.0, 0.0,
	1.0, 1.0, 0.0,
	1.0, -1.0, 0.0,
	-1.0, -1.0, 0.0,
	-1.0, 1.0, 0.0,
	1.0, 1.0, 0.0,

	0.0, -1.0, 1.0,
	0.0, 1.0, -1.0,
	0.0, -1.0, -1.0,
	0.0, -1.0, 1.0,
	0.0, 1.0, 1.0,
	0.0, 1.0, -1.0,
};

// Wspolrzedne UV dwoch kwadratow
GLfloat uvs[12*2] = {
	0.00, 0.00,
	0.98, 0.98,
	0.98, 0.00,
	0.00, 0.00,
	0.00, 0.98,
	0.98, 0.98,

	0.98, 0.00,
	0.00, 0.98,
	0.00, 0.00,
	0.98, 0.00,
	0.98, 0.98,
	0.00, 0.98,
};

// ---------------------------------------
void DisplayScene()
{
	// Czyszczenie bufora koloru i glebokosci
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Obliczanie macierzy widoku
	matView = UpdateViewMatrix();

	// Wlaczenie programu
	glUseProgram( idProgram );

		// Przekazanie macierzy
		glUniformMatrix4fv( loc_matProj, 1, GL_FALSE, glm::value_ptr(matProj));
		glUniformMatrix4fv( loc_matView, 1, GL_FALSE, glm::value_ptr(matView));
		glUniformMatrix4fv( loc_matModel, 1, GL_FALSE, glm::value_ptr(matModel));

		// Aktywowanie jednostki teksturujacej nr 0,
		// przypisanie do niej obiektu tekstury oraz
		// polaczenie j.t. z uchwytem tekstury w shaderze
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, idTexture);
		glUniform1i(loc_tex, 0);

		// Rendering
		glBindVertexArray( idVAO );
		glDrawArrays( GL_TRIANGLES, 0, 12 );
		glBindVertexArray( 0 );


	// Wylaczanie
	glUseProgram( 0 );

}

// ---------------------------------------
void Initialize()
{

	// Ustawienia maszyny OpenGL
	glEnable(GL_DEPTH_TEST);
	glClearColor( 0.7f, 0.7f, 0.7f, 1.0f );

	// Obliczanie macierzy rzutowania perspektywicznego
	// za pierwszym razem (po utworzeniu okna aplikacji)
	if (windowHeight != 0)
		matProj = glm::perspective(glm::radians(80.0f), windowWidth/(float)windowHeight, 0.1f, 50.0f);


	// Potok
	idProgram = glCreateProgram();
	glAttachShader( idProgram, LoadShader(GL_VERTEX_SHADER, "shaders/vertex.glsl"));
	glAttachShader( idProgram, LoadShader(GL_FRAGMENT_SHADER, "shaders/fragment.glsl"));
	LinkAndValidateProgram( idProgram );

	// Pobranie lokalizacji zmiennych jednorodnych
	glUseProgram(idProgram);
		loc_matProj = glGetUniformLocation(idProgram, "matProj");
		loc_matView = glGetUniformLocation(idProgram, "matView");
		loc_matModel = glGetUniformLocation(idProgram, "matModel");
		loc_tex = glGetUniformLocation(idProgram, "tex");
	glUseProgram(0);

	// VAO
	glGenVertexArrays( 1, &idVAO );
	glBindVertexArray( idVAO );

		// Bufor na wspolrzedne wierzcholkow
		glGenBuffers( 1, &idVBO_coord );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_coord );
		glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );

		// -------------------------------------
		// NOWE: Bufor na wspolrzedne tekstury
		// -------------------------------------
		glGenBuffers( 1, &idVBO_uv );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_uv );
		glBufferData( GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );

	glBindVertexArray( 0 );


	// ------------------------------------
	// NOWE: Wczytanie pliku tekstury RGBA
	// ------------------------------------
	int tex_width, tex_height, tex_n;
	unsigned char *tex_data;

	// ustawienie odwracania wczytywania obrazow
	stbi_set_flip_vertically_on_load(true);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


	// wczytanie pliku graficznego
	tex_data = stbi_load ("assets/flower32bit.png", &tex_width , &tex_height , &tex_n, 0);
	if ( tex_data == NULL) {
		printf ("Image can't be loaded!\n");
		exit(1);
	}

	// Utworzenie obiektu tekstury RGBA
	glGenTextures(1, &idTexture);
	glBindTexture(GL_TEXTURE_2D, idTexture);
	// ----------------------------------------------
	// NOWE: Zauwazmy ze wczytujemy plik w formacie
	// GL_RGBA i taki tez bedziemy przechowywac w
	// obiekcie tekstury OpenGL
	// ----------------------------------------------
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


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



	// Cleaning
	glDeleteProgram( idProgram );
	glDeleteVertexArrays( 1, &idVBO_coord );
	glDeleteVertexArrays( 1, &idVBO_color );
	glDeleteVertexArrays( 1, &idVBO_uv );
	glDeleteVertexArrays( 1, &idVAO );
	glDeleteTextures(1, &idTexture );

	return 0;
}
