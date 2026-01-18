// ---------------------------------------------------
// Funkcje pomocnicze, w szczegolnosci
// 1. funkcja do obslugi widoku sceny
// 2. funkcje zwrotne GLFW
// 3. funkcje do obslugi plikow shaderow
// ---------------------------------------------------
#ifndef __UTILITIES_HPP
#define __UTILITIES_HPP

// ---------------------------------------------------
// Makro do znajdowania prostych bledow opengla
// ---------------------------------------------------
#define __CHECK_FOR_ERRORS 	{GLenum errCode; if ((errCode = glGetError()) != GL_NO_ERROR) printf("Error code %d in the file %s at line %d !\n", errCode, __FILE__,  __LINE__);}


// Okno aplikacji
extern int windowWidth, windowHeight;
extern const char *windowTitle;
extern GLFWwindow* window;

// Macierz rzutowania ustalana w funkcji zwrotnej
// framebuffer_size_callback
extern glm::mat4 matProj;

// Zmienne do kontroli obrotu kamery
GLfloat cameraRotateX = 0.3f;
GLfloat cameraRotateY = 0.0f;

// Zmienne do kontroli pozycji kamery
GLfloat cameraTranslateX = 0.0f;
GLfloat cameraTranslateY = 0.0f;
GLfloat cameraTranslateZ = -8.0f;


// Zmienne do obslugi myszy
double __mouse_buttonX;
double __mouse_buttonY;
bool __mouse_press = false;
int  __mouse_button = 0;


// --------------------------------------------------------------
// Funkcja zwraca macierz widoku dla kamery
// --------------------------------------------------------------
glm::mat4 UpdateViewMatrix()
{
	glm::mat4 matView = glm::mat4x4( 1.0 );
	matView = glm::translate( matView, glm::vec3( cameraTranslateX, cameraTranslateY, cameraTranslateZ) );
	matView = glm::rotate( matView, cameraRotateX, glm::vec3( 1.0f, 0.0f, 0.0f ) );
	matView = glm::rotate( matView, cameraRotateY, glm::vec3( 0.0f, 1.0f, 0.0f ) );

	return matView;
}


// --------------------------------------------------------------
// NOWE: funkcja wyliczajaca pozycje kamery z macierzy widoku
// zrodlo: https://community.khronos.org/t/extracting-camera-position-from-a-modelview-matrix/68031
// --------------------------------------------------------------
glm::vec3 ExtractCameraPos(const glm::mat4 & a_modelView)
{
  // Get the 3 basis vector planes at the camera origin and transform them into model space.
  //
  // NOTE: Planes have to be transformed by the inverse transpose of a matrix
  //       Nice reference here: http://www.opengl.org/discussion_boards/showthread.php/159564-Clever-way-to-transform-plane-by-matrix
  //
  //       So for a transform to model space we need to do:
  //            inverse(transpose(inverse(MV)))
  //       This equals : transpose(MV) - see Lemma 5 in http://mathrefresher.blogspot.com.au/2007/06/transpose-of-matrix.html
  //
  // As each plane is simply (1,0,0,0), (0,1,0,0), (0,0,1,0) we can pull the data directly from the transpose matrix.
  //
  glm::mat4 modelViewT = transpose(a_modelView);

  // Get plane normals
  glm::vec3 n1(modelViewT[0]);
  glm::vec3 n2(modelViewT[1]);
  glm::vec3 n3(modelViewT[2]);

  // Get plane distances
  float d1(modelViewT[0].w);
  float d2(modelViewT[1].w);
  float d3(modelViewT[2].w);

  // Get the intersection of these 3 planes
  // http://paulbourke.net/geometry/3planes/
  glm::vec3 n2n3 = cross(n2, n3);
  glm::vec3 n3n1 = cross(n3, n1);
  glm::vec3 n1n2 = cross(n1, n2);

  glm::vec3 top = (n2n3 * d1) + (n3n1 * d2) + (n1n2 * d3);
  float denom = dot(n1, n2n3);

  return top / -denom;
}


