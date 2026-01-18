// -------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// -------------------------------------------------
// Przyklad startowy dla tematow
// - rendering pozaekranowy
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


// Okno aplikacji
int windowWidth = 800, windowHeight = 800;
const char *windowTitle = "OpenGL (Rendering pozaekranowy - nacisnij SPACE)";


// Obiekty na scenie
enum {
	SCENE = 0,
	SCREEN,
	NUMBER_OF_OBJECTS,
};

// Identyfikatory
GLuint idProgram;
GLuint idVAO[NUMBER_OF_OBJECTS];
GLuint idVAOpos[NUMBER_OF_OBJECTS];
GLuint idVAOuv[NUMBER_OF_OBJECTS];
GLuint idTexture[NUMBER_OF_OBJECTS];

// --------------------------------------
// NOWE: Skladowe potrzebne do renderingu
// pozaekranowego do tekstury
// --------------------------------------
GLuint idFrameBuffer;	// FBO - Frame buffer object
GLuint idDepthBuffer;	// RBO - Render buffer object na skladowa glebokosci
GLuint idTextureBuffer;	// Obiekt tekstury na skladowa koloru
int bufferWidth = 800; // wymiary FBO
int bufferHeight = 600;
glm::mat4 screenMatView; // Macierz widoku renderingu pozaekranowego
// --------------------------------------


// Macierze PVM
glm::mat4x4 matPVM;
glm::mat4x4 matProj;
glm::mat4x4 matView;
glm::mat4x4 matModel = glm::mat4(1.0);

// Lokalizacje zmiennych jednorodnych
GLint loc_matPVM;
GLint loc_tex;


// Do plikow OBJ
std::vector<glm::vec3> OBJ_vertices[2];
std::vector<glm::vec2> OBJ_uvs[2];
std::vector<glm::vec3> OBJ_normals[2];


// Kwadrat - wspolrzedne
GLfloat vertices_pos[] =
{
	-1.0f, -1.0f, 0.0f,
	 2.0f, -1.0f, 0.0f,
	 2.0f,  2.0f, 0.0f,
	 2.0f,  2.0f, 0.0f,
	-1.0f,  2.0f, 0.0f,
	-1.0f, -1.0f, 0.0f,
};

// Kwadrat - uv
GLfloat vertices_tex[] =
{
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	1.0f, 1.0f,
	0.0f, 1.0f,
	0.0f, 0.0f,
};


bool isOffscreenRendering = false;


// ---------------------------------------
// NOWE: Rendering do tekstury
// ---------------------------------------
void RenderScene_to_Texture()
{
	// Aktywacja utworzonego wczesniej FBO
	// Ustawienie viewporta
	// Wyczyszczenie buforow koloru i glebokosci
	glViewport(0, 0, bufferWidth, bufferHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, idFrameBuffer);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// AKTYWUJEMY program
	glUseProgram( idProgram );


		// Obliczanie macierzy widoku kamery "pozaekrnowej"
		screenMatView = glm::mat4(1.0);
		screenMatView = glm::rotate(screenMatView, 0.2f, glm::vec3(1.0, 0.0, 0.0));

		// Obliczenie macierzy PVM i przeslanie do potoku
		matPVM = matProj * screenMatView * matModel;
		glUniformMatrix4fv( loc_matPVM, 1, GL_FALSE, glm::value_ptr(matPVM) );

		// Tekstura sceny
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, idTexture[SCENE]);
		glUniform1i(loc_tex, 0);

		// Scena
		glBindVertexArray( idVAO[SCENE] );
		glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices[SCENE].size() );
		glBindVertexArray( 0 );

		// Ekran
		glBindVertexArray( idVAO[SCREEN] );
		glDrawArrays( GL_TRIANGLES, 0, 6 );
		glBindVertexArray( 0 );

	// WYLACZAMY program
	glUseProgram(0);
}


// ---------------------------------------
// NOWE: Rendering w oknie aplikacji
// ---------------------------------------
void RenderScene_on_Screen()
{
	// Aktywnujemy domyslny frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport( 0, 0, windowWidth, windowHeight );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// AKTYWUJEMY program
	glUseProgram( idProgram );

		// Obliczenie macierzy PVM i przeslanie do potoku
		matPVM = matProj * matView * matModel;
		glUniformMatrix4fv( loc_matPVM, 1, GL_FALSE, glm::value_ptr(matPVM) );

		// RENDEROWANIE sceny (Podloza)
		{
			// Tekstura sceny
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, idTexture[SCENE]);

			glBindVertexArray( idVAO[SCENE] );
			glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices[SCENE].size() );
			glBindVertexArray( 0 );
		}

		// RENDEROWANIE kwadratu (Ekranu)
		{
			// Tekstura sceny
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, idTextureBuffer);

			glBindVertexArray( idVAO[SCREEN] );
			glDrawArrays( GL_TRIANGLES, 0, 6 );
			glBindVertexArray( 0 );

		}

	// WYLACZAMY program
	glUseProgram(0);

}


