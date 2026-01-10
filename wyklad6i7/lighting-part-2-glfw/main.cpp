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
#include "objloader.hpp"

// -----------------------------------------
// NOWE: plik do obslugi plikow graficznych
// -----------------------------------------
# define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using glm::vec3;

// Obiekty na scenie
enum {
	OBJ_GROUND,	// ziemia
	OBJ_SUZANNE	// obiekt na scenie
};

// Macierze transformacji i rzutowania
glm::mat4 matProj = glm::mat4(1.0);
glm::mat4 matView = glm::mat4(1.0);
glm::mat4 matModel[2];

// Lokalne pliki naglowkowe
#include "utilities.hpp"


// Identyfikatory obiektow
GLuint idProgram;
GLuint idVAO[2];


// Wektory na dane z pliku OBJ
std::vector<glm::vec3> OBJ_vertices[2];
std::vector<glm::vec2> OBJ_uvs[2];
std::vector<glm::vec3> OBJ_normals[2];


// Struktura parametrow swiatla
struct LightParam
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
	vec3 Attenuation;
	vec3 Position; // lub Direction dla kierunkowego
};

// Struktura materialu obiektu
struct MaterialParam
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
	float Shininess;
};


// Przykladowe swiatlo punktowe
struct LightParam myLight =
{
	vec3 (0.1 , 0.1 , 0.1), // ambient
	vec3 (1.0 , 1.0 , 1.0), // diffuse
	vec3 (1.0 , 1.0 , 1.0), // specular
	vec3 (1.0 , 0.0 , 0.01), // attenuation
	vec3 (2.0 , 3.0 , 1.0)  // position
};


// Przykladowe materialy
struct MaterialParam myMaterial[2] =
{
	{
		vec3 (0.2, 0.2, 0.2), // ambient
		vec3 (1.0, 1.0, 1.0), // diffuse
		vec3 (0.5, 0.5, 0.5), // specular
		32.0 // shininess
	},
	{
		vec3 (0.2, 0.2, 0.2), // ambient
		vec3 (1.0, 1.0, 1.0), // diffuse
		vec3 (1.0, 1.0, 1.0), // specular
		8.0 // shininess
	}
};


// ---------------------------------------------------
// Przeslanie parametrow oswietlenia do shadera
void sendLightParameters(LightParam light){
	// pobranie id aktualnego programu
    GLint programId;
    glGetIntegerv(GL_CURRENT_PROGRAM, &programId);

    glUniform3fv(glGetUniformLocation(programId, "myLight.Ambient"), 1, glm::value_ptr(light.Ambient));
    glUniform3fv(glGetUniformLocation(programId, "myLight.Diffuse"), 1, glm::value_ptr(light.Diffuse));
    glUniform3fv(glGetUniformLocation(programId, "myLight.Specular"), 1, glm::value_ptr(light.Specular));
    glUniform3fv(glGetUniformLocation(programId, "myLight.Attenuation"), 1, glm::value_ptr(light.Attenuation));
    glUniform3fv(glGetUniformLocation(programId, "myLight.Position"), 1, glm::value_ptr(light.Position));
}

// ---------------------------------------------------
// Przeslanie parametrow materialow do shadera
void sendMaterialParameters(MaterialParam material){
	// pobranie id aktualnego programu
    GLint programId;
    glGetIntegerv(GL_CURRENT_PROGRAM, &programId);

    glUniform3fv(glGetUniformLocation(programId, "myMaterial.Ambient"), 1, glm::value_ptr(material.Ambient));
    glUniform3fv(glGetUniformLocation(programId, "myMaterial.Diffuse"), 1, glm::value_ptr(material.Diffuse));
    glUniform3fv(glGetUniformLocation(programId, "myMaterial.Specular"), 1, glm::value_ptr(material.Specular));
    glUniform1f(glGetUniformLocation(programId, "myMaterial.Shininess"), material.Shininess);
}