// ---------------------------------------------------
// funkcja zwrotna do obslugi klawiatury
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	extern bool isOffscreenRendering;

	if (action == GLFW_PRESS)
	{
		switch(key)
		{
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;

		case GLFW_KEY_SPACE:
			isOffscreenRendering = !isOffscreenRendering;
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
// funkcja zwrotna do obslugi scrolla myszy
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	cameraTranslateZ += yoffset;
}

// --------------------------------------------------------------
// funkcja zwrotna do obslugi ruchu kursora myszy
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{

	if (__mouse_press && __mouse_button == GLFW_MOUSE_BUTTON_LEFT)
	{
		cameraRotateX += 2.0*(ypos - __mouse_buttonY)/(float)windowWidth;
		cameraRotateY += 2.0*(xpos - __mouse_buttonX)/(float)windowHeight;
		__mouse_buttonX = xpos;
		__mouse_buttonY = ypos;
	}
}


// --------------------------------------------------------------
// funkcja zwrotna do obslugi klawiszy myszy
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS)
	{
		__mouse_press = true;
		__mouse_button = button;

		glfwGetCursorPos(window, &__mouse_buttonX, &__mouse_buttonY);
	}

	if (action == GLFW_RELEASE)
	{
		__mouse_press = false;
	}
}


// ---------------------------------------------------
// funkcja zwrotna do obslugi bledow glfw
static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}


// ---------------------------------------------
// Inicjalizacja kontekstu OpenGL i GLFW
void Initialize_GLFW(GLFWwindow* &window)
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

	window = glfwCreateWindow((int)(windowWidth), (int)(windowHeight), windowTitle, nullptr, nullptr);
	if (window == nullptr) exit(1);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // vsync
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("Failed to initialize GLAD\n");
		exit(-1);
	}

	// Rejestracja funkcji zwrotnych
	// do obslugi zdarzen
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
}

// ---------------------------------------------
unsigned long getFileLength(const char *filename)
{
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		fprintf(stderr, "Can't open the file %s!\n", filename);
		exit(1);
	}

	unsigned long fileSize;

	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	fclose(file);

	return fileSize;
}



// ---------------------------------------------
GLchar * LoadShaderFile(const char* filename)
{
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		fprintf(stderr, "Nie moge otworzyc pliku %s!\n", filename);
		exit(1);
	}

	unsigned long fileSize;

	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (fileSize == 0) {
		printf("Plik %s jest pusty!\n", filename);
		exit(1);
	};

	GLchar *ShaderSource = new GLchar[fileSize + 1];
	if (ShaderSource == NULL) {
		printf("Nie moge zaalokowac %ld bajtow \n", fileSize + 1);
		exit(1);
	}

	int bytesRead = 0;
	char c;
	while ((c = fgetc(file)) != EOF && bytesRead < fileSize - 1) {
		ShaderSource[bytesRead++] = c;
	}
	ShaderSource[bytesRead] = '\0';

	fclose(file);
	return ShaderSource;
}

// ---------------------------------------
void CheckForErrors_Shader(GLuint shader)
{
	GLint status;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
	if( status != GL_TRUE )
	{
		printf("OpenGL Error!\n");
		GLint logLength;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLength );
		char *log = new char[logLength];
		glGetShaderInfoLog( shader, logLength, NULL, log );
		printf("LOG: %s\n", log);
		delete[] log;
	}
}

// ---------------------------------------
void CheckForErrors_Program(GLuint program, GLenum mode)
{
	GLint status;
	glGetProgramiv( program, mode, &status );
	if( status != GL_TRUE )
	{
		switch (mode)
		{
			case GL_LINK_STATUS:
				printf("Blad konsolidacji programu!\n");
				break;
			case GL_VALIDATE_STATUS:
				printf("Blad walidacji programu!\n");
				break;
			default:
				printf("Inny blad programu!\n");

		}
		GLint logLength;
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logLength );
		char *log = new char[logLength];
		glGetProgramInfoLog( program, logLength, NULL, log );
		printf("LOG: %s\n", log);
		delete[] log;
	}
}

// ---------------------------------------
GLuint LoadShader( GLuint MODE, const char *filename  )
{
	// utworzenie obiektu shadera
	GLuint shader = glCreateShader( MODE );

	// Wczytanie kodu shadera z pliku
	GLchar *code = LoadShaderFile(filename);

	glShaderSource( shader, 1, &code, NULL );
	glCompileShader( shader );
	CheckForErrors_Shader(shader);

	delete[] code;
	return shader;
}

// ---------------------------------------
void LinkAndValidateProgram(GLuint program)
{
	// Konsolidacja programu
	glLinkProgram( program );
	CheckForErrors_Program(program, GL_LINK_STATUS);

	// Walidacja programu
	glValidateProgram( program );
	CheckForErrors_Program(program, GL_VALIDATE_STATUS);
}

#endif

