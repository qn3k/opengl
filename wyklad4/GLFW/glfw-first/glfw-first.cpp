// -------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// ------------------------------------------------
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
// Opengl 4.5 i w razie problemow nalezy podmienic
// plikiem glad.c pobranym z powyzszej strony
// dla nizszej wersji opengla.
// ------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Identyfikatory obiektow
GLuint idProgram;	// programu
GLuint idVBO;		// bufora wierzcholkow
GLuint idVAO;		// tablic wierzcholkow

// Wspolrzedne wierzcholkow
GLfloat triangles[1*3*2] =
{
	-1.f, -1.f,
	1.f, -1.f,
	0.f,  1.f
};

// Zmienna czas do animacji
float Time = 0.0;

// ---------------------------------------
void CreateVertexShader( void )
{
	GLuint shader = glCreateShader( GL_VERTEX_SHADER );

	const GLchar * code =
	"#version 330 \n			"
	"in vec4 inPosition;	"
	"void main() 				"
	"{ 							"
	"	gl_Position = inPosition; "
	"}							";

	glShaderSource( shader, 1, &code, NULL );
	glCompileShader( shader );

	GLint status;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
	if( status != GL_TRUE )
	{
		printf("Blad kompilacji vertex shadera!\n");
		return;
	}
	glAttachShader( idProgram, shader );
}

// ---------------------------------------
void CreateFragmentShader( void )
{
	GLuint shader = glCreateShader( GL_FRAGMENT_SHADER );

	const GLchar *code =
	"#version 150 \n			"
	"out vec4 outColor;			"
	"uniform float Time;        "
	"void main()				"
	"{							"
	"	outColor = vec4(Time, 1.f, 0.f, 1.f); "
	"}";


	glShaderSource( shader, 1, &code, NULL );
	glCompileShader( shader );

	GLint status;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
	if( status != GL_TRUE )
	{
		printf("Blad kompilacji fragment shadera!\n");
		return;
	}
	glAttachShader( idProgram, shader );
}

// ---------------------------------------
void DisplayScene()
{
    Time += 0.05;
    if (Time > 1.0) Time = 0.0;


    // Czyszczenie bufora ramki koloru
    glClear( GL_COLOR_BUFFER_BIT );

    // Aktywacja potoku
    glUseProgram( idProgram );
    glUniform1f(glGetUniformLocation(idProgram, "Time"), Time);

        // Rendering z VAO
        glBindVertexArray( idVAO );
        glDrawArrays( GL_TRIANGLES, 0, 1*3 );
        glBindVertexArray( 0 );

    // Deaktywacja potoku
    glUseProgram( 0 );
}

// ---------------------------------------
void Initialize()
{
	// ustawianie koloru ktorym bedzie czyszczony bufor ramki
	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

	// 1. Potok
	idProgram = glCreateProgram();
	CreateVertexShader();
	CreateFragmentShader();
	// Walidacja shaderow
	glLinkProgram( idProgram );
	glValidateProgram( idProgram );

	// 2. Dane (VAO)
	glGenVertexArrays( 1, &idVAO );
	glBindVertexArray( idVAO );
	glGenBuffers( 1, &idVBO );
	glBindBuffer( GL_ARRAY_BUFFER, idVBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof(float)*6, triangles, GL_STATIC_DRAW );
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, NULL );
	glEnableVertexAttribArray( 0 );
	glBindVertexArray( 0 );

}



// ---------------------------------------------------
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

}

// ---------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport( 0, 0, width, height );
}

// ---------------------------------------------------
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);

}

// ---------------------------------------------------
int main( int argc, char *argv[] )
{
	GLFWwindow* window;

	// Funkcja zwrotna do bledow
	glfwSetErrorCallback(error_callback);

	// Inicjalizacja glfw
	if (!glfwInit()) exit(EXIT_FAILURE);

	// Ustalenie wersji OpenGL na 4.4
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

	// Tworzenie okna aplikacji
	window = glfwCreateWindow(800, 800, "Programowanie grafiki 3D w OpenGL (GLFW template)", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);

	// Inne funkcje zwrotne (obecnie niepotrzebne)
	//glfwSetMouseButtonCallback(window, mouse_button_callback);
	//glfwSetCursorPosCallback(window, cursor_position_callback);
	//glfwSetWindowSizeCallback(window, window_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    glfwSwapInterval(1);


	// Inicjalizacja, tworzenie potoku, VAO itp.
	Initialize();

	while (!glfwWindowShouldClose(window))
	{
		// Obsluga zdarzen
		glfwPollEvents();


		DisplayScene();


		// Podmianka buforow koloru
		glfwSwapBuffers(window);
	}

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);

	return 0;
}
