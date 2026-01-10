// -------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// ------------------------------------------------
// Szablon aplikacji w OpenGL+GLFW+ImGui
//
// ImGui:
// https://github.com/ocornut/imgui
//
// Kompilacja pod windowsem
// g++ main.cpp glad.c ImGui\*.cpp -lglfw3 -lopengl32 -lgdi32
//
// Wymagana jest bibliteka glad
// https://glad.dav1d.de/
//
// Dolaczony plik glad.c jest skonfigurowany pod
// Opengl 4.5. W razie problemow nalezy podmienic
// skonfigurowac glada pod swoje srodowisko.
// ------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// Biblioteki GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// Okno aplikacji
int windowWidth = 600, windowHeight = 600;
const char *windowTitle = "OpenGL w GLFW (Szablon)";

// Lokalne pliki naglowkowe
#include "objloader.hpp"
// -----------------------------------
// ImGui ImGui ImGui ImGui ImGui ImGui
// -----------------------------------
#include "myImGui.hpp"
#include "utilities.hpp"




// Macierze transformacji i rzutowania
glm::mat4 matProj;
glm::mat4 matView;
glm::mat4 matModel;


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

// Zmienna czas do animacji
float Time = 0.0;



// ---------------------------------------
void DisplayScene()
{
	if (show_animation) Time += 0.01;
	if (Time > 2*3.1416) Time = 0.0;

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


// -----------------------------------
// ImGui ImGui ImGui ImGui ImGui ImGui
// -----------------------------------
void ImGui_Display()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuiIO& io = ImGui::GetIO();

	// 1. Show demo
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);


	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
	{
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);
		ImGui::Checkbox("Animation", &show_animation);

		ImGui::SliderFloat("Time", &Time, 0.0f, 2*3.1416f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);


		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Time: %.1f ", Time);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	// Zamykanie procesu renderowania ImGui
	ImGui::Render();


	// Renderowanie do framebuffera
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	// Update and Render additional Platform Windows
	// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
	//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
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
// funkcja zwrotna do obslugi zmiany rozmiaru framebuffera
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	windowWidth = width;
	windowHeight = height;

	// Aktualizacja macierzy rzutowania
	if (windowHeight != 0)
		matProj = glm::perspective(glm::radians(80.0f), windowWidth/(float)windowHeight, 0.1f, 50.0f);

	// Viewport
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
}

// ---------------------------------------------------
// funkcja zwrotna do obslugi klawiatury
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// -----------------------------------
	// ImGui ImGui ImGui ImGui ImGui ImGui
	// -----------------------------------
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard) return; // ImGui aktualnie uzywa klawiatury

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (action == GLFW_PRESS)
	{
		printf("Nacisnieto klawisz %d \n", key);
	}

}

// ---------------------------------------------------
int main( int argc, char *argv[] )
{
	// Kontekst i okno aplikacji
	GLFWwindow* window = nullptr;
	Initialize_GLFW(window);

	// -----------------------------------
	// ImGui ImGui ImGui ImGui ImGui ImGui
	// -----------------------------------
	ImGui_Init(window);


	// Inicjalizacja
	Initialize();


	// Glowna petla
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// Render Sceny
		DisplayScene();

		// -----------------------------------
		// ImGui ImGui ImGui ImGui ImGui ImGui
		// -----------------------------------
		ImGui_Display();


		glfwSwapBuffers(window);
	}

	// -----------------------------------
	// ImGui ImGui ImGui ImGui ImGui ImGui
	// -----------------------------------
	ImGui_Clean();


	// Czyszczenie
	glDeleteVertexArrays(1, &idVAO);
	glDeleteBuffers(1, &idVBO_coord);
	glDeleteBuffers(1, &idVBO_color);
	glDeleteProgram(idProgram);
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