// -----------------------------------------
// NOWE: Glowna funkcja renderujaca zostaje
// rozbita na dwie
// -----------------------------------------
void DisplayScene()
{
	// Obliczanie macierzy widoku i innych danych
	// wspolnych dla calego renderingu
	matView = UpdateViewMatrix();
	glm::vec3 cameraPos = ExtractCameraPos(matView);


	// --------------------------------
	// NOWE: Renderowanie pozaekranowe
	// --------------------------------
	// ETAP I. Renderowanie do tekstury
	if (isOffscreenRendering)
		RenderScene_to_Texture();


	// ETAP II. Renderowanie na ekran
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



	// Scena OBJ
	if (!loadOBJ("assets/scene.obj", OBJ_vertices[SCENE], OBJ_uvs[SCENE], OBJ_normals[SCENE]))
	{
		printf("Not loaded!\n");
		exit(1);
	}

	// Tekstura sceny
	int tex_width, tex_height, tex_n;
	unsigned char *tex_data;
	tex_data = stbi_load("assets/sand.jpg", &tex_width, &tex_height, &tex_n, 0);
	if (!tex_data) { printf("File texture error!\n"); }

	glGenTextures(1, &idTexture[SCENE]);
	glBindTexture(GL_TEXTURE_2D, idTexture[SCENE]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


	// Potok
	idProgram = glCreateProgram();
	glAttachShader( idProgram, LoadShader(GL_VERTEX_SHADER, "shaders/vertex.glsl"));
	glAttachShader( idProgram, LoadShader(GL_FRAGMENT_SHADER, "shaders/fragment.glsl"));
	LinkAndValidateProgram( idProgram );

	// Pobieranie lokalizacji zmiennych jednorodnych
	// oraz ustawianie uchwytow tekstur
	glUseProgram(idProgram);
		loc_matPVM = glGetUniformLocation( idProgram, "matPVM" );
		loc_tex = glGetUniformLocation(idProgram, "tex");
		glUniform1i(loc_tex, 0);
	glUseProgram(0);


	// Scena
	glGenVertexArrays( 1, &idVAO[SCENE] );
	glBindVertexArray( idVAO[SCENE] );
		GLuint vao_pos, vao_uv;
		glGenBuffers( 1, &vao_pos );
		glBindBuffer( GL_ARRAY_BUFFER, vao_pos );
		glBufferData( GL_ARRAY_BUFFER, OBJ_vertices[SCENE].size() * sizeof(glm::vec3), &(OBJ_vertices[SCENE])[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		glGenBuffers( 1, &vao_uv );
		glBindBuffer( GL_ARRAY_BUFFER, vao_uv );
		glBufferData( GL_ARRAY_BUFFER, OBJ_uvs[SCENE].size() * sizeof(glm::vec2), &(OBJ_uvs[SCENE])[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
	glBindVertexArray( 0 );


	// Ekran
	glGenVertexArrays( 1, &idVAO[SCREEN] );
	glBindVertexArray( idVAO[SCREEN] );
		glGenBuffers( 1, &vao_pos );
		glBindBuffer( GL_ARRAY_BUFFER, vao_pos );
		glBufferData( GL_ARRAY_BUFFER, sizeof(vertices_pos), vertices_pos, GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		glGenBuffers( 1, &vao_uv );
		glBindBuffer( GL_ARRAY_BUFFER, vao_uv );
		glBufferData( GL_ARRAY_BUFFER, sizeof(vertices_tex), vertices_tex, GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
	glBindVertexArray( 0 );



	// 1. Stworzenie obiektu FBO
	glGenFramebuffers(1, &idFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, idFrameBuffer);

		// 2. Stworzenie obiektu tekstury na skladowa koloru
		glGenTextures(1, &idTextureBuffer);
		glBindTexture(GL_TEXTURE_2D, idTextureBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bufferWidth, bufferHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// 3. Polaczenie tekstury ze skladowa koloru FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			idTextureBuffer, 0);

		// 4. Stworzenie obiektu render buffer dla skladowej glebokosci
		glGenRenderbuffers(1, &idDepthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, idDepthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, bufferWidth, bufferHeight);

		// 5. Polaczenie bufora glebokosci z aktualnym obiektem FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, idDepthBuffer);

		// 6. Sprawdzenie czy pomyslnie zostal utworzony obiekt bufora ramki
		//    a nastepnie powrot do domyslnego bufora ramki

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("Error: Framebuffer is not complete!\n");
			exit(1);
		}

	// Przelaczenie na domyslny framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	glDeleteVertexArrays( 1, &idVAO[SCENE] );
	glDeleteVertexArrays( 1, &idVAO[SCREEN] );
	glDeleteBuffers(1, &idVAOpos[SCENE]);
	glDeleteBuffers(1, &idVAOpos[SCREEN]);
	glDeleteBuffers(1, &idVAOuv[SCENE]);
	glDeleteBuffers(1, &idVAOuv[SCREEN]);
	glDeleteTextures(1, &idTexture[SCENE]);
	glDeleteTextures(1, &idTextureBuffer);
	glDeleteFramebuffers(1, &idFrameBuffer);
	glDeleteRenderbuffers(1, &idDepthBuffer);

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
