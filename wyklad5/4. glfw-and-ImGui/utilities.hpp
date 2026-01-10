// ---------------------------------------------------
// Funkcje pomocnicze, w szczegolnosci do wczytywania
// plikow shaderow
// ---------------------------------------------------
#ifndef __UTILITIES_HPP
#define __UTILITIES_HPP


// Zmienne do kontroli obrotu kamery
GLfloat cameraRotateX = 0.3f;
GLfloat cameraRotateY = 0.0f;

// Zmienne do kontroli pozycji kamery
GLfloat cameraTranslateX = 0.0f;
GLfloat cameraTranslateY = 0.0f;
GLfloat cameraTranslateZ = -10.0f;


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


// ---------------------------------------------------
// funkcja zwrotna do obslugi klawiatury
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
// tresc przeniesiona do glownego pliku

// ---------------------------------------------------
// funkcja zwrotna do obslugi zmiany rozmiaru framebuffera
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// tresc przeniesiona do glownego pliku

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
	// -----------------------------------
	// ImGui ImGui ImGui ImGui ImGui ImGui
	// -----------------------------------
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse) return;  // ImGui aktualnie uzywa myszy


	if (__mouse_press && __mouse_button == GLFW_MOUSE_BUTTON_LEFT)
	{
		cameraRotateX += (ypos - __mouse_buttonY)/(float)windowWidth;
		cameraRotateY += (xpos - __mouse_buttonX)/(float)windowHeight;
		__mouse_buttonX = xpos;
		__mouse_buttonY = ypos;
	}
}


// --------------------------------------------------------------
// funkcja zwrotna do obslugi klawiszy myszy
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// -----------------------------------
	// ImGui ImGui ImGui ImGui ImGui ImGui
	// -----------------------------------
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse) return;  // ImGui aktualnie uzywa myszy


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

	// -----------------------------------
	// ImGui ImGui ImGui ImGui ImGui ImGui
	// -----------------------------------
	float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

	// Okno aplikacji i kontekst
	window = glfwCreateWindow((int)(windowWidth * main_scale), (int)(windowHeight * main_scale), windowTitle, nullptr, nullptr);
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