// ---------------------------------------------------
void DisplayScene()
{
	// Czyszczenie bufora koloru i glebokosci
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	__CHECK_FOR_ERRORS

	// Wyliczanie macierz widoku
	matView = UpdateViewMatrix();

	// Wyliczanie pozycji kamery z matView
	glm::vec3 cameraPos = ExtractCameraPos(matView);


	// Wlaczenie programu
	glUseProgram( idProgram );


		// Przekazanie macierzy rzutowania i pozycje kamery
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(matProj));
		glUniformMatrix4fv( glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(matView));
		glUniform3fv( glGetUniformLocation( idProgram, "cameraPos" ), 1, glm::value_ptr(cameraPos) );

        // ----------------------------------------
        // NOWE: Przekazanie parametrow oswietlenia
        // ----------------------------------------
        sendLightParameters(myLight);

		// RENDERING: Podloze
		{
			// Material
			sendMaterialParameters(myMaterial[OBJ_GROUND]);
			// Macierz modelu obiektu
			matModel[OBJ_GROUND] = glm::mat4(1.0);
			glUniformMatrix4fv( glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModel[OBJ_GROUND]));

			// Rendering
			glBindVertexArray( idVAO[OBJ_GROUND] );
			glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices[OBJ_GROUND].size() );
			glBindVertexArray( 0 );
		}

		// RENDERING: Suzanne
		{
			// Material
			sendMaterialParameters(myMaterial[OBJ_SUZANNE]);
			// Macierz modelu obiektu
			matModel[OBJ_SUZANNE] = glm::mat4(1.0);
			// TO_DO: Animacja Suzanne
			glUniformMatrix4fv( glGetUniformLocation(idProgram, "matModel"), 1, GL_FALSE, glm::value_ptr(matModel[OBJ_SUZANNE]));

			// Rendering
			glBindVertexArray( idVAO[OBJ_SUZANNE] );
			glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices[OBJ_SUZANNE].size() );
			glBindVertexArray( 0 );
		}


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
	if (!loadOBJ("scene.obj", OBJ_vertices[OBJ_GROUND], OBJ_uvs[OBJ_GROUND], OBJ_normals[OBJ_GROUND]))
	{
		printf("Not loaded!\n");
		exit(1);
	}

	// Potok
	idProgram = glCreateProgram();
	glAttachShader( idProgram, LoadShader(GL_VERTEX_SHADER, "vertex.glsl"));
	glAttachShader( idProgram, LoadShader(GL_FRAGMENT_SHADER, "fragment.glsl"));
	LinkAndValidateProgram( idProgram );


	// Wczytanie pliku obj
	if (!loadOBJ("scene.obj", OBJ_vertices[OBJ_GROUND], OBJ_uvs[OBJ_GROUND], OBJ_normals[OBJ_GROUND]))
	{
		printf("Not loaded!\n");
		exit(1);
	}

	GLuint idVBO_coord, idVBO_uv, idVBO_normal;

	// VAO
	glGenVertexArrays( 1, &idVAO[OBJ_GROUND] );
	glBindVertexArray( idVAO[OBJ_GROUND] );
		// Bufor na wspolrzedne wierzcholkow
		glGenBuffers( 1, &idVBO_coord );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_coord );
		glBufferData( GL_ARRAY_BUFFER, OBJ_vertices[OBJ_GROUND].size() * sizeof(glm::vec3), &OBJ_vertices[OBJ_GROUND][0], GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		// Bufor na wspolrzedne UV
		glGenBuffers( 1, &idVBO_uv );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_uv );
		glBufferData( GL_ARRAY_BUFFER, OBJ_uvs[OBJ_GROUND].size() * sizeof(glm::vec2), &OBJ_uvs[OBJ_GROUND][0], GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
		// Bufor na wektory normalne
		glGenBuffers( 1, &idVBO_normal );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_normal );
		glBufferData( GL_ARRAY_BUFFER, OBJ_normals[OBJ_GROUND].size() * sizeof(glm::vec3), &OBJ_normals[OBJ_GROUND][0], GL_STATIC_DRAW );
		glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 2 );
	glBindVertexArray( 0 );

	// Wczytanie pliku obj
	if (!loadOBJ("suzanne.obj", OBJ_vertices[OBJ_SUZANNE], OBJ_uvs[OBJ_SUZANNE], OBJ_normals[OBJ_SUZANNE]))
	{
		printf("Not loaded!\n");
		exit(1);
	}

	// VAO
	glGenVertexArrays( 1, &idVAO[OBJ_SUZANNE] );
	glBindVertexArray( idVAO[OBJ_SUZANNE] );
		// Bufor na wspolrzedne wierzcholkow
		glGenBuffers( 1, &idVBO_coord );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_coord );
		glBufferData( GL_ARRAY_BUFFER, OBJ_vertices[OBJ_SUZANNE].size() * sizeof(glm::vec3), &OBJ_vertices[OBJ_SUZANNE][0], GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		// Bufor na wspolrzedne UV
		glGenBuffers( 1, &idVBO_uv );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_uv );
		glBufferData( GL_ARRAY_BUFFER, OBJ_uvs[OBJ_SUZANNE].size() * sizeof(glm::vec2), &OBJ_uvs[OBJ_SUZANNE][0], GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
		// Bufor na wektory normalne
		glGenBuffers( 1, &idVBO_normal );
		glBindBuffer( GL_ARRAY_BUFFER, idVBO_normal );
		glBufferData( GL_ARRAY_BUFFER, OBJ_normals[OBJ_SUZANNE].size() * sizeof(glm::vec3), &OBJ_normals[OBJ_SUZANNE][0], GL_STATIC_DRAW );
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
